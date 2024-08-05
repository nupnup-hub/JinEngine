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
using namespace DirectX;
namespace JinEngine::Graphic
{
	//Update per object
	namespace
	{
		class JConstantCache
		{
		public:
			JObjectConstantsSet objectSet;
			JCameraConstantsSet cameraSet;
			JLightConstantsSet lightSet;
		public:
			//used by pass
			//initialize first update constants buffer after initialize graphic class 
			//always exist until enigne end
			JUserPtr<JTexture> missing;
			JUserPtr<JTexture> bluseNoise;
		public:
			void Clear();
		}; 

		struct CacheSet
		{
		public:
			//used by pass
			//initialize first update constants buffer after initialize graphic class 
			//always exist until enigne end
			JUserPtr<JTexture> missing;
			JUserPtr<JTexture> bluseNoise;
		public:
			void Initialize()
			{
				missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING);
				bluseNoise = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::BLUE_NOISE);
			}
			void Clear()
			{
				missing = nullptr;
				bluseNoise = nullptr;
			}
		};

		struct UpdateFrameBufferSet
		{
		public:
			const JGraphicInfo& info;
			const JGraphicOption& option;
		public:
			JDx12FrameResourceManager* fm;
			JDx12FrameResource* frame;
		public:
			//Per object
			JObject* obj;
			JFrameUpdateInterface* updateInterface;
		public:
			UpdateFrameBufferSet(const JGraphicInfo& info, const JGraphicOption& option, JDx12FrameResourceManager* fm)
				:info(info), option(option), fm(fm)
			{
				frame = fm->GetCurrentDxFrameResource();
			}
		public:
			template<J_FRAME_RESOURCE_UPLOAD_TYPE type, typename Type>
			void CopyData(Type* constants)const
			{
				frame->CopyData(type, updateInterface->GetFrameIndex(type), constants);
			}
		};
		using UpdateFrameBufferFunc = JinEngine::Core::JSFunctorType<void, const UpdateFrameBufferSet&>;

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
		static void UpdateAnimator(const UpdateFrameBufferSet& set)
		{
			static JAnimationConstants animation;
			JAnimator* animator = static_cast<JAnimator*>(set.obj);
			animator->Compute(animation.set);
			animator->Update();

			set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::ANIMATION>(&animator);
		}
		template<int threadIndex>
		static void UpdateBehavior(const UpdateFrameBufferSet& set)
		{
			/*
			* 미구현
			*/
		}
		template<int threadIndex>
		static void UpdateCamera(const UpdateFrameBufferSet& set)
		{
			JCamera* camera = static_cast<JCamera*>(set.obj);		  
			static JDrawSceneCameraConstants drawScene;
			static JDepthTestConstants depthTest;
			static JHzbOccComputeConstants hzb;
			static JSsaoConstants ssao;
			static JLightCullingCameraConstants lightCulling;
			static JGIConstants gi;
			static JGIDenoiserPassConstants denoise;
			   
			const size_t sceneGuid = camera->GetOwner()->GetOwnerGuid();
			const XMMATRIX view = camera->GetView();
			const XMMATRIX proj = camera->GetProj();
			const XMMATRIX invView = camera->GetInvView(); 
			const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
			const XMMATRIX preViewProj = camera->GetPreViewProj();
			const JVector2F rtSize = camera->GetRenderTargetSize();
			JVector2F uvToViewA;
			JVector2F uvToViewB;
			camera->GetUvToView(uvToViewA, uvToViewB);

			const JVector3F posW = camera->GetTransform()->GetWorldPosition();
			const float camNear = camera->GetNear();
			const float camFar = camera->GetFar();
			const float viewWidth = camera->GetRenderViewWidth();
			const float viewHeight = camera->GetRenderViewHeight();

			//DrawScene
			{
				drawScene.invView.StoreXM(XMMatrixTranspose(XMMatrixInverse(nullptr, view)));
				drawScene.viewProj.StoreXM(XMMatrixTranspose(viewProj));
				drawScene.preViewProj.StoreXM(XMMatrixTranspose(preViewProj));
				drawScene.renderTargetSize = rtSize;
				drawScene.invRenderTargetSize = JVector2F::One() / rtSize;
				drawScene.uvToViewA = uvToViewA;
				drawScene.uvToViewB = uvToViewB;
				drawScene.eyePosW = posW;
				drawScene.nearZ = camNear;
				drawScene.farZ = camFar;
				drawScene.csmLocalIndex = camera->GetModuleManagedData()->GetCsmTargetUserInterface()->GetTargetIndex();
				drawScene.hasAoTexture = camera->AllowSsao() && set.option.CanUseSSAO();
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA>(&drawScene); 
			}
			//DepthTest
			if (camera->AllowHdOcclusionCulling() || camera->AllowHzbOcclusionCulling())
			{
				depthTest.viewProj = drawScene.viewProj;
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::DEPTH_TEST_PASS>(&depthTest); 
			}
			//HzbOccCompute
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
				hzb.occMapCount = set.info.resource.occlusionMapCount;
				hzb.occIndexOffset = JMathHelper::Log2Int(set.info.resource.occlusionMinSize);
				hzb.correctFailTrigger = (int)set.option.culling.allowHZBCorrectFail;
				hzb.usePerspective = true; 
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT>(&hzb);
			}
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

				ssao.camRtSize = drawScene.renderTargetSize;
				ssao.camInvRtSize = drawScene.invRenderTargetSize;

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
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::SSAO_PASS>(&hzb);
			}
			if (camera->AllowLightCulling() && set.option.culling.allowLightCluster)
			{
				lightCulling.camView.StoreXM(XMMatrixTranspose(view));
				lightCulling.camProj.StoreXM(XMMatrixTranspose(proj)); 
				lightCulling.camRenderTargetSize = drawScene.renderTargetSize;
				lightCulling.camInvRenderTargetSize = drawScene.invRenderTargetSize;
				lightCulling.camNearZ = camNear;
				lightCulling.camFarZ = camFar; 
				set.CopyData<J_FRAME_RESOURCE_UPLOAD_TYPE::LIGHT_CULLING_PASS>(&lightCulling);
			}
			
			//나중에 수정
			//Camera관련 data를 pass용 data를 분리하거나 지금처럼 subclass에서 자체적으로 upload할것.
			/*
			if (camera->AllowRaytracingGI() && set.option.CanUseRtGi())
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
			
				denoise.camInvView.StoreXM(DirectX::XMMatrixTranspose(camInvView));
				denoise.camPreInvView.StoreXM(DirectX::XMMatrixTranspose(camPreInvView.LoadXM()));
				denoise.camPreViewProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreViewProj()));
				denoise.rtSize = camRtSize;
				denoise.invRtSize = 1.0f / camRtSize;
				denoise.uvToViewA = uvToViewA;
				denoise.uvToViewB = uvToViewB;
				denoise.preUvToViewA = preUvToViewA;
				denoise.preUvToViewB = preUvToViewB;
				denoise.camNearFar = JVector2F(cam->GetNear(), cam->GetFar());
				denoise.camNearMulFar = constants.camNearFar.x * constants.camNearFar.y;
				denoise.denoiseRange = Common::denoiseRange;
				denoise.baseRadius = Common::baseRadius;
				denoise.radiusRange = Common::radiusRange;
				++denoise.sampleNumber;
				if (denoise.sampleNumber >= Common::sampleNumberMax)
					denoise.sampleNumber = 0;

				frameBuffer.CopyData(helper.info.frame.currIndex, constants);
				camPreInvView.StoreXM(camInvView);
				preUvToViewA = constants.uvToViewA;
				preUvToViewB = constants.uvToViewB;
			}
 
			*/
		}
		template<int threadIndex>
		static void UpdateDirctionalLight(const UpdateFrameBufferSet& set)
		{
			/*
					void UpdateFrame(Graphic::JDirectionalLightConstants& constant) noexcept final
		{
			const XMMATRIX viewM = view.LoadXM();
			const XMMATRIX projM = proj.LoadXM();

			constant.view.StoreXM(XMMatrixTranspose(viewM));
			constant.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM, projM)));
			if (CanAllocateCsm())
			{
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(GetShadowMapTransform()));
				//constant.shadowMapTransform.StoreXM(XMMatrixTranspose(viewM));
				constant.shadowMapIndex = GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, 0);
				constant.csmDataIndex = CsmInfoFrame::GetFrameIndex();
			}
			else if (thisPointer->IsShadowActivated())
			{
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(GetShadowMapTransform()));
				constant.shadowMapIndex = GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0);
			}

			constant.shadowMapType = (uint)GetShadowMapType();
			constant.color = thisPointer->GetColor();
			constant.power = thisPointer->GetPower();
			constant.direction = direction;
			constant.frustumSize = JVector2F(abs(vSceneBBoxMaxF.x - vSceneBBoxMinF.x), abs(vSceneBBoxMaxF.y - vSceneBBoxMinF.y));
			constant.frustumNear = vSceneBBoxMinF.z;
			constant.frustumFar = vSceneBBoxMaxF.z;
			constant.penumbraScale = thisPointer->GetPenumbraWidth();
			constant.penumbraBlockerScale = thisPointer->GetPenumbraBlockerWidth();
			constant.shadowMapSize = thisPointer->GetShadowMapSize();
			constant.shadowMapInvSize = 1.0f / constant.shadowMapSize;
			constant.tanAngle = XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(constant.direction.ToXmV(), Private::GetInitDir()));
			constant.bias = thisPointer->GetBias();
			if (constant.direction == Private::GetInitDir())
				constant.bias -= Private::initDirBias;
			DirLitFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JCsmConstants& constant, const uint index)noexcept final
		{
			const auto& result = GetCsmComputeResult(index);
			for (uint i = 0; i < result.subFrustumCount; ++i)
			{
				constant.scale[i] = result.scale[i];
				constant.posOffset[i] = result.posOffset[i];
				constant.frustumNear[i] = result.fNear[i];
				constant.frustumFar[i] = result.fFar[i];
			}
			constant.mapMinBorder = (float)(1.0f / (float)thisPointer->GetShadowMapSize());
			constant.mapMaxBorder = (float)(((float)thisPointer->GetShadowMapSize() - 1.0f) / (float)thisPointer->GetShadowMapSize());
			constant.levelBlendRate = GetCsmOptionRef().GetLevelBlendRate();
			constant.count = result.subFrustumCount;
			CsmInfoFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapArrayDrawConstants& constant, const uint index)noexcept final
		{
			const auto& result = GetCsmComputeResult(index);
			const XMMATRIX viewM = view.LoadXM();
			for (uint i = 0; i < result.subFrustumCount; ++i)
			{
				constant.shadowMapTransform[i].StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM,
					result.shadowProjM[i].LoadXM())));
			}
			CsmDrawFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapDrawConstants& constant)noexcept final
		{
			//for draw shadow map
			constant.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(view.LoadXM(), proj.LoadXM())));
			ShadowMapNormalDrawFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JDepthTestConstants& constants)noexcept final
		{
			//for draw depth map
			constants.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(view.LoadXM(), proj.LoadXM())));
			DepthTestFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JHzbOccComputeConstants& constant, const uint queryCount, const uint queryOffset)noexcept final
		{
			//only directional light  can execute occ
			auto info = JGraphic::Instance().GetGraphicInfo();
			auto option = JGraphic::Instance().GetGraphicOption();

			const XMMATRIX viewM = view.LoadXM();
			const XMMATRIX projM = proj.LoadXM();

			constant.view.StoreXM(XMMatrixTranspose(viewM));
			constant.proj.StoreXM(XMMatrixTranspose(projM)); 

			constant.viewWidth = vSceneBBoxMaxF.x - vSceneBBoxMinF.x;
			constant.viewHeight = vSceneBBoxMaxF.y - vSceneBBoxMinF.y;
			constant.camNear = vSceneBBoxMinF.z;
			constant.camFar = vSceneBBoxMaxF.z;
			constant.validQueryCount = queryCount;
			constant.validQueryOffset = queryOffset;
			constant.occMapCount = info.resource.occlusionMapCount;
			constant.occIndexOffset = JMathHelper::Log2Int(info.resource.occlusionMinSize);
			constant.correctFailTrigger = (int)option.culling.allowHZBCorrectFail;
			constant.usePerspective = false;	//use ortho
			HzbOccComputeFrame::MinusMovedDirty();
		}
			*/
		}
		template<int threadIndex>
		static void UpdatePointLight(const UpdateFrameBufferSet& set)
		{

		}
		template<int threadIndex>
		static void UpdateSpotLight(const UpdateFrameBufferSet& set)
		{

		}
		template<int threadIndex>
		static void UpdateRectLight(const UpdateFrameBufferSet& set)
		{

		}
		template<int threadIndex>
		static void UpdateRenderItem(const UpdateFrameBufferSet& set)
		{

		}
		template<int threadIndex>
		static void UpdateMaterial(const UpdateFrameBufferSet& set)
		{

		}
		template<int threadIndex>
		static void UpdateScene(const UpdateFrameBufferSet& set)
		{
			static JScenePassConstants scene;
		}

		static UpdateFrameBufferFunc::Ptr GetStuffConstantsFunc(const JObjectDataSetMetadata& metadata)
		{
			if (metadata.IsComponentType())
			{

			}
			else if (metadata.IsResourceType())
			{

			}
			else
				return;
		}
	} 

	void JDx12FrameResourceManager::Initialize(JGraphicDevice* device)
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(resource); ++i)
			resource[i].Initialize(device);
		currResourceIndex = 0;
	}
	void JDx12FrameResourceManager::Clear()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(resource); ++i)
			resource[i].Clear();
		currResourceIndex = 0;

		for (uint i = 0; i < SIZE_OF_ARRAY(updateInfoVec); ++i)
			updateInfoVec[i].clear();
		cache.Clear();
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
	int JDx12FrameResourceManager::GetAreaEdIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type, const size_t areaGuid)const noexcept
	{
		const JFrameUpdateAreaInfo* areaInfo = GetAreaInfo(type, areaGuid);
		return areaInfo != nullptr ? areaInfo->GetEdIndex() : invalidIndex;
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
		++currResourceIndex;
		if (currResourceIndex >= Constants::gNumFrameResources)
			currResourceIndex = 0;
	}
	JUserPtr<JFrameUpdateInfo> JDx12FrameResourceManager::Register(const JFrameUploadDataCreationDesc& desc)
	{
		const int areaIndex = GetAreaVecIndex(desc.type, desc.areaGuid);
		if (areaIndex == invalidIndex)
			PushBack(desc);
		else
			Insert(desc, areaIndex);
	}
	void JDx12FrameResourceManager::DeRegister(const JUserPtr<JFrameUpdateInfo>& info)
	{
		if (info == nullptr || !info->HasValidFrameIndex())
			return;

		JFrameResourceManager::DeRegister(info);
		Pop(info);
	}
	bool JDx12FrameResourceManager::PushBack(const JFrameUploadDataCreationDesc& desc)
	{
		InfoVec& vec = updateInfoVec[(uint)desc.type];
		AreaInfoVec& areaVec = areaInfoVec[(uint)desc.type];
		const int count = vec.size();

		JFrameUpdateAreaInfo* beforeArea = areaVec[areaVec.size() - 1].get();
		std::unique_ptr<JFrameUpdateAreaInfo> newAreaInfo = std::make_unique<JFrameUpdateAreaInfo>(desc.areaGuid, beforeArea->edIndex + 1, beforeArea->edIndex + 2);
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

		areaVec.push_back(std::move(newAreaInfo));
		vec.push_back(std::move(newInfo));
		return true;
	}
	bool JDx12FrameResourceManager::Insert(const JFrameUploadDataCreationDesc& desc, const int areaIndex)
	{
		InfoVec& uVec = updateInfoVec[(uint)desc.type];
		AreaInfoVec& aVec = areaInfoVec[(uint)desc.type];
		JFrameUpdateAreaInfo* areaInfo = aVec[areaIndex].get();
		JOwnerPtr<JDx12FrameUpdateInfo> newInfo = CreateInfo(desc, areaInfo);

		int areaStIndex = areaInfo->GetStIndex();
		int areaEdIndex = areaInfo->GetEdIndex();

		int8 sortOrder = newInfo->GetSortOrder();
		int insertIndex = -1;
		if (sortOrder != UCHAR_MAX)
		{
			for (int i = areaStIndex; i <= areaEdIndex; ++i)
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
			insertIndex = areaEdIndex + 1;

		JFrameUpdateInfo* before = uVec[insertIndex - 1].Get();
		newInfo->SetFrameIndex(before->GetFrameIndex() + before->GetFrameIndexSize());
		newInfo->SetNumber(before->GetNumber() + 1);
		areaInfo->IncreaseEdIndex();

		const uint aVecCount = (uint)aVec.size();
		for (uint i = areaIndex + 1; i < aVecCount; ++i)
			aVec[i]->Increase();

		const uint indexSize = newInfo->GetFrameIndexSize();
		const uint uVecCount = (uint)uVec.size();
		for (int i = insertIndex + 1; i < uVecCount; ++i)
		{
			uVec[i]->SetFrameIndex(uVec[i]->GetFrameIndex() + indexSize);
			uVec[i]->SetNumber(uVec[i]->GetNumber() + 1);
		}
		uVec.insert(uVec.begin() + insertIndex, std::move(newInfo));
		return true;
	}
	bool JDx12FrameResourceManager::Pop(const JUserPtr<JFrameUpdateInfo>& info)
	{
		//const int arrayIndex = GetArrayIndex(type, holder); 
		InfoVec& uVec = updateInfoVec[(uint)info->GetType()];
		AreaInfoVec& aVec = areaInfoVec[(uint)info->GetType()];

		JFrameUpdateAreaInfo* areaInfo = info->GetAreaInfo();
		const size_t areaGuid = areaInfo->guid;

		const int uVecIndex = info->GetNumber();
		const int aVecIndex = GetAreaVecIndex(info->GetType(), areaGuid);

		uVec.erase(uVec.begin() + uVecIndex);
		areaInfo->DecreaseEdIndex();

		const uint aVecCount = (uint)aVec.size();
		for (uint i = aVecIndex + 1; i < aVecCount; ++i)
			aVec[i]->Decrease();

		if (areaInfo->GetInfoCount() <= 0)
			aVec.erase(aVec.begin() + aVecIndex);

		const uint indexSize = info->GetFrameIndexSize();
		const uint uVecCount = (int)uVec.size();
		for (uint i = uVecIndex + 1; i < uVecCount; ++i)
		{
			uVec[i]->SetFrameIndex(uVec[i]->GetFrameIndex() - indexSize);
			uVec[i]->SetNumber(uVec[i]->GetNumber() - 1);
		}
		uVec.erase(uVec.begin() + uVecIndex);
		return true;
	}
	JOwnerPtr<JDx12FrameUpdateInfo> JDx12FrameResourceManager::CreateInfo(const JFrameUploadDataCreationDesc& desc, JFrameUpdateAreaInfo* areaInfo)const noexcept
	{
		JOwnerPtr<JDx12FrameUpdateInfo> newInfo = Core::JPtrUtil::MakeOwnerPtr<JDx12FrameUpdateInfo>(desc.type, areaInfo, this);
		newInfo->SetFrameIndexSize(desc.indexSize);
		newInfo->SetSordOrder(desc.sortOrder);
	}
	void JDx12FrameResourceManager::BeginUpdate()
	{
		JFrameResourceManager::BeginUpdate(); 
	}
	void JDx12FrameResourceManager::Update(JFrameUpdateDataSet& set)
	{
		if (!set.metadata.isSupportedFrameResourceUpload)
			return;

		UpdateFrameBufferFunc::Ptr updateFrameBufferFunc = GetStuffConstantsFunc(set.metadata);
		if (updateFrameBufferFunc == nullptr)
			return;
		 
		UpdateFrameBufferSet updateFrameBufferSet(GetGraphicInfo(), GetGraphicOption(), this); 

		const uint count = set.objDataVec->Count();
		for (uint i = 0; i < count; ++i)
		{
			JOwnerPtr<JGraphicObjectDataSetBase>& objSet = *set.objDataVec->Get(i);
			auto frameInterface = objSet->GetFrameUpdateInterface();

			if (set.option.forcedUpdateTrigger)
				frameInterface->SetFrameDirty();

			if (frameInterface->IsDirted())
			{
				if (frameInterface->GetDirtyBase()->IsFrameHotDirted())
					frameInterface->TryExecuteObjectUpdateBind();

				updateFrameBufferSet.obj = objSet->Object().Get();
				updateFrameBufferSet.updateInterface = frameInterface;
				updateFrameBufferFunc(updateFrameBufferSet);

				frameInterface->GetDirtyBase()->MinusFrameDirty();
				if (frameInterface->GetDirtyBase()->IsLastFrameHotUpdated())
					++set.updateLog.hotUpdatedCount;
				++set.updateLog.updatedCount;
			}
		}
	}
	void JDx12FrameResourceManager::EndUpdate()
	{
		JFrameResourceManager::EndUpdate();
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
			manager->updateInfoVec[(uint)movedInfo->GetType()][movedInfo->GetNumber()].Release();
			manager->updateInfoVec[(uint)movedInfo->GetType()][movedInfo->GetNumber()].Reset(movedInfo);
		};
		auto reAllocF = std::make_unique<JAllocationDesc::NotifyReAllocF::Functor>(notifyPtr);
		std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();

		desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
		JDx12FrameUpdateInfo::StaticTypeInfo().SetAllocationOption(std::move(desc));
	}
}