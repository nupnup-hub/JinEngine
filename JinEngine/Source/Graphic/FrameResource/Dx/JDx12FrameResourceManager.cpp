/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

#include"JDx12FrameResourceManager.h" 

#include"../../../Core/Math/JMathHelper.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Core/Time/JGameTimer.h" 
#include"../../../Core/Threading/JThreadManager.h"
#include"../../../Core/Threading/JThreadUtil.h"

#include"../../../Object/Component/RenderItem/JRenderItem.h" 
#include"../../../Object/Component/Transform/JTransform.h"
#include"../../../Object/Component/Camera/JCamera.h" 
#include"../../../Object/Component/Animator/JAnimator.h"  
#include"../../../Object/Component/Light/JDirectionalLight.h" 
#include"../../../Object/Component/Light/JPointLight.h" 
#include"../../../Object/Component/Light/JSpotLight.h" 
#include"../../../Object/Component/Light/JRectLight.h" 

#include"../../../Object/Resource/JResourceObjectDefualtData.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Scene/JScene.h" 
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Object/Resource/Material/JMaterial.h" 
#include"../../../Object/Resource/Shader/JShader.h" 
#include"../../../Object/Resource/AnimationClip/JAnimationClip.h" 
#include"../../../Object/Resource/Texture/JTexture.h" 

#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Develop/Debug/JDevelopDebug.h"

using namespace DirectX;
namespace JinEngine::Graphic
{ 
	namespace
	{ 
		/**
		* Multi thread rule
		* 
		* 적용여부는 항상 업데이트되며 업로드시간이 긴 경우로 한정한다.
		* Light & Material (X) :  많은 갯수가 있을법하나 자주 업데이트 되지않음.
		* Camera (X) : 갯수가 적으므로 제외.
		* Render item (X): 업로드하는 시간이 적고 항상 업데이트 되지않으므로 제외(어림잡아 한 프레임에 250~ 500개정도 업데이트 되는게 최대가 아닐까?) 
		* Animation (O) : 항상 업데이트되며 업로드 시간이 적지않다.
		* 
		* loop cost
		* 2000 count except upload func =  0.15 ~ 0.25ms
		* 2000 count all upload func = 2.5ms ~ 3.5ms (RenderItem)
		* 8 Thread begin cost 0.02ms
		* Render item upload => 0.0035ms
		* Animation upload => Just bind pose = 0.001ms , One diagram 3 clip = 0.03ms
		* 
		* Animation thread apply condition
		* 
		* Animation timer 미작동인 경우 80개
		* Animation timer 작동 중 인경우 4개
		* 4개라는 숫자는 Preview scene을 제외하고 Thread cost보다 upload cost가 확실히 높다고 예상하는 숫자이다
		*/

		static constexpr uint maxNumOfUpdateThread = JDx12FrameResourceManager::maxNumOfUpdateThread; 
		static constexpr uint aniThreadBorder = 4;
		static constexpr uint aniOnlyBindPoseThreadBorder = 80;

		struct InnerUpdateDataSet
		{
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
		public:
			JDx12FrameResourceManager* fm;
			JDx12FrameResource* frame;
		public:
			JDx12FrameResourceManager::CacheData* cacheData;
		public:
			//Per object
			JObject* obj;
			JFrameUpdateInterface* updateInterface;
		public:
			int minMoveDirtyIndex[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];
			bool isFrameDirted = false;
			bool hasMoveDirted = false;
		public:
			InnerUpdateDataSet(const JGraphicInfo& info,
				const JGraphicOption& option,
				JDx12FrameResourceManager* fm,
				JDx12FrameResourceManager::CacheData* cacheData)
				:info(info), option(option), fm(fm), cacheData(cacheData)
			{ 
				frame = fm->GetCurrentDxFrameResource();
				memset(minMoveDirtyIndex, 0, sizeof(int) * (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT);
			}
		public:
			template<J_FRAME_RESOURCE_UPLOAD_TYPE type, typename Type>
			void CopyData(Type* constants)const
			{
				//if (type == J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO ||
				//	type == J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW)
				/*
							{
					Develop::JDevelopDebug::PushLog(obj->GetTypeInfo().Name() + " " + std::to_string(obj->GetGuid()));
					Develop::JDevelopDebug::PushLog(Core::GetName(type));
					for (uint i = 0; i < fm->updateInfoVec[(uint)type].size(); ++i)
					{
						Develop::JDevelopDebug::PushLog(std::to_string(i) + " " +
							std::to_string(fm->updateInfoVec[(uint)type][i]->GetNumber()) + " " +
							std::to_string(fm->updateInfoVec[(uint)type][i]->GetFrameIndex()) + " " +
							std::to_string(uint64(fm->updateInfoVec[(uint)type][i].Get())) + " " +
							std::to_string(uint64(&fm->updateInfoVec[(uint)type][i])));
					}
					Develop::JDevelopDebug::Write();
				}
				*/

				frame->CopyData(type, updateInterface->GetFrameIndex(type), constants);
			}
			template<J_FRAME_RESOURCE_UPLOAD_TYPE type, typename Type>
			void CopyData(const std::vector<Type>& constants, const int count)const
			{
				//if (type == J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO ||
				//	type == J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW)
				/*
								{
					Develop::JDevelopDebug::PushLog(obj->GetTypeInfo().Name() + " " + std::to_string(obj->GetGuid()));
					Develop::JDevelopDebug::PushLog(Core::GetName(type));
					for (uint i = 0; i < fm->updateInfoVec[(uint)type].size(); ++i)
					{
						Develop::JDevelopDebug::PushLog(std::to_string(i) + " " +
							std::to_string(fm->updateInfoVec[(uint)type][i]->GetNumber()) + " " +
							std::to_string(fm->updateInfoVec[(uint)type][i]->GetFrameIndex()) + " " +
							std::to_string(uint64(fm->updateInfoVec[(uint)type][i].Get())) + " " +
							std::to_string(uint64(&fm->updateInfoVec[(uint)type][i])));
					}
					Develop::JDevelopDebug::Write();
				}
				*/
				frame->CopyData(type, updateInterface->GetFrameIndex(type), count, constants);
			}
		public:
			bool HasMoveDirty(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
			{
				return minMoveDirtyIndex[(uint)type] != invalidIndex && minMoveDirtyIndex[(uint)type] <= updateInterface->GetFrameIndex(type);
			}
		};
		struct InnerUpdateFuncSet
		{
		public:
			using UpdateF = JinEngine::Core::JSFunctorType<void, const InnerUpdateDataSet&>::Ptr;
			using IsForcedUpdateF = JinEngine::Core::JSFunctorType<bool, JDx12FrameResourceManager*>::Ptr;
			using CanUseMultiThread = JinEngine::Core::JSFunctorType<bool, const JFrameUpdateOption&, const uint>::Ptr;			 
		public:
			UpdateF updateF[maxNumOfUpdateThread];
			IsForcedUpdateF isForcedUpdateF = nullptr; 
			CanUseMultiThread canUseMultiThread = nullptr;
		public:
			InnerUpdateFuncSet()
			{
				for (uint i = 0; i < maxNumOfUpdateThread; ++i)
					updateF[i] = nullptr;
			}
		};
 
		template<typename T>
		static void ControlStaticConstantsSize(std::vector<T>& vec, const uint updateCount, const float resizeRate = 2.0f)
		{
			if (vec.size() < updateCount || vec.size() > (updateCount * resizeRate))
				vec.resize(updateCount);
		}
		/**
		* Caution!
		* Constants set은 256 byte단위로 Gpu에 upload되며
		* 이는 Cpu Constants structure size와 다를수있으므로
		* 복수의 data를 CopyData할시에는 Gpu에 upload되는 크기와 같은
		* structure을 할당하거나(권장), 하나씩 CopyData를 해야한다(비권장 너무느림)
		*/
		/*
		speed test by data
		Data size: 16384
		Loop Count: 100000
		Task => Call function and do (infoData.f4x4[a % 16]._22 + a) ... 2 ref 1 indexing 1 add

		static stack
		0.1275.ms

		stack
		0.351.ms

		heap
		0.1227.ms

		static statck과 heap에 차이가 미미하나 static statck가 좀 더 깔끔한 코드작성이 가능하므로 사용한다.
		*/
		template<int threadIndex>
		static void UpdateAnimator(const InnerUpdateDataSet& set)
		{
			//Just bind pose = 0.001ms, One diagram 3 clip = 0.03ms   
			static JAnimationConstants animation;
			JAnimator* animator = static_cast<JAnimator*>(set.obj);
			 
			animator->Compute(animation.set);
			animator->Update();
			set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::ANIMATION>(&animation);
		}
		template<int threadIndex>
		static void UpdateBehavior(const InnerUpdateDataSet& set)
		{
			/*
			* 미구현
			*/
		}
		template<int threadIndex>
		static void UpdateCamera(const InnerUpdateDataSet& set)
		{
			JCamera* camera = static_cast<JCamera*>(set.obj);
			auto gUser = camera->ModuleManagedData()->GetGraphicResourceUserInterface();

			static JDrawSceneCameraConstants drawScene;
			static JDepthTestConstants depthTest;
			static JHzbOccComputeConstants hzb;
			static JSsaoConstants ssao;
			static JLightCullingCameraConstants lightCulling;
			static JGIConstants gi;
			static JGIDenoiserPassConstants denoise;

			const size_t sceneGuid = camera->GetOwner()->GetOwnerGuid();
			const XMMATRIX view = camera->GetView().LoadXM();
			const XMMATRIX proj = camera->GetProj().LoadXM();
			const XMMATRIX invView = camera->GetInvView();
			const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
			//const XMMATRIX preViewProj = camera->GetPreViewProj();
			const JVector2F rtSize = camera->GetRenderTargetSize();
			const JVector2F invRtSize = JVector2F::One() / rtSize;
			JVector2F uvToViewA;
			JVector2F uvToViewB;
			camera->GetUvToView(uvToViewA, uvToViewB);

			const JVector3F posW = camera->GetTransform()->GetWorldPosition();
			const float camNear = camera->GetNear();
			const float camFar = camera->GetFar();
			const float viewWidth = camera->GetRenderViewWidth();
			const float viewHeight = camera->GetRenderViewHeight();

			//DrawScene
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA))
			{
				drawScene.invView.StoreXM(XMMatrixTranspose(invView));
				drawScene.viewProj.StoreXM(XMMatrixTranspose(viewProj));
				//drawScene.preViewProj.StoreXM(XMMatrixTranspose(preViewProj));
				drawScene.renderTargetSize = rtSize;
				drawScene.invRenderTargetSize = invRtSize;
				drawScene.uvToViewA = uvToViewA;
				drawScene.uvToViewB = uvToViewB;
				drawScene.eyePosW = posW;
				drawScene.nearZ = camNear;
				drawScene.farZ = camFar;
				drawScene.csmLocalIndex = camera->ModuleManagedData()->GetCsmTargetUserInterface()->GetTargetIndex();
				drawScene.hasAoTexture = camera->AllowSsao() && set.option.CanUseSSAO();
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA>(&drawScene);
			}
			//DepthTest
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS))
			{
				if (camera->AllowHdOcclusionCulling() || camera->AllowHzbOcclusionCulling())
				{
					depthTest.viewProj.StoreXM(XMMatrixTranspose(viewProj));
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS>(&depthTest);
				}
			}
			//HzbOccCompute
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS))
			{
				if (camera->AllowHzbOcclusionCulling())
				{
					hzb.view.StoreXM(XMMatrixTranspose(view));
					hzb.proj.StoreXM(XMMatrixTranspose(proj));

					const BoundingFrustum frustum = camera->GetBoundingFrustum();
					XMVECTOR planeV[6];
					frustum.GetPlanes(&planeV[0], &planeV[1], &planeV[2], &planeV[3], &planeV[4], &planeV[5]);
					for (uint i = 0; i < 6; ++i)
						XMStoreFloat4(&hzb.frustumPlane[i], planeV[i]);

					hzb.frustumDir = frustum.Orientation;
					hzb.frustumPos = frustum.Origin;
					hzb.viewWidth = viewWidth;
					hzb.viewHeight = viewHeight;
					hzb.camNear = camNear;
					hzb.camFar = camFar;
					hzb.validQueryCount = set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT, sceneGuid);
					hzb.occMapCount = gUser->GetViewCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_TASK_TYPE::HZB_CULLING);
					//gUser->GetMipmapCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
					//set.info.resource.occlusionMapCount;
					hzb.occIndexOffset = JMathHelper::Log2Int(set.info.resource.occlusionMinSize);
					hzb.correctFailTrigger = (int)set.option.culling.allowHZBCorrectFail;
					hzb.usePerspective = true;
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS>(&hzb);
				}
			}
			//SSAO
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::SSAO_PASS))
			{
				if (camera->AllowSsao() && set.option.CanUseSSAO())
				{
					//if (ssaoDesc.blurKenelSize == Graphic::J_KERNEL_SIZE::_5x5)
					//{
					//	using namespace Graphic;
					//	JFilter::ComputeFilter<JKenelType::Size<J_KERNEL_SIZE::_5x5>(), JKenelType::MaxSize(), true, true>(constant.kernel, &JFilter::Gaus, ssaoDesc.blurIntensity);
					//}
					//else
					//{
					//	using namespace Graphic;
					////	JFilter::ComputeFilter<JKenelType::Size<J_KERNEL_SIZE::_3x3>(), JKenelType::MaxSize(), true, true>(constant.kernel, &JFilter::Gaus, ssaoDesc.blurIntensity);
					//}

					const JSsaoDesc ssaoDesc = camera->GetSsaoDesc();

					ssao.camView.StoreXM(XMMatrixTranspose(view));
					ssao.camProj.StoreXM(XMMatrixTranspose(proj));

					ssao.radius = ssaoDesc.radius;
					ssao.radius2 = ssao.radius * ssao.radius;
					ssao.bias = ssaoDesc.bias;
					ssao.sharpness = ssaoDesc.sharpness;

					//x camRenderTargetWidth, y camRenderTargetHeight, z near, w far,
					ssao.camNearFar = JVector2F(camNear, camFar);

					ssao.camRtSize = rtSize;
					ssao.camInvRtSize = invRtSize;

					ssao.aoRtSize = ssao.camRtSize;
					ssao.aoInvRtSize = ssao.camInvRtSize;

					const JVector2F aoQuaterRtSize = (ssao.camRtSize + 4.0f - 1.0f) / 4.0f;
					ssao.aoInvQuaterRtSize = JVector2F(1.0f / aoQuaterRtSize.x, 1.0f / aoQuaterRtSize.y);

					float radiusToScreen = ssao.radius * 0.5f / camera->GetTanHalfFovY() * ssao.camRtSize.y;
					ssao.radiusToScreen = radiusToScreen;

					ssao.uvToViewA = uvToViewA;
					ssao.uvToViewB = uvToViewB;

					ssao.negInvR2 = -1.0f / ssao.radius2;
					//constant.tanBias = tan(30.0f * JMathHelper::DegToRad);
					ssao.tanBias = std::clamp(ssao.bias, tan(-45.0f * JMathHelper::DegToRad), tan(45.0f * JMathHelper::DegToRad));

					const float AOAmountScaleFactor = 1.0f / (1.0f - ssao.tanBias);
					ssao.smallScaleAOAmount = ssaoDesc.smallAoScale * AOAmountScaleFactor * 2.0f;
					ssao.largeScaleAOAmount = ssaoDesc.largeAoScale * AOAmountScaleFactor;
					ssao.viewPortTopLeft = JVector2F::Zero();
					ssao.camNearMulFar = camNear * camFar;
					//constant.depthThresholdNegInv = -1.0f / JMathHelper::Epsilon;
					//constant.depthThresholdSharpness = Max(Params.Sharpness, 0.f);	
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SSAO_PASS>(&ssao);
				}
			}
			//LightCulling
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::LIGHT_CULLING_PASS))
			{
				if (camera->AllowLightCulling() && set.option.culling.allowLightCluster)
				{
					lightCulling.camView.StoreXM(XMMatrixTranspose(view));
					lightCulling.camProj.StoreXM(XMMatrixTranspose(proj));

					lightCulling.camRenderTargetSize = rtSize;
					lightCulling.camInvRenderTargetSize = invRtSize;
					lightCulling.camNearZ = camNear;
					lightCulling.camFarZ = camFar;
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::LIGHT_CULLING_PASS>(&lightCulling);
				}
			}

			//나중에 수정
			//Camera관련 data를 pass용 data를 분리하거나 지금처럼 subclass에서 자체적으로 upload할것.
			/*if (camera->AllowRaytracingGI() && set.option.CanUseRtGi())
			{
				const JVector2F camHalfRtSize = rtSize * 0.5f;

				const uint directionalLitCount = set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT, sceneGuid);
				const uint pointLitCount = set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT, sceneGuid);
				const uint spotLitCount = set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT, sceneGuid);
				const uint rectLitCount = set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT, sceneGuid);
				const uint lightSum = directionalLitCount + pointLitCount + spotLitCount + rectLitCount;

				const uint directionalLitOffset = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT, sceneGuid);
				const uint pointLitOffset = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT, sceneGuid);
				const uint spotLitOffset = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT, sceneGuid);
				const uint rectLitOffset = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT, sceneGuid);

				gi.camInvView.StoreXM(DirectX::XMMatrixTranspose(invView));
				gi.camPreViewProj.StoreXM(DirectX::XMMatrixTranspose(preViewProj));
				gi.camNearFar = JVector2F(camNear, camFar);
				gi.uvToViewA = uvToViewA;
				gi.uvToViewB = uvToViewB;
				gi.rtSize = camHalfRtSize;
				gi.invRtSize = 1.0f / camHalfRtSize;
				gi.origianlRtSize = rtSize;
				gi.invOrigianlRtSize = 1.0f / rtSize;
				gi.tMax = (JVector3F(camera->GetOwner()->GetOwnerScene()->GetSceneBBox().Extents) * 2).Length();
				gi.totalNumPixels = camHalfRtSize.x * camHalfRtSize.y;

				gi.camPosW = posW;
				gi.camNearMulFar = gi.camNearFar.x * gi.camNearFar.y;

				gi.directionalLightRange = directionalLitCount;
				gi.pointLightRange = gi.directionalLightRange + pointLitCount;
				gi.spotLightRange = gi.pointLightRange + spotLitCount;
				gi.rectLightRange = gi.spotLightRange + rectLitCount;

				gi.directionalLightOffset = directionalLitOffset;
				gi.pointLightOffset = pointLitOffset;
				gi.spotLightOffset = spotLitOffset;
				gi.rectLightOffset = rectLitOffset;

				gi.totalLightCount = lightSum;
				gi.invTotalLightCount = 1.0f / (float)gi.totalLightCount;
				//constants.rectLightVerticesIndex = rectLitShape->GraphicResourceUserInterface().GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::VERTEX, J_GRAPHIC_BIND_TYPE::SRV, 0);
				//constants.rectLightIndiciesIndex = rectLitShape->GraphicResourceUserInterface().GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::INDEX, J_GRAPHIC_BIND_TYPE::SRV, 0);

				denoise.camInvView = drawScene.invView;
				denoise.camPreInvView.StoreXM(DirectX::XMMatrixTranspose(camPreInvView.LoadXM()));
				denoise.camPreViewProj = drawScene.preViewProj;
				denoise.rtSize = drawScene.renderTargetSize;
				denoise.invRtSize = drawScene.invRenderTargetSize;
				denoise.uvToViewA = uvToViewA;
				denoise.uvToViewB = uvToViewB;
				denoise.preUvToViewA = preUvToViewA;
				denoise.preUvToViewB = preUvToViewB;
				denoise.camNearFar = JVector2F(camNear, camFar);
				denoise.camNearMulFar = denoise.camNearFar.x * denoise.camNearFar.y;
				//denoise.denoiseRange = Common::denoiseRange;
				//denoise.baseRadius = Common::baseRadius;
				//denoise.radiusRange = Common::radiusRange;
				//++denoise.sampleNumber;
				//if (denoise.sampleNumber >= Common::sampleNumberMax)
				//	denoise.sampleNumber = 0;
			}*/
		}
		template<int threadIndex>
		static void UpdateDirctionalLight(const InnerUpdateDataSet& set)
		{
			JDirectionalLight* light = static_cast<JDirectionalLight*>(set.obj);
			auto gUser = light->ModuleManagedData()->GetGraphicResourceUserInterface();
			auto csmUser = light->ModuleManagedData()->GetCsmHandleUserInterface();

			static JDirectionalLightConstants litConstants;
			static std::vector<JCsmConstants> csm;
			static JShadowMapDrawConstants shadowMapDraw;
			static std::vector<JShadowMapArrayDrawConstants> shadowMapArrayDraw;
			static JDepthTestConstants depthTest;
			static JHzbOccComputeConstants hzb;

			const size_t sceneGuid = light->GetOwner()->GetOwnerGuid();

			const XMMATRIX viewM = light->GetView().LoadXM();
			const XMMATRIX projM = light->GetProj().LoadXM();

			const JVector3F frustumMinP = light->GetFrustumMinPoint();
			const JVector3F frustumMaxP = light->GetFrustumMaxPoint();
			const JVector2F frustumSize(abs(frustumMaxP.x - frustumMinP.x), abs(frustumMaxP.y - frustumMinP.y));

			const bool isShadowMapActivated = light->IsShadowActivated();
			const bool isCsmActivated = light->IsCsmActivated() && isShadowMapActivated;

			//Directional Light
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT))
			{
				litConstants.view.StoreXM(XMMatrixTranspose(viewM));
				litConstants.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM, projM)));

				if (isCsmActivated)
				{
					litConstants.shadowMapTransform.StoreXM(XMMatrixTranspose(light->GetShadowMapTransform()));
					//constant.shadowMapTransform.StoreXM(XMMatrixTranspose(viewM));
					litConstants.shadowMapIndex = gUser->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, 0);
					litConstants.csmDataIndex = set.updateInterface->GetFrameIndex(J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO);
				}
				else if (isShadowMapActivated)
				{
					litConstants.shadowMapTransform.StoreXM(XMMatrixTranspose(light->GetShadowMapTransform()));
					litConstants.shadowMapIndex = gUser->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0);
				}
				litConstants.shadowMapType = (uint)light->GetShadowMapType();
				litConstants.color = light->GetColor();
				litConstants.power = light->GetPower();
				litConstants.direction = light->GetCachedWorldDirection();
				litConstants.frustumSize = frustumSize;
				litConstants.frustumNear = frustumMinP.z;
				litConstants.frustumFar = frustumMaxP.z;
				litConstants.penumbraScale = light->GetPenumbraWidth();
				litConstants.penumbraBlockerScale = light->GetPenumbraBlockerWidth();
				litConstants.shadowMapSize = light->GetShadowMapSize();
				litConstants.shadowMapInvSize = 1.0f / litConstants.shadowMapSize;
				litConstants.tanAngle = XMVectorGetX(XMVector3AngleBetweenNormals(litConstants.direction.ToXmV(), light->GetInitWorldDirection().ToXmV()));
				litConstants.bias = light->GetBias();

				static constexpr float initDirBias = 0.0025f;
				if (litConstants.direction == light->GetInitWorldDirection())
					litConstants.bias -= initDirBias;

				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT>(&litConstants);

			}
			//Csm
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO) || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW))
			{
				if (isCsmActivated)
				{
					const uint targetCount = csmUser->GetTargetCount();
					ControlStaticConstantsSize(csm, targetCount);
					ControlStaticConstantsSize(shadowMapArrayDraw, targetCount);

					for (uint i = 0; i < targetCount; ++i)
					{
						const auto& result = csmUser->GetComputeResult(i);
						for (uint j = 0; j < result.subFrustumCount; ++j)
						{
							csm[i].scale[j] = result.scale[j];
							csm[i].posOffset[j] = result.posOffset[j];
							csm[i].frustumNear[j] = result.fNear[j];
							csm[i].frustumFar[j] = result.fFar[j];
						}
						csm[i].mapMinBorder = (float)(1.0f / (float)light->GetShadowMapSize());
						csm[i].mapMaxBorder = (float)(((float)light->GetShadowMapSize() - 1.0f) / (float)light->GetShadowMapSize());
						csm[i].levelBlendRate = csmUser->GetOption().GetLevelBlendRate();
						csm[i].count = result.subFrustumCount;

						for (uint j = 0; j < result.subFrustumCount; ++j)
						{
							shadowMapArrayDraw[i].shadowMapTransform[j].StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM,
								result.shadowProjM[j].LoadXM())));
						}
					}
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO>(csm, targetCount);
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW>(shadowMapArrayDraw, targetCount);
				}
			}
			//Shdaow map
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW))
			{
				if (isShadowMapActivated && !isCsmActivated)
				{
					shadowMapDraw.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM, projM)));
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW>(&shadowMapDraw);
				}
			}
			//Hzb pass
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS))
			{
				if (light->AllowHzbOcclusionCulling())
				{
					depthTest.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM, projM)));

					hzb.view.StoreXM(XMMatrixTranspose(viewM));
					hzb.proj.StoreXM(XMMatrixTranspose(projM));

					hzb.viewWidth = frustumMaxP.x - frustumMinP.x;
					hzb.viewHeight = frustumMaxP.y - frustumMinP.y;
					hzb.camNear = frustumMinP.z;
					hzb.camFar = frustumMaxP.z;
					hzb.validQueryCount = set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT, sceneGuid);
					hzb.occMapCount = gUser->GetViewCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_TASK_TYPE::HZB_CULLING);
					hzb.occIndexOffset = JMathHelper::Log2Int(set.info.resource.occlusionMinSize);
					hzb.correctFailTrigger = (int)set.option.culling.allowHZBCorrectFail;
					hzb.usePerspective = false;
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS>(&hzb);
				}
			}
		}
		template<int threadIndex>
		static void UpdatePointLight(const InnerUpdateDataSet& set)
		{
			JPointLight* light = static_cast<JPointLight*>(set.obj);
			auto gUser = light->ModuleManagedData()->GetGraphicResourceUserInterface();

			static JPointLightConstants litConstants;
			static JShadowMapCubeDrawConstants shadowMapDraw;

			//shadow map index에 대한 변수가 있으므로
			//shadow map update시 JPointLightConstants와 JShadowMapCubeDrawConstants를 동시에
			//update해줘야한다. 
			const XMMATRIX proj = light->GetProj().LoadXM();
			const XMMATRIX ndcM = JMatrix4x4::NdcToTextureSpaceXM();

			const bool canUploadLight = set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT);
			const bool canUploadShadow = set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_CUBE_DRAW);

			//Calucate shadow map transform
			if (canUploadLight || canUploadShadow)
			{
				for (uint i = 0; i < Graphic::Constants::cubeMapPlaneCount; ++i)
				{
					const XMMATRIX viewProj = XMMatrixMultiply(light->GetView(i).LoadXM(), proj);
					litConstants.shadowMapTransform[i].StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewProj, ndcM)));
					shadowMapDraw.shadowMapTransform[i].StoreXM(XMMatrixTranspose(viewProj));
				}
			}
			//Point Light
			if (canUploadLight)
			{
				light->GetSidePosition(litConstants.sidePosition[0], litConstants.sidePosition[1]);
				litConstants.midPosition = light->GetOwner()->GetTransform()->GetWorldPosition();
				litConstants.color = light->GetColor();
				litConstants.power = light->GetPower();
				litConstants.frustumNear = light->GetFrustumNear();
				litConstants.frustumFar = light->GetFrustumFar();
				litConstants.radius = light->GetRadius();
				litConstants.penumbraScale = light->GetPenumbraWidth();
				litConstants.penumbraBlockerScale = light->GetPenumbraBlockerWidth();
				litConstants.shadowMapIndex = light->IsShadowActivated() ? gUser->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE, 0) : 0;
				litConstants.hasShadowMap = light->IsShadowActivated();
				litConstants.shadowMapSize = light->GetShadowMapSize();
				litConstants.shadowMapInvSize = 1.0f / litConstants.shadowMapSize;
				litConstants.bias = light->GetBias();
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT>(&litConstants);
			}
			//Shadow map cube
			if (canUploadShadow)
			{
				if (light->IsShadowActivated())
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_CUBE_DRAW>(&shadowMapDraw);
			}
		}
		template<int threadIndex>
		static void UpdateSpotLight(const InnerUpdateDataSet& set)
		{
			JSpotLight* light = static_cast<JSpotLight*>(set.obj);
			auto gUser = light->ModuleManagedData()->GetGraphicResourceUserInterface();

			static JSpotLightConstants litConstants;
			static JShadowMapDrawConstants shadowMapDraw;

			const XMMATRIX view = light->GetView().LoadXM();
			const XMMATRIX proj = light->GetProj().LoadXM();

			const bool canUploadLight = set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT);
			const bool canUploadShadow = set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);

			//Calucate shadow map transform
			if (canUploadLight || canUploadShadow)
			{
				if (light->IsShadowActivated())
				{
					const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
					litConstants.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewProj, JMatrix4x4::NdcToTextureSpaceXM())));
					shadowMapDraw.shadowMapTransform.StoreXM(XMMatrixTranspose(viewProj));
				}

			}
			//Point Light
			if (canUploadLight)
			{
				litConstants.color = light->GetColor();
				litConstants.power = light->GetPower();
				litConstants.position = light->GetWorldPosition();

				litConstants.frustumNear = light->GetFrustumNear();
				litConstants.direction = light->GetCachedWorldDirection();
				litConstants.frustumFar = light->GetFrustumFar();
				litConstants.innerConeCosAngle = cos(light->GetInnerConeAngle());
				litConstants.outerConeCosAngle = cos(light->GetOuterConeAngle());
				litConstants.outerConeAngle = light->GetOuterConeAngle();
				litConstants.penumbraScale = light->GetPenumbraWidth();
				litConstants.penumbraBlockerScale = light->GetPenumbraBlockerWidth();
				litConstants.shadowMapIndex = light->IsShadowActivated() ? gUser->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0) : 0;
				litConstants.hasShadowMap = light->IsShadowActivated();
				litConstants.shadowMapSize = light->GetShadowMapSize();
				litConstants.shadowMapInvSize = 1.0f / litConstants.shadowMapSize;
				litConstants.bias = light->GetBias();
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT>(&litConstants);
			}
			//Shadow map
			if (canUploadShadow)
			{
				if (light->IsShadowActivated())
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW>(&shadowMapDraw);
			}
		}
		template<int threadIndex>
		static void UpdateRectLight(const InnerUpdateDataSet& set)
		{
			JRectLight* light = static_cast<JRectLight*>(set.obj);
			auto gUser = light->ModuleManagedData()->GetGraphicResourceUserInterface();

			static JRectLightConstants litConstants;
			static JShadowMapDrawConstants shadowMapDraw;
			static constexpr uint missingIndex = Graphic::Constants::missingIndex;

			const XMMATRIX view = light->GetView().LoadXM();
			const XMMATRIX proj = light->GetProj().LoadXM(); ;
			const JUserPtr<JTexture> sourceTexture = light->GetSourceTexture();

			const bool canUploadLight = set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT);
			const bool canUploadShadow = set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);

			//Calucate shadow map transform
			if (canUploadLight || canUploadShadow)
			{
				if (light->IsShadowActivated())
				{
					const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
					litConstants.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewProj, JMatrix4x4::NdcToTextureSpaceXM())));
					shadowMapDraw.shadowMapTransform.StoreXM(XMMatrixTranspose(viewProj));
				}
			}
			//Rect Light
			if (canUploadLight)
			{
				litConstants.origin = light->GetOwner()->GetTransform()->GetWorldPosition();
				litConstants.extents = light->GetAreaSize() * 0.5f;
				light->GetWorldAxis(litConstants.axis[0], litConstants.axis[1], litConstants.axis[2]);

				litConstants.direction = light->GetCachedWorldDirection();
				litConstants.color = light->GetColor();
				litConstants.power = light->GetPower();
				litConstants.frustumNear = light->GetFrustumNear();
				litConstants.frustumFar = light->GetFrustumFar();
				litConstants.barndoorLength = light->GetBarndoorLength();
				litConstants.barndoorCosAngle = std::cos(JMathHelper::DegToRad * light->GetBarndoorAngle());
				//constant.isTwoSide = isTwoSide;
				litConstants.shadowMapIndex = light->IsShadowActivated() ? gUser->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0) : 0;
				litConstants.hasShadowMap = light->IsShadowActivated();
				litConstants.sourceTextureIndex = sourceTexture != nullptr ?
					sourceTexture->ModuleManagedData()->GetGraphicResourceUserInterface()->GetFirstResourceArrayIndex() :
					missingIndex;

				JUserPtr<JTexture>& ltcMat = set.cacheData->ltcMat;
				JUserPtr<JTexture>& ltcAmp = set.cacheData->ltcAmp;

				litConstants.ltcMatTextureIndex = ltcMat != nullptr ? ltcMat->ModuleManagedData()->GetGraphicResourceUserInterface()->GetFirstResourceArrayIndex() : invalidIndex;
				litConstants.ltcAmpTextureIndex = ltcAmp != nullptr ? ltcAmp->ModuleManagedData()->GetGraphicResourceUserInterface()->GetFirstResourceArrayIndex() : invalidIndex;
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT>(&litConstants);
			}
			//Shadow map
			if (canUploadShadow)
			{
				if (light->IsShadowActivated())
					set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW>(&shadowMapDraw);
			}
		}
		template<int threadIndex>
		static void UpdateRenderItem(const InnerUpdateDataSet& set)
		{
			//0.0015ms ~ 0.0035ms 
			JRenderItem* rItem = static_cast<JRenderItem*>(set.obj);
			JTransform* transform = rItem->GetOwner()->GetTransform().Get();
			auto gUser = rItem->ModuleManagedData()->GetGraphicResourceUserInterface();

			static std::vector<JObjectCpuConstants> object;
			static JBoundingObjectConstants bounding;
			static JHzbOccObjectConstants hzb;
			static std::vector<JObjectRefereneceInfoConstants> refInfo;

			const uint subMeshCount = rItem->GetSubmeshCount();
			ControlStaticConstantsSize(object, subMeshCount);
			ControlStaticConstantsSize(refInfo, subMeshCount);
	 
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT) || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT_REF_INFO))
			{
				const JMatrix4x4 textureTransform = rItem->GetTextransform();
				for (uint i = 0; i < subMeshCount; ++i)
				{
					const JUserPtr<JMaterial>& material = rItem->GetValidMaterial(i);
					object[i].world.StoreXM(XMMatrixTranspose(transform->GetWorldMatrix().LoadXM()));
					object[i].texTransform.StoreXM(XMMatrixTranspose(textureTransform.LoadXM()));
					object[i].materialIndex = material->ModuleManagedData()->GetFrameUpdateUserInterface()->GetFrameIndex(J_FRAME_RESOURCE_UPLOAD_TYPE::MATERIAL);

					const JUserPtr<JMeshGeometry>& mesh = rItem->GetMesh();
					auto meshGUser = mesh->ModuleManagedData()->GetGraphicResourceUserInterface();
					refInfo[i].materialIndex = object[i].materialIndex;
					refInfo[i].verticesIndex = meshGUser->GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::VERTEX, J_GRAPHIC_BIND_TYPE::SRV, 0);
					refInfo[i].indicesIndex = meshGUser->GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::INDEX, J_GRAPHIC_BIND_TYPE::SRV, 0);
					refInfo[i].verticesOffset = mesh->GetSubmeshBaseVertexLocation(i);
					refInfo[i].indicesOffset = mesh->GetSubmeshStartIndexLocation(i);
					refInfo[i].verticesType = (uint)mesh->GetMeshGeometryType();
					refInfo[i].indicesType = mesh->GetIndexByteSize() == sizeof(uint16) ? 0 : 1;
				}
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT>(object, subMeshCount);
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT_REF_INFO>(refInfo, subMeshCount);
			}
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT))
			{
				bounding.boundWorld.StoreXM(XMMatrixTranspose(rItem->GetBBoxWorldMaxtrix()));
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT>(&bounding);
			}
			if (set.isFrameDirted || set.HasMoveDirty(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT))
			{
				const DirectX::BoundingOrientedBox bbox = rItem->GetOrientedBoundingBox();
				bbox.GetCorners(hzb.coners);
				hzb.center = bbox.Center;
				hzb.extents = bbox.Extents;
				hzb.isValid = rItem->GetRenderLayer() == J_RENDER_LAYER::OPAQUE_OBJECT;
				hzb.queryResultIndex = set.updateInterface->GetFrameIndex(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT);

				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT>(&hzb);
			}
		}
		template<int threadIndex>
		static void UpdateMaterial(const InnerUpdateDataSet& set)
		{
			//0.001ms 
			JMaterial* mat = static_cast<JMaterial*>(set.obj);

			static JMaterialConstants matConstants;
			static constexpr uint missingIndex = Graphic::Constants::missingIndex;

			auto TryGetResourceArrayIndex = [](const JUserPtr<JTexture>& texture, const int failReturn = invalidIndex)
			{
				return texture != nullptr ? texture->ModuleManagedData()->GetGraphicResourceUserInterface()->GetFirstResourceArrayIndex() : failReturn;
			};

			matConstants.albedoColor = mat->GetAlbedoColor();
			matConstants.metallic = mat->GetMetallic();
			matConstants.roughness = mat->GetRoughness();
			matConstants.specularFactor = mat->GetSpecularFactor();
			matConstants.matTransform.StoreXM(XMMatrixTranspose(mat->GetMatTransform().LoadXM()));
			matConstants.albedoMapIndex = TryGetResourceArrayIndex(mat->GetAlbedoMap(), missingIndex);
			matConstants.normalMapIndex = TryGetResourceArrayIndex(mat->GetNormalMap(), missingIndex);
			matConstants.heightMapIndex = TryGetResourceArrayIndex(mat->GetHeightMap(), missingIndex);
			matConstants.metallicMapIndex = TryGetResourceArrayIndex(mat->GetMetallicMap(), missingIndex);
			matConstants.roughnessMapIndex = TryGetResourceArrayIndex(mat->GetRoughnessMap(), missingIndex);
			matConstants.ambientOcclusionMapIndex = TryGetResourceArrayIndex(mat->GetAmbientOcclusionMap(), missingIndex);
			matConstants.specularMapIndex = TryGetResourceArrayIndex(mat->GetSpecularMap(), missingIndex);

			set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::MATERIAL>(&matConstants);
		}
		template<int threadIndex>
		static void UpdateScene(const InnerUpdateDataSet& set)
		{
			JScene* scene = static_cast<JScene*>(set.obj);
			const size_t sceneGuid = scene->GetGuid();
			static JScenePassConstants constants;

			constants.appTotalTime = JEngineTimer::Data().TotalTime();
			constants.appDeltaTime = JEngineTimer::Data().DeltaTime();
			if (scene->IsActivatedSceneTime())
			{
				constants.sceneTotalTime = scene->GetTotalTime();
				constants.sceneDeltaTime = scene->GetDeltaTime();
			}
			else
			{
				constants.sceneTotalTime = 0;
				constants.sceneDeltaTime = 0;
			}

			constants.directionalLitSt = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT, sceneGuid);
			constants.directionalLitEd = constants.directionalLitSt + set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT, sceneGuid);
			constants.pointLitSt = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT, sceneGuid);
			constants.pointLitEd = constants.pointLitSt + set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT, sceneGuid);

			constants.spotLitSt = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT, sceneGuid);
			constants.spotLitEd = constants.spotLitSt + set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT, sceneGuid);
			constants.rectLitSt = set.fm->GetAreaRegistedOffset(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT, sceneGuid);
			constants.rectLitEd = constants.rectLitSt + set.fm->GetAreaRegistedCount(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT, sceneGuid);

			auto missingInterface = set.cacheData->missing->ModuleManagedData()->GetGraphicResourceUserInterface();
			auto blueNoiseInterface = set.cacheData->bluseNoise->ModuleManagedData()->GetGraphicResourceUserInterface();

			constants.missingTextureIndex = missingInterface->GetFirstResourceArrayIndex();
			constants.bluseNoiseTextureIndex = blueNoiseInterface->GetFirstResourceArrayIndex();
			constants.bluseNoiseTextureSize = blueNoiseInterface->GetFirstResourceSize();
			constants.invBluseNoiseTextureSize = blueNoiseInterface->GetFirstResourceInvSize();
			constants.clusterMinDepth = std::log2(set.option.culling.clusterNear);

			set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SCENE_PASS>(&constants);
		}
 												
		template<size_t ...Is>
		static void CreateUpdaetFunc(InnerUpdateFuncSet& set, const JObjectDataSetMetadata& metadata, std::index_sequence<Is...>)
		{
			const UniqueIndex index = metadata.uniqueIndex;

			switch (index)
			{
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_ANIMATOR>():
			{ 
				((set.updateF[Is]= &UpdateAnimator<Is>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm) {return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::ANIMATION); };
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount)
				{
					return (option.isActivatedSceneTimer ? aniThreadBorder : aniOnlyBindPoseThreadBorder) <= taskCount;
				}; 
				break;
			}
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::USER_BEHAVIOR>():
			{
				((set.updateF[Is] = &UpdateBehavior<0>), ...);
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm) {return false; };
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>():
			{
				((set.updateF[Is]= &UpdateCamera<0>), ...);
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm)
				{
					return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_COMPUTE_PASS);
				}; 
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::DIRECTIONAL):
			{
				((set.updateF[Is]= &UpdateDirctionalLight<0>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm)
				{
					return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::CASCADE_SHADOW_MAP_INFO) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);
				}; 
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::POINT):
			{
				((set.updateF[Is]= &UpdatePointLight<0>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm)
				{
					return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_CUBE_DRAW);
				}; 
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::SPOT):
			{
				((set.updateF[Is]= &UpdateSpotLight<0>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm)
				{
					return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);
				}; 
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_LIGHT>(J_LIGHT_TYPE::RECT):
			{
				((set.updateF[Is]= &UpdateRectLight<0>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm)
				{
					return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW);
				};
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			case ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_RENDERITEM>():
			{
				((set.updateF[Is]= &UpdateRenderItem<0>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm)
				{
					return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::OBJECT) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT) ||
						fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT);
				}; 
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			default:
				break;
			}
			switch (index)
			{
			case ConvertResourceUniqueIndex<J_RESOURCE_TYPE::MATERIAL>(totalCompVariation):
			{
				((set.updateF[Is]= &UpdateMaterial<0>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm) {return fm->IsForcedUpdate(J_FRAME_RESOURCE_UPLOAD_TYPE::MATERIAL); };
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			case ConvertResourceUniqueIndex<J_RESOURCE_TYPE::SCENE>(totalCompVariation):
			{
				((set.updateF[Is]= &UpdateScene<0>), ...); 
				set.isForcedUpdateF = [](JDx12FrameResourceManager* fm) {return true; };
				set.canUseMultiThread = [](const JFrameUpdateOption& option, const uint taskCount) {return false; };
				break;
			}
			default:
				break;
			}
		}
		static InnerUpdateFuncSet GetUpdateFuncSet(const JObjectDataSetMetadata& metadata)
		{
			static constexpr uint variation = totalCompVariation + totalResourceVariation;
			static InnerUpdateFuncSet set[variation];
			static std::bitset<variation> isInit;

			if (!isInit[metadata.uniqueIndex])
			{ 
				CreateUpdaetFunc(set[metadata.uniqueIndex], metadata, std::make_index_sequence <maxNumOfUpdateThread>());	    
				isInit[metadata.uniqueIndex] = true;
			}
			return set[metadata.uniqueIndex];
		}
	 
		template<typename ObjectStucture>
		static void DoUpdate(JDx12FrameResourceManager* fm, JFrameUpdateDataSet& set, InnerUpdateDataSet& dataSet, InnerUpdateFuncSet& funcSet)
		{  
			const bool isForcedUpdate = funcSet.isForcedUpdateF(fm);
			const uint count = (uint)set.GetDataStorageCount();
			 
			for (uint i = 0; i < count; ++i)
			{
				JFrameUpdateInterface* frameInterface = nullptr;
				JObject* obj = nullptr;
				if constexpr (std::is_same_v<ObjectStucture, JFrameUpdateDataSet::CompVec>)
				{
					JComponent* comp = (*set.compVec)[i].UnsafeGet();
					frameInterface = static_cast<JFrameUpdateInterface*>(comp->ModuleManagedData()->GetFrameUpdateUserInterface());
					obj = comp;
				}
				else
				{
					JGraphicObjectDataSetBase* objSet = set.objDataVec->Get(i)->Get();
					frameInterface = objSet->GetFrameUpdateInterface();
					obj = objSet->Object().UnsafeGet();
				}

				JFrameDirtyBase* frameDirty = frameInterface->GetDirtyBase();
				frameDirty->BeginUpdate();

				if (isForcedUpdate)
					frameInterface->SetFrameDirty();

				frameInterface->TryExecuteObjectAlwaysUpdateBind();
				if (frameInterface->IsDirted())
				{
					if (frameDirty->IsFrameHotDirted())
					{
						frameInterface->TryExecuteObjectHotUpdateBind();
						++set.updateLog.hotUpdateCount;
					}
					++set.updateLog.updateCount;

					dataSet.obj = obj;
					dataSet.updateInterface = frameInterface;
					dataSet.isFrameDirted = true;
					funcSet.updateF[0](dataSet);
				}
				else if (dataSet.hasMoveDirted)
				{
					++set.updateLog.moveCount;
					dataSet.obj = obj;
					dataSet.updateInterface = frameInterface;
					dataSet.isFrameDirted = false;
					funcSet.updateF[0](dataSet);
				}
				frameDirty->EndUpdate();
			}
		}

		template<typename ObjectStucture>
		static void DoAlwaysUpdate(const JFrameUpdateDataSet& set,
			InnerUpdateDataSet dataSet,
			const InnerUpdateFuncSet& funcSet,
			uint threadIndex,
			uint threadCount)
		{
			if (funcSet.updateF == nullptr)
				return;
			   
			uint st, ed;
			Core::JThreadUtil::DispatchWorkIndex(set.GetDataStorageCount(), threadCount, threadIndex, st, ed);
			 
			for (uint i = st; i < ed; ++i)
			{ 
				JFrameUpdateInterface* frameInterface = nullptr;
				JObject* obj = nullptr;
				if constexpr (std::is_same_v<ObjectStucture, JFrameUpdateDataSet::CompVec>)
				{
					JComponent* comp = (*set.compVec)[i].UnsafeGet();
					frameInterface = static_cast<JFrameUpdateInterface*>(comp->ModuleManagedData()->GetFrameUpdateUserInterface());
					obj = comp;
				}
				else
				{
					JGraphicObjectDataSetBase* objSet = set.objDataVec->Get(i)->Get();
					frameInterface = objSet->GetFrameUpdateInterface();
					obj = objSet->Object().UnsafeGet();
				}

				JFrameDirtyBase* frameDirty = frameInterface->GetDirtyBase();
				frameDirty->BeginUpdate();
				 
				dataSet.obj = obj;
				dataSet.updateInterface = frameInterface;
				dataSet.isFrameDirted = true;
				funcSet.updateF[threadIndex](dataSet);

				frameDirty->EndUpdate();
			}
		}
	}

	void JDx12FrameResourceManager::CacheData::Initialize()
	{
		missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING);
		bluseNoise = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::BLUE_NOISE);
		ltcAmp = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_AMP);
		ltcMat = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_MAT);
	}
	void JDx12FrameResourceManager::CacheData::Clear()
	{
		missing = nullptr;
		bluseNoise = nullptr;
		ltcAmp = nullptr;
		ltcMat = nullptr;
	}
	void JDx12FrameResourceManager::CacheData::Update()
	{
		if (missing == nullptr)
			missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING);
		if (bluseNoise == nullptr)
			bluseNoise = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::BLUE_NOISE);
		if (ltcAmp == nullptr)
			ltcAmp = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_AMP);
		if (ltcMat == nullptr)
			ltcMat = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_MAT);
	}

	JDx12FrameResourceManager::~JDx12FrameResourceManager()
	{
		ClearResource();
	}
	void JDx12FrameResourceManager::Initialize(JGraphicDevice* device)
	{
		if (!IsSameDevice(device))
			return;

		JFrameResourceManager::Initialize(device);
		BuildResource(device); 

		workerFunctor = std::make_unique<WorkerF::Functor>(&JDx12FrameResourceManager::WorkerThread, this);
	}
	void JDx12FrameResourceManager::Clear()
	{
		workerFunctor = nullptr;

		ClearResource();
		JFrameResourceManager::Clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12FrameResourceManager::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	JFrameResource* JDx12FrameResourceManager::GetCurrentFrameResource() noexcept
	{
		return &resource[currResourceIndex];
	}
	JDx12FrameResource* JDx12FrameResourceManager::GetCurrentDxFrameResource() noexcept
	{
		return &resource[currResourceIndex];
	}
	JFrameResource* JDx12FrameResourceManager::GetFrameResource(const uint index) noexcept
	{
		return &resource[index];
	}
	uint JDx12FrameResourceManager::GetCurrentFrameIndex() const noexcept
	{
		return currResourceIndex;
	}
	uint JDx12FrameResourceManager::GetNextFrameIndex()const noexcept
	{
		return (currResourceIndex + 1) % Constants::gNumFrameResources;
	}
	uint JDx12FrameResourceManager::GetTotalRegistedCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		return (uint)updateInfoVec[(uint)type].size();
	}
	uint JDx12FrameResourceManager::GetTotalFrameCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		//만약 frame holderVec[index].ptr이 nullptr 이면
		//memory extend시에 호출되는 NotifyReAlloc에서 ReRegister를 호출하지 않았는지 확인해보자.
		//2024-07-27 object에 impl이 더 이상 FrameInterface를 상속받지 않고 Graphic module에서 관리하기 때문에 위에 사항은 유효하지않다.

		const uint registeredCount = GetTotalRegistedCount(type);
		if (registeredCount == 0)
			return 0;

		JFrameUpdateInfo* lastInfo = updateInfoVec[(uint)type][registeredCount - 1].Get();
		return lastInfo->GetFrameIndex() + lastInfo->GetFrameIndexSize();
	}
	uint JDx12FrameResourceManager::GetAreaRegistedCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept
	{
		int index = GetAreaVecIndex(type, areaGuid);
		return index != invalidIndex ? areaInfoVec[(int)type][index]->GetInfoCount() : 0;
	}
	//count 0 ~ areaStart
	uint JDx12FrameResourceManager::GetAreaRegistedOffset(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept
	{
		int result = GetAreaStIndex(type, areaGuid);
		return result == invalidIndex ? 0 : result;
	}
	uint JDx12FrameResourceManager::GetFrameResourceCapacity(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		return resource[currResourceIndex].GetElementCount(type);
	}
	int JDx12FrameResourceManager::GetAreaVecIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid) const noexcept
	{
		const AreaInfoVec& vec = areaInfoVec[(uint)type];
		const uint count = (uint)vec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (vec[i]->guid == areaGuid)
				return i;
		}
		return invalidIndex;
	}
	const JFrameUpdateAreaInfo* JDx12FrameResourceManager::GetAreaInfo(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept
	{
		int areaIndex = GetAreaVecIndex(type, areaGuid);
		return areaIndex != invalidIndex ? areaInfoVec[(uint)type][areaIndex].get() : nullptr;
	}
	int JDx12FrameResourceManager::GetAreaStIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept
	{
		const JFrameUpdateAreaInfo* areaInfo = GetAreaInfo(type, areaGuid);
		return areaInfo != nullptr ? areaInfo->GetStIndex() : invalidIndex;
	}
	int JDx12FrameResourceManager::GetArrayIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type, JFrameUpdateInfo* ptr)const noexcept
	{
		const InfoVec& vec = updateInfoVec[(uint)type];
		const uint count = (uint)vec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (vec[i].Get() == ptr)
				return i;
		}
		return invalidIndex;
	}
	void JDx12FrameResourceManager::SetNextFrameResource()
	{
		currResourceIndex = GetNextFrameIndex();
	}
	JUserPtr<JFrameUpdateInfo> JDx12FrameResourceManager::Register(const JFrameUploadDataCreationDesc& desc)
	{
		const int areaIndex = GetAreaVecIndex(desc.type, desc.areaGuid);
		if (areaIndex == invalidIndex)
			return PushBack(desc);
		else
		{
			auto user = Insert(desc, areaIndex);
			ReflectInsertNumber(user->GetType(), user->GetNumber());
			return user;
		}
	}
	bool JDx12FrameResourceManager::DeRegister(JFrameUpdateInfo* info)
	{
		if (info == nullptr || !info->HasValidFrameIndex())
			return false;

		ReflectPopNumber(info->GetType(), info->GetNumber());
		return Pop(info);
	}
	JUserPtr<JFrameUpdateInfo> JDx12FrameResourceManager::PushBack(const JFrameUploadDataCreationDesc& desc)
	{
		InfoVec& vec = updateInfoVec[(uint)desc.type];
		AreaInfoVec& areaVec = areaInfoVec[(uint)desc.type];
		const int count = vec.size();

		JFrameUpdateAreaInfo* beforeArea = nullptr;
		std::unique_ptr<JFrameUpdateAreaInfo> newAreaInfo = nullptr;
		if (areaVec.size() > 0)
		{
			beforeArea = areaVec[areaVec.size() - 1].get();
			newAreaInfo = std::make_unique<JFrameUpdateAreaInfo>(desc.areaGuid, beforeArea->GetEdIndex(), 1);
		}
		else
			newAreaInfo = std::make_unique<JFrameUpdateAreaInfo>(desc.areaGuid, 0, 1);

		JOwnerPtr<JDx12FrameUpdateInfo> newInfo = CreateInfo(desc, newAreaInfo.get());
		if (count == 0)
		{
			newInfo->SetFrameIndex(0);
			newInfo->SetNumber(0);
		}
		else
		{
			JFrameUpdateInfo* before = vec[count - 1].Get();
			newInfo->SetFrameIndex(before->GetFrameIndex() + before->GetFrameIndexSize());
			newInfo->SetNumber(before->GetNumber() + 1);
		}

		JUserPtr<JFrameUpdateInfo> result = newInfo;
		areaVec.push_back(std::move(newAreaInfo));
		vec.push_back(std::move(newInfo));
		return result;
	}
	JUserPtr<JFrameUpdateInfo> JDx12FrameResourceManager::Insert(const JFrameUploadDataCreationDesc& desc, const int areaIndex)
	{
		InfoVec& uVec = updateInfoVec[(uint)desc.type];
		AreaInfoVec& aVec = areaInfoVec[(uint)desc.type];
		JFrameUpdateAreaInfo* areaInfo = aVec[areaIndex].get();
		JOwnerPtr<JDx12FrameUpdateInfo> newInfo = CreateInfo(desc, areaInfo);

		int areaStIndex = areaInfo->GetStIndex();
		int areaEdIndex = areaInfo->GetEdIndex();

		uint8 sortOrder = newInfo->GetSortOrder();
		int insertIndex = -1;
		if (sortOrder != UCHAR_MAX)
		{
			for (int i = areaStIndex; i < areaEdIndex; ++i)
			{
				if (uVec[i]->GetSortOrder() > sortOrder)
				{
					insertIndex = i;
					break;
				}
			}
		}
		//holder->sortOrder == -1 or holder->sortOrder is last
		if (insertIndex == invalidIndex)
			insertIndex = areaEdIndex;

		JFrameUpdateInfo* before = uVec[insertIndex - 1].Get();
		newInfo->SetFrameIndex(before->GetFrameIndex() + before->GetFrameIndexSize());
		newInfo->SetNumber(before->GetNumber() + 1);

		areaInfo->IncreaseCount();
		const uint aVecCount = (uint)aVec.size();
		for (uint i = areaIndex + 1; i < aVecCount; ++i)
			aVec[i]->IncreaseStIndex();

		const uint indexSize = newInfo->GetFrameIndexSize();
		const uint uVecCount = (uint)uVec.size();
		for (int i = insertIndex; i < uVecCount; ++i)
		{
			uVec[i]->SetFrameIndex(uVec[i]->GetFrameIndex() + indexSize);
			uVec[i]->SetNumber(uVec[i]->GetNumber() + 1);
		}
		JUserPtr<JFrameUpdateInfo> result = newInfo;
		uVec.insert(uVec.begin() + insertIndex, std::move(newInfo));
		return result;
	}
	bool JDx12FrameResourceManager::Pop(JFrameUpdateInfo* info)
	{
		//const int arrayIndex = GetArrayIndex(type, holder); 
		InfoVec& uVec = updateInfoVec[(uint)info->GetType()];
		AreaInfoVec& aVec = areaInfoVec[(uint)info->GetType()];

		JFrameUpdateAreaInfo* areaInfo = info->GetAreaInfo();
		const size_t areaGuid = areaInfo->guid;

		const int uVecIndex = info->GetNumber();
		const int aVecIndex = GetAreaVecIndex(info->GetType(), areaGuid);

		const uint indexSize = info->GetFrameIndexSize();
		const uint uVecCount = (uint)uVec.size();
		for (uint i = uVecIndex + 1; i < uVecCount; ++i)
		{
			uVec[i]->SetFrameIndex(uVec[i]->GetFrameIndex() - indexSize);
			uVec[i]->SetNumber(uVec[i]->GetNumber() - 1);
		}
		uVec.erase(uVec.begin() + uVecIndex);

		areaInfo->DecreaseCount();
		const uint aVecCount = (uint)aVec.size();
		for (uint i = aVecIndex + 1; i < aVecCount; ++i)
			aVec[i]->DecreaseStIndex();

		if (areaInfo->GetInfoCount() <= 0)
			aVec.erase(aVec.begin() + aVecIndex);
		return true;
	}
	JOwnerPtr<JDx12FrameUpdateInfo> JDx12FrameResourceManager::CreateInfo(const JFrameUploadDataCreationDesc& desc, JFrameUpdateAreaInfo* areaInfo)
	{
		JOwnerPtr<JDx12FrameUpdateInfo> newInfo = Core::JPtrUtil::MakeOwnerPtr<JDx12FrameUpdateInfo>(desc.type, areaInfo, this);
		newInfo->SetFrameIndexSize(desc.indexSize);
		newInfo->SetSordOrder(desc.sortOrder);
		return std::move(newInfo);
	}
	void JDx12FrameResourceManager::ReBuild(JGraphicDevice* device, const J_FRAME_RESOURCE_UPLOAD_TYPE type, const uint newCount)
	{
		JFrameResourceManager::ReBuild(device, type, newCount);
	}
	void JDx12FrameResourceManager::BeginUpdate()
	{
		JFrameResourceManager::BeginUpdate();
		cacheData.Update();
	}
	void JDx12FrameResourceManager::Update(JFrameUpdateDataSet& set)
	{
		if (!set.metadata.isSupportedFrameResourceUpload)
			return;

		InnerUpdateFuncSet funcSet = GetUpdateFuncSet(set.metadata);
		if (funcSet.updateF == nullptr)
			return;

		if (set.option.setUpdateThreadTask != nullptr && set.metadata.isNeedToUpdateEveryFrame &&  funcSet.canUseMultiThread(set.option, set.GetDataStorageCount()))
		{ 
			WaitAllThreadTaskDone();
			auto setUpdateThreadTask = set.option.setUpdateThreadTask;
			const uint threadCount = GetGraphicInfo().frame.threadCount;

			//작업분배
			for (uint i = 0; i < threadCount; ++i)
			{
				cacheSet[i] = set;
				threadHandle[i] = setUpdateThreadTask(Core::JThreadInitInfo{}, UniqueBind(*workerFunctor, std::move(i)));
			}
			set.updateLog.updateCount += set.GetDataStorageCount();
			hasRequestThreadSync = true;
		}
		else
		{
			InnerUpdateDataSet dataSet(GetGraphicInfo(), GetGraphicOption(), this, &cacheData);
			for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
			{
				if (!set.metadata.supportedFrameType[i])
					continue;

				dataSet.minMoveDirtyIndex[i] = GetMoveDirtyMinIndex((J_FRAME_RESOURCE_UPLOAD_TYPE)i);
				dataSet.hasMoveDirted |= dataSet.minMoveDirtyIndex[i] != invalidIndex;
			}

			if (set.objDataVec != nullptr)
				DoUpdate<ObjectDataSetVec>(this, set, dataSet, funcSet);
			else
				DoUpdate<JFrameUpdateDataSet::CompVec>(this, set, dataSet, funcSet);
		}
	}
	void JDx12FrameResourceManager::EndUpdate()
	{
		JFrameResourceManager::EndUpdate();
		WaitAllThreadTaskDone();
	} 
	void JDx12FrameResourceManager::WorkerThread(uint threadIndex)
	{ 
		//Always update로 한정한다.
		InnerUpdateFuncSet funcSet = GetUpdateFuncSet(cacheSet[threadIndex].metadata);
		InnerUpdateDataSet dataSet(GetGraphicInfo(), GetGraphicOption(), this, &cacheData);
		 
		if (cacheSet[threadIndex].objDataVec != nullptr)
			DoAlwaysUpdate<ObjectDataSetVec>(cacheSet[threadIndex], dataSet, funcSet, threadIndex, dataSet.info.frame.threadCount);
		else
			DoAlwaysUpdate<JFrameUpdateDataSet::CompVec>(cacheSet[threadIndex], dataSet, funcSet, threadIndex, dataSet.info.frame.threadCount);
		cacheSet[threadIndex] = JFrameUpdateDataSet();
	}
	void JDx12FrameResourceManager::WaitAllThreadTaskDone()
	{
		if (!hasRequestThreadSync)
			return;
		  
		for (uint i = 0; i < maxNumOfUpdateThread; ++i)
			_JThreadManager::Instance().WaitUntilThreadEnd(threadHandle[i]); 
		hasRequestThreadSync = false;
	} 
	void JDx12FrameResourceManager::BuildResource(JGraphicDevice* device)
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(resource); ++i)
			resource[i].Initialize(device);
		currResourceIndex = 0;
		cacheData.Initialize();
	}
	void JDx12FrameResourceManager::ClearResource()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(resource); ++i)
			resource[i].Clear();
		currResourceIndex = 0;

		for (uint i = 0; i < SIZE_OF_ARRAY(updateInfoVec); ++i)
			updateInfoVec[i].clear();
		cacheData.Clear();
	}
	void JDx12FrameResourceManager::RegisterTypeData()
	{
		using JAllocationDesc = JinEngine::Core::JAllocationDesc;
		using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;
		using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;
		using ReceiverPtr = JAllocationDesc::ReceiverPtr;
		using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;
		using MemIndex = JAllocationDesc::MemIndex;

		NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
		{
			JDx12FrameUpdateInfo* movedInfo = static_cast<JDx12FrameUpdateInfo*>(movedPtr);
			JDx12FrameResourceManager* manager = movedInfo->manager;

			//Release를 먼저하지않으면 Reset시 유효한 pointer를 소유하므로 pointer 파괴를 시도하며
			//현재 alloc class에서 메모리를 재배치하는 과정에서 에러를 일으킬수 있으므로
			//Release() 한다음 Reset()을 호출해야한다.
			//2024-08-15 수정 포인터만 변경하는 Swap 사용 
			manager->updateInfoVec[(uint)movedInfo->GetType()][movedInfo->GetNumber()].Swap(movedInfo);
		};
		auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
		std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

		desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
		JDx12FrameUpdateInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
	}
}