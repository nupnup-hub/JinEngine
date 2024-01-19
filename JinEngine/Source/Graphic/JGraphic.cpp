#pragma once
#include"JGraphic.h" 
#include"JGraphicPrivate.h" 
#include"JGraphicConstants.h"
#include"JGraphicDrawList.h" 
#include"JGraphicUpdateHelper.h"
#include"GraphicResource/JGraphicResourceManager.h"
#include"GraphicResource/JGraphicResourceInfo.h"
#include"GraphicResource/JGraphicResourceInterface.h"
#include"Device/JGraphicDevice.h"
#include"Scene/JSceneDraw.h"
#include"ShadowMap/JShadowMap.h"
#include"ShadowMap/JCsmManager.h"
#include"Debug/JGraphicDebug.h"
#include"DepthMap/JDepthTest.h" 
#include"Buffer/JGraphicBuffer.h"  
#include"Image/JImageProcessing.h"
#include"Culling/JCullingInfo.h"
#include"Culling/JCullingManager.h"
#include"Culling/Frustum/JFrustumCulling.h"
#include"Culling/Occlusion/JHardwareOccCulling.h"
#include"Culling/Occlusion/JHZBOccCulling.h"
#include"Culling/Light/JLightCulling.h"
#include"Outline/JOutline.h"

#include"FrameResource/JFrameUpdate.h" 
#include"FrameResource/JFrameResource.h" 
#include"FrameResource/JObjectConstants.h" 
#include"FrameResource/JAnimationConstants.h" 
#include"FrameResource/JMaterialConstants.h" 
#include"FrameResource/JPassConstants.h" 
#include"FrameResource/JCameraConstants.h" 
#include"FrameResource/JLightConstants.h"  
#include"FrameResource/JBoundingObjectConstants.h"
#include"FrameResource/JOcclusionConstants.h"
#include"FrameResource/JShadowMapConstants.h"
#include"FrameResource/JDepthTestConstants.h"

#include"Gui/JGuiBackendInterface.h"
#include"Gui/JGuiBackendDataAdapter.h"
#include"Adapter/JGraphicAdapter.h"
#include"Gui/JGuiBackendDataAdaptee.h"

#include"../Core/Time/JGameTimer.h"
#include"../Core/Time/JStopWatch.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/File/JFileConstant.h"
#include"../Core/Exception/JExceptionMacro.h"	
#include"../Core/Platform/JHardwareInfo.h"
#include"../Core/Threading/JThreadManager.h"
#include"../Core/Threading/JThreadManagerPrivate.h"
#include"../Core/Memory/JMemoryCapture.h"	
#include"../Core/Unit/JByteUnit.h"	
#include"../Core/Utility/JCommonUtility.h" 
#include"../Core/Log/JLogHandler.h" 

#include"../Object/GameObject/JGameObject.h"
#include"../Object/Component/RenderItem/JRenderItem.h"
#include"../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../Object/Component/Transform/JTransform.h"
#include"../Object/Component/Camera/JCamera.h"
#include"../Object/Component/Camera/JCameraPrivate.h"
#include"../Object/Component/Animator/JAnimator.h"
#include"../Object/Component/Animator/JAnimatorPrivate.h"
#include"../Object/Component/Light/JLight.h"
#include"../Object/Component/Light/JLightPrivate.h"
#include"../Object/Component/Light/JDirectionalLight.h"
#include"../Object/Component/Light/JDirectionalLightPrivate.h"
#include"../Object/Component/Light/JPointLight.h"
#include"../Object/Component/Light/JPointLightPrivate.h"
#include"../Object/Component/Light/JSpotLight.h"
#include"../Object/Component/Light/JSpotLightPrivate.h"
#include"../Object/Component/Light/JRectLight.h"
#include"../Object/Component/Light/JRectLightPrivate.h"

#include"../Object/Resource/JResourceObjectDefualtData.h"
#include"../Object/Resource/JResourceManager.h"
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Resource/Scene/JScenePrivate.h"
#include"../Object/Resource/Mesh/JMeshGeometry.h"
#include"../Object/Resource/Mesh/JMeshGeometryPrivate.h"
#include"../Object/Resource/Material/JMaterial.h"
#include"../Object/Resource/Material/JMaterialPrivate.h"
#include"../Object/Resource/Shader/JShader.h"
#include"../Object/Resource/Shader/JShaderPrivate.h" 
#include"../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../Object/Resource/Scene/JSceneManager.h"
#include"../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"
#include"../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../Object/Resource/Texture/JTexture.h" 

#include"../Window/JWindow.h"
#include"../Window/JWindowPrivate.h"

#include"../Application/JApplicationEngine.h"
#include"../Application/JApplicationEnginePrivate.h"
#include"../Application/JApplicationProject.h"  

#ifdef DEVELOP
#include"../Develop/Debug/JDevelopDebug.h"
#endif
namespace JinEngine
{
	using namespace DirectX;
	namespace Graphic
	{
		class JConstantCash
		{
		public:
			JEnginePassConstants enginePass;
		public:
			JScenePassConstants scenePass;
			JAnimationConstants ani;
		public:
			JCameraConstants camContants;
		public:
			JDirectionalLightConstants dLight;
			JPointLightConstants pLight;
			JSpotLightConstants sLight;
			JRectLightConstants rLight;
			JCsmConstants csmInfo;
			JShadowMapArrayDrawConstants csmDraw;		//cascade shadow map
			JShadowMapCubeDrawConstants cubeShadowMap;	//cube shadow map
			JShadowMapDrawConstants normalShadowMap;	//normal shadow map
			JHzbOccComputeConstants hzbOccCmpute;
			JSsaoConstants ssao;
		public:
			JDepthTestPassConstants depthTest;
		public:
			//used by pass
			//initialize first update constants buffer after initialize graphic class 
			//always exist until enigne end
			JUserPtr<JTexture> missing;
			JUserPtr<JTexture> bluseNoise;
			JUserPtr<JTexture> ltcMat;
			JUserPtr<JTexture> ltcAmp;
		public:
			void Clear()
			{
				scenePass = JScenePassConstants();
				ani = JAnimationConstants();
			}
		};
		namespace
		{
			using CamEditorSettingInterface = JCameraPrivate::EditorSettingInterface;
			using GraphicThreadInteface = Core::JThreadManagerPrivate::GraphicInterface; 
			using WindowEventListener = Core::JEventListener<size_t, Window::J_WINDOW_EVENT>;
		}
		namespace Private
		{
			/*
			* newFirst, newSecond중 하나만 true값을 가지며
			* 입력값이 비정확할경우 newFirst을 true로 한다.
			*/
			static void SwitchBoolValue(bool& newFirst, bool& newSecond, bool& oldFirst, bool& oldSecond, const bool selectFirstIfAllFail = true)
			{
				if (newFirst && newSecond)
				{
					if (!oldFirst)
						newSecond = false;
					else if (!oldSecond)
						newFirst = false;

					if (newFirst && newSecond)
						newSecond = false;
				}
				else if (!newFirst && !newSecond && selectFirstIfAllFail)
				{
					if (oldFirst)
						newFirst = oldFirst;
					if (oldSecond)
						newSecond = oldSecond;
					if (!newFirst && !newSecond)
						newFirst = true;
				}
			}
			/*
			* newFirst, newSecond, newThird 중 하나만 true값을 가지며
			* 입력값이 비정확할경우 newFirst을 true로 한다.
			*/
			static void SwitchBoolValue(bool& newFirst, bool& newSecond, bool& newThird, bool& oldFirst, bool& oldSecond, bool& oldThird, const bool selectFirstIfAllFail = true)
			{
				int trueCount = newFirst + newSecond + newThird;
				if (trueCount > 1)
				{
					if (!oldFirst && newFirst)
						newSecond = newThird = false;
					else if (!oldSecond && newSecond)
						newFirst = newThird = false;
					else if (!oldThird && newThird)
						newFirst = newSecond = false;

					trueCount = newFirst + newSecond + newThird;
					if (trueCount > 1)
						newSecond = newThird = false;
				}
				else if (trueCount == 0 && selectFirstIfAllFail)
				{
					if (oldFirst)
						newFirst = oldFirst;
					if (oldSecond)
						newSecond = oldSecond;
					if (oldThird)
						newThird = oldThird;

					if (!newFirst && !newSecond && !newThird)
						newFirst = true;
				}
			}
			static void SwitchBoolValue(bool& newFirst, bool& newSecond, bool& newThird, bool& newForth, bool& oldFirst, bool& oldSecond, bool& oldThird, bool& oldForth, const bool selectFirstIfAllFail = true)
			{
				int trueCount = newFirst + newSecond + newThird + newForth;
				if (trueCount > 1)
				{
					if (!oldFirst && newFirst)
						newSecond = newThird = newForth = false;
					else if (!oldSecond && newSecond)
						newFirst = newThird = newForth = false;
					else if (!oldThird && newThird)
						newFirst = newSecond = newForth = false;
					else if (!oldForth && newForth)
						newFirst = newSecond = newThird = false;

					trueCount = newFirst + newSecond + newThird + newForth;
					if (trueCount > 1)
						newSecond = newThird = newForth = false;
				}
				else if (trueCount == 0 && selectFirstIfAllFail)
				{
					if (oldFirst)
						newFirst = oldFirst;
					if (oldSecond)
						newSecond = oldSecond;
					if (oldThird)
						newThird = oldThird;
					if (oldForth)
						newForth = oldForth;

					if (!newFirst && !newSecond && !newThird && !newForth)
						newFirst = true;
				}
			}
			static void SwitchBoolValue(bool& newFirst, bool& newSecond, bool& newThird, bool& newForth, bool& newFifth, bool& oldFirst, bool& oldSecond, bool& oldThird, bool& oldForth, bool& oldFifth, const bool selectFirstIfAllFail = true)
			{
				int trueCount = newFirst + newSecond + newThird + newForth;
				if (trueCount > 1)
				{
					if (!oldFirst && newFirst)
						newSecond = newThird = newForth = newFifth = false;
					else if (!oldSecond && newSecond)
						newFirst = newThird = newForth = newFifth = false;
					else if (!oldThird && newThird)
						newFirst = newSecond = newForth = newFifth = false;
					else if (!oldForth && newForth)
						newFirst = newSecond = newThird = newFifth = false;
					else if (!oldFifth && newFifth)
						newFirst = newSecond = newThird = newForth = false;

					trueCount = newFirst + newSecond + newThird + newForth + newFifth;
					if (trueCount > 1)
						newSecond = newThird = newForth = newFifth = false;
				}
				else if (trueCount == 0 && selectFirstIfAllFail)
				{
					if (oldFirst)
						newFirst = oldFirst;
					if (oldSecond)
						newSecond = oldSecond;
					if (oldThird)
						newThird = oldThird;
					if (oldForth)
						newForth = oldForth;
					if (oldFifth)
						newFifth = oldFifth;

					if (!newFirst && !newSecond && !newThird && !newForth && !newFifth)
						newFirst = true;
				}
			}
		}

#pragma region Impl
		class JGraphic::JGraphicImpl : public WindowEventListener, public GraphicEventManager
		{
		private:
			using WorkerThreadF = Core::JMFunctorType<JGraphicImpl, void, uint>;
		public:
			JGraphic* thisGraphic;
		public:
			const size_t guid;
		public:
			std::unique_ptr<JFrameResource> frameResources[Constants::gNumFrameResources];
			JFrameResource* currFrameResource = nullptr;
		public:
			JGraphicInfo info;
			JGraphicOption option;
			JUpdateHelper updateHelper;
			//전체 opaque object 만큼 할당된 object vec
			//hard ware occlusion이나 object align이 필요할때 결과를 담을 vector로써 사용된다.
			JGameObjectBuffer alignedObject;
			JConstantCash contCash;
		private:
			std::unique_ptr<JGraphicAdapter> adapter;
			std::unique_ptr<JGraphicDevice> device;
			std::unique_ptr<JGraphicResourceManager> graphicResourceM;
			std::unique_ptr<JCullingManager> cullingM;
			std::unique_ptr<JCsmManager> csmM;	//safe change device
			std::unique_ptr<JSceneDraw> sceneDraw;
			std::unique_ptr<JShadowMap> shadowMap;
			std::unique_ptr<JFrustumCulling> frustumHelper; //safe change device
			std::unique_ptr<JHardwareOccCulling> hdOccHelper;
			std::unique_ptr<JHZBOccCulling> hzbOccHelper;
			std::unique_ptr<JLightCulling> litCullHelper;
			std::unique_ptr<JGraphicDebug> debug;
			std::unique_ptr<JDepthTest> depthTest;
			std::unique_ptr<JOutline> outlineHelper;
			std::unique_ptr<JImageProcessing> imageP;
			std::unique_ptr<JGraphicDrawReferenceSet> drawRefSet; 
		private:
			std::unique_ptr<WorkerThreadF::Functor> workerFunctor;
		private:
			JGuiBackendInterface* guiBackendInterface;
			std::unique_ptr<JGuiBackendDataAdapter> guiAdapter;
		public:
			JGraphicImpl(const size_t guid, JGraphic* thisGraphic)
				:GraphicEventManager([](const size_t& a, const size_t& b) {return a == b; }),
				guid(guid), thisGraphic(thisGraphic)
			{
				IntializeGraphicInfo();
				InitializeGameObjectBuffer();
				RegisterResouceNotifyFunc();
				workerFunctor = std::make_unique<WorkerThreadF::Functor>(&JGraphicImpl::WorkerThread, this);
			}
			~JGraphicImpl()
			{ }
		private:
			//CallOnece
			void IntializeGraphicInfo()
			{
				const uint occMipmapViewCapa = JGraphicResourceManager::GetOcclusionMipmapViewCapacity();
				info.occlusionWidth = std::pow(2, occMipmapViewCapa - 1);
				info.occlusionHeight = std::pow(2, occMipmapViewCapa - 1);
				info.occlusionMinSize = JGraphicResourceManager::GetOcclusionMinSize();
				info.occlusionMapCapacity = occMipmapViewCapa;
				info.occlusionMapCount = JMathHelper::Log2Int(info.occlusionWidth) - JMathHelper::Log2Int(JGraphicResourceManager::GetOcclusionMinSize()) + 1;
				info.frameThreadCount = _JThreadManager::Instance().GetReservedSpaceCount(Core::J_THREAD_USE_CASE_TYPE::GRAPHIC_DRAW);
			}
			//CallOnece
			void InitializeGameObjectBuffer()
			{
				alignedObject.common.resize(info.minCapacity);
			}
			//CallOnece
			void RegisterResouceNotifyFunc()
			{
				auto objGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT); };
				auto enginePassGetElemenLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS); };;
				auto scenePassGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS); };
				auto aniGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION); };
				auto camGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA); };
				auto materialGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL); };
				auto boundObjGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT); };
				auto dLitGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT); };
				auto pLitGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT); };
				auto sLitGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT); };
				auto rLitGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT); };
				auto csmInfoGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO); };
				auto shadowMapDrawingGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW); };
				auto shadowMapArrayDrawingGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW); };
				auto shadowMapCubeDrawingGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW); };
				auto hzbRequestorGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS); };
				auto hzbObjectGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT); };
				auto depthTestGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS); };
				auto ssaoGetElementLam = []() {return JFrameUpdateData::GetTotalFrameCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS); };

				using GetElementCount = JUpdateHelper::GetElementCountT::Ptr;
				std::unordered_map<J_UPLOAD_FRAME_RESOURCE_TYPE, GetElementCount> uGetCountFunc
				{
					{J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, objGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS, enginePassGetElemenLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS, scenePassGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, aniGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, camGetElementLam},{J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, materialGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, boundObjGetElementLam},	{J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT, dLitGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, pLitGetElementLam},	{J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sLitGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, rLitGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO, csmInfoGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, shadowMapDrawingGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW, shadowMapArrayDrawingGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW, shadowMapCubeDrawingGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, hzbRequestorGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, hzbObjectGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, depthTestGetElementLam},
					 {J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, ssaoGetElementLam}
				};

				using NotifyUpdateCapacity = JUpdateHelper::NotifyUpdateCapacityT::Callable;
				using ExtraUpdateCapacity = JUpdateHelper::ExtraUpdateListenerT::Callable;
				auto updateFrustumAndHdCullingResultCapaLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					const uint newCapa = impl->currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT);
					impl->cullingM->ReBuildBuffer(J_CULLING_TYPE::FRUSTUM, impl->device.get(), newCapa, J_CULLING_TARGET::RENDERITEM);
					impl->cullingM->ReBuildBuffer(J_CULLING_TYPE::HD_OCCLUSION, impl->device.get(), newCapa);
					impl->hdOccHelper->NotifyReBuildHdOccBuffer(impl->device.get(), newCapa, impl->cullingM->GetCullingInfoVec(J_CULLING_TYPE::HD_OCCLUSION));
					impl->alignedObject.common.resize(newCapa);
				};
				auto updateHzbOccResultCapaLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					const uint newCapa = impl->currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT);

					impl->cullingM->ReBuildBuffer(J_CULLING_TYPE::HZB_OCCLUSION, impl->device.get(), newCapa);
					impl->hzbOccHelper->NotifyReBuildHzbOccBuffer(impl->device.get(), newCapa, impl->cullingM->GetCullingInfoVec(J_CULLING_TYPE::HZB_OCCLUSION));
				};
				auto updateLightCullingResultCapaLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					auto& pointLitUData = impl->updateHelper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT];
					auto& spotLitUData = impl->updateHelper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT];
					auto& rectLitUData = impl->updateHelper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT];
					if (pointLitUData.reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP ||
						spotLitUData.reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP ||
						rectLitUData.reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
					{
						impl->ReBuildLightCullingResource(true, false);
					}
				};

				for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				{
					J_UPLOAD_FRAME_RESOURCE_TYPE type = (J_UPLOAD_FRAME_RESOURCE_TYPE)i;
					auto data = uGetCountFunc.find(type);
					if (data == uGetCountFunc.end())
						continue;

					updateHelper.RegisterCallable(type, uGetCountFunc.find(type)->second);
				}
				updateHelper.RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, std::make_unique<NotifyUpdateCapacity>(updateFrustumAndHdCullingResultCapaLam));
				updateHelper.RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, std::make_unique<NotifyUpdateCapacity>(updateHzbOccResultCapaLam));
				updateHelper.RegisterExListener(std::make_unique<ExtraUpdateCapacity>(updateLightCullingResultCapaLam));
				//updateHelper.RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, std::make_unique<NotifyUpdateCapacity>(updateHdOccResultCapaLam));

				auto texture2DGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D); };
				auto cubeMapGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE); };
				auto shadowMapGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP); };
				auto shadowMapArrayGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY); };
				auto shadowMapCubeGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE); };

				auto texture2DGetCapacityLam = []() {return _JGraphic::Instance().impl->info.binding2DTextureCapacity; };
				auto cubeMapGetCapacityLam = []() {return _JGraphic::Instance().impl->info.bindingCubeMapCapacity; };
				auto shadowMapGetCapacityLam = []() {return _JGraphic::Instance().impl->info.bindingShadowTextureCapacity; };
				auto shadowMapArrayGetCapacityLam = []() {return _JGraphic::Instance().impl->info.bindingShadowTextureArrayCapacity; };
				auto shadowMapCubeGetCapacityLam = []() {return _JGraphic::Instance().impl->info.bindingShadowTextureCubeCapacity; };

				using BindTextureGetCount = JUpdateHelper::GetElementCountT::Ptr;
				using BindTextureGetCapacity = JUpdateHelper::GetElementCapacityT::Ptr;

				//recompile shader for change texture array capacity	 
				std::unordered_map <J_GRAPHIC_RESOURCE_TYPE, bool> hasCallable
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, true},
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, true},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, true},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, true},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE, true}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureGetCount> bindTextureGetCountFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DGetCountLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapGetCountLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapGetCountLam},{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, shadowMapArrayGetCountLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE, shadowMapCubeGetCountLam}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureGetCapacity> bindTextureGetCapacityFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DGetCapacityLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapGetCapacityLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapGetCapacityLam}, {J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, shadowMapArrayGetCapacityLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE, shadowMapCubeGetCapacityLam}
				};

				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					J_GRAPHIC_RESOURCE_TYPE type = (J_GRAPHIC_RESOURCE_TYPE)i;
					auto data = hasCallable.find(type);
					if (data != hasCallable.end())
					{
						updateHelper.RegisterCallable(type,
							&bindTextureGetCountFunc.find(type)->second,
							&bindTextureGetCapacityFunc.find(type)->second);
					}
				}
			}
		public:
			JGraphicInfo GetGraphicInfo()const noexcept
			{
				return info;
			}
			JGraphicOption GetGraphicOption()const noexcept
			{
				return option;
			}
			void GetLastDeviceErrorInfo(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)const noexcept
			{
				device->GetLastDeviceError(errorCode, errorMsg);
			}
			JGraphicResourceManager* GetGraphicResourceManager()const noexcept
			{
				return graphicResourceM.get();
			}
			std::unique_ptr<JGuiInitData> GetGuiInitData()const noexcept
			{
				return guiAdapter->CreateInitData(device.get(),
					graphicResourceM.get(),
					option.deviceType,
					guiBackendInterface->GetGuiIdentification());
			}
		public:
			void SetOption(JGraphicOption newGraphicOption)noexcept
			{ 
				bool needRecompileGraphicShader = false;
				bool needRecompileLightClusterShader = false;
				bool needRecompileSsaoShader = false;
				bool needRebuildlightCullingResource = false;
				bool isChanged = false;

				newGraphicOption.clusterXIndex = std::clamp(newGraphicOption.clusterXIndex, (uint)0, Constants::litClusterXVariation - 1);
				newGraphicOption.clusterYIndex = std::clamp(newGraphicOption.clusterYIndex, (uint)0, Constants::litClusterYVariation - 1);
				newGraphicOption.clusterZIndex = std::clamp(newGraphicOption.clusterZIndex, (uint)0, Constants::litClusterZVariation - 1);
				newGraphicOption.lightPerClusterIndex = std::clamp(newGraphicOption.lightPerClusterIndex, (uint)0, Constants::lightPerClusterVariation - 1);
				newGraphicOption.clusterNear = std::clamp(newGraphicOption.clusterNear, Constants::litClusterNearMin, Constants::litClusterNearMax);
				newGraphicOption.clusterPointLightRangeOffset = std::clamp(newGraphicOption.clusterPointLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);
				newGraphicOption.clusterSpotLightRangeOffset = std::clamp(newGraphicOption.clusterSpotLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);
				newGraphicOption.clusterRectLightRangeOffset = std::clamp(newGraphicOption.clusterRectLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);

				//rendering
				needRecompileGraphicShader |= (option.allowLightCluster != newGraphicOption.allowLightCluster);
				needRecompileGraphicShader |= (option.allowDeferred != newGraphicOption.allowDeferred);
				needRecompileGraphicShader |= (option.useMSAA != newGraphicOption.useMSAA);

				//shadow option
				needRecompileGraphicShader |= (option.useHighQualityShadow != newGraphicOption.useHighQualityShadow);
				needRecompileGraphicShader |= (option.useMiddleQualityShadow != newGraphicOption.useMiddleQualityShadow);
				needRecompileGraphicShader |= (option.useLowQualityShadow != newGraphicOption.useLowQualityShadow);

				//ao
				needRecompileGraphicShader |= (option.useSsao != newGraphicOption.useSsao); 

				//brdf
				needRecompileGraphicShader |= (option.useSmithMasking != newGraphicOption.useSmithMasking);
				needRecompileGraphicShader |= (option.useTorranceMaskig != newGraphicOption.useTorranceMaskig);
				needRecompileGraphicShader |= (option.useGGXNDF != newGraphicOption.useGGXNDF);
				needRecompileGraphicShader |= (option.useBeckmannNDF != newGraphicOption.useBeckmannNDF);
				needRecompileGraphicShader |= (option.useBlinnPhongNDF != newGraphicOption.useBlinnPhongNDF);
				needRecompileGraphicShader |= (option.useIsotropy != newGraphicOption.useIsotropy);
				needRecompileGraphicShader |= (option.useDisneyDiffuse != newGraphicOption.useDisneyDiffuse);
				needRecompileGraphicShader |= (option.useHammonDiffuse != newGraphicOption.useHammonDiffuse);
				needRecompileGraphicShader |= (option.useOrenNayarDiffuse != newGraphicOption.useOrenNayarDiffuse);
				needRecompileGraphicShader |= (option.useShirleyDiffuse != newGraphicOption.useShirleyDiffuse);
				needRecompileGraphicShader |= (option.useLambertianDiffuse != newGraphicOption.useLambertianDiffuse);

				//cluster
				bool isClusterXIndexChanged = false;
				bool isClusterYIndexChanged = false;
				isClusterXIndexChanged = (option.clusterXIndex != newGraphicOption.clusterXIndex);
				isClusterYIndexChanged = (option.clusterYIndex != newGraphicOption.clusterYIndex);
				needRebuildlightCullingResource |= (isClusterXIndexChanged || isClusterYIndexChanged);
				needRebuildlightCullingResource |= (option.clusterZIndex != newGraphicOption.clusterZIndex);
				needRebuildlightCullingResource |= (option.lightPerClusterIndex != newGraphicOption.lightPerClusterIndex);
				needRebuildlightCullingResource |= (option.clusterNear != newGraphicOption.clusterNear);
				needRecompileLightClusterShader |= (option.clusterPointLightRangeOffset != newGraphicOption.clusterPointLightRangeOffset);
				needRecompileLightClusterShader |= (option.clusterSpotLightRangeOffset != newGraphicOption.clusterSpotLightRangeOffset);
				needRecompileLightClusterShader |= (option.clusterRectLightRangeOffset != newGraphicOption.clusterRectLightRangeOffset);

				//x, y는 항상 2:1 resolution을 유지하기위해 각자 다른 배열에 같은 index위치에
				//2:1 비율이 되는 값을 할당한다.
				if (isClusterXIndexChanged)
					newGraphicOption.clusterYIndex = newGraphicOption.clusterXIndex;
				if (isClusterYIndexChanged)
					newGraphicOption.clusterXIndex = newGraphicOption.clusterYIndex;

#ifdef DEVELOP
				//debugging`
				needRecompileGraphicShader |= newGraphicOption.requestRecompileGraphicShader;
				needRecompileLightClusterShader |= newGraphicOption.requestRecompileLightClusterShader;
				needRecompileSsaoShader |= newGraphicOption.requestRecompileSsaoShader;
				needRecompileSsaoShader |= (option.useSsaoInterleave != newGraphicOption.useSsaoInterleave); ;
				 
				//needRecompileSsaoShader |= (option.drawSsaoByComputeShader != newGraphicOption.drawSsaoByComputeShader);
#endif 
				Private::SwitchBoolValue(newGraphicOption.useHighQualityShadow, newGraphicOption.useMiddleQualityShadow, newGraphicOption.useLowQualityShadow,
					option.useHighQualityShadow, option.useMiddleQualityShadow, option.useLowQualityShadow);
				 
				Private::SwitchBoolValue(newGraphicOption.useSmithMasking, newGraphicOption.useTorranceMaskig,
					option.useSmithMasking, option.useTorranceMaskig);
				Private::SwitchBoolValue(newGraphicOption.useGGXNDF, newGraphicOption.useBeckmannNDF, newGraphicOption.useBlinnPhongNDF,
					option.useGGXNDF, option.useBeckmannNDF, option.useBlinnPhongNDF);
				Private::SwitchBoolValue(newGraphicOption.useDisneyDiffuse, 
					newGraphicOption.useHammonDiffuse, 
					newGraphicOption.useOrenNayarDiffuse,
					newGraphicOption.useShirleyDiffuse, 
					newGraphicOption.useLambertianDiffuse,
					option.useDisneyDiffuse, 
					option.useHammonDiffuse,
					option.useOrenNayarDiffuse,
					option.useShirleyDiffuse,
					option.useLambertianDiffuse);

				JGraphicOption preOption = option;
				option = newGraphicOption;
				 
				const bool needCommand = needRecompileGraphicShader || 
					needRebuildlightCullingResource || 
					needRecompileLightClusterShader || 
					needRecompileSsaoShader;
				if (device != nullptr && needCommand)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();
					if (needRecompileGraphicShader || needRebuildlightCullingResource)
						sceneDraw->RecompileShader(JGraphicShaderCompileSet(device.get(), info, option));
					if (needRecompileLightClusterShader || needRebuildlightCullingResource)
						litCullHelper->RecompileShader(JGraphicShaderCompileSet(device.get(), info, option));
					if(needRecompileSsaoShader)
						imageP->RecompileShader(JGraphicShaderCompileSet(device.get(), info, option));
					if (needRebuildlightCullingResource)
						ReBuildLightCullingResource(false, true);
					NotifyEvent<J_GRAPHIC_EVENT_TYPE::OPTION_CHANGED>(guid, preOption, option);
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}
				if (cullingM != nullptr)
				{
					if (preOption.isOcclusionQueryActivated != option.isOcclusionQueryActivated)
					{
						cullingM->StuffClearValue(device.get(), J_CULLING_TYPE::HZB_OCCLUSION);
						cullingM->StuffClearValue(device.get(), J_CULLING_TYPE::HD_OCCLUSION);
					} 
				}
#ifdef DEVELOP
				option.requestRecompileGraphicShader =  option.requestRecompileLightClusterShader = option.requestRecompileSsaoShader = false;
#endif
			}
		private:
			bool IsEntryUpdateLoop()
			{
				return JApplicationEngine::GetApplicationSubState() == J_APPLICATION_SUB_STATE::UPDATE_LOOP;
			}
		public:
			JUserPtr<JGraphicResourceInfo> CreateResource(const JGraphicResourceCreationDesc& createDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				auto userPtr = graphicResourceM->CreateResource(device.get(), createDesc, rType);
				if (userPtr == nullptr)
					return userPtr;

				if (rType == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D || rType == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
				{
					if (!createDesc.textureDesc->UseMipmap())
						userPtr->SetMipmapType(J_GRAPHIC_MIP_MAP_TYPE::NONE);
					else if (userPtr != nullptr && createDesc.textureDesc->mipMapDesc.type != J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT)
					{
						if (!CreateMipmap(userPtr, *createDesc.textureDesc))
							userPtr->SetMipmapType(J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT);
					}
				} 
				else if (rType == J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP)
				{
					//아래 함수와같이 resource creation에 대해 observe를 요구하는 상황이 추가적으로 발생하면
					//event혹은 interface class를 만들어서 묶어서 관리하도록 한다.
					imageP->TryCreationSsaoIntermediate(device.get(), graphicResourceM.get(), userPtr.Get());
				}
				return userPtr;
			} 
			bool CreateOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
			{
				return graphicResourceM->CreateOption(device.get(), info, opType);
			}
			bool CreateMipmap(const JUserPtr<JGraphicResourceInfo>& srcInfo, JTextureCreationDesc& createDesc)
			{
				if (srcInfo == nullptr || createDesc.mipMapDesc.type == J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT || createDesc.mipMapDesc.type == J_GRAPHIC_MIP_MAP_TYPE::NONE)
					return false;

				JUserPtr<JGraphicResourceInfo> modInfo = nullptr;
				auto rType = srcInfo->GetGraphicResourceType();
				if (rType != J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D && rType != J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
					return false;

				//추가로 custom mipmap이 필요한 J_GRAPHIC_RESOURCE_TYPE이 있을경우 수정필요.
				createDesc.allowUav = true;
				JGraphicResourceCreationDesc cDesc(std::make_unique<JTextureCreationDesc>(createDesc));
				modInfo = graphicResourceM->CreateResource(device.get(), cDesc, rType);
				createDesc.allowUav = false;

				if (modInfo == nullptr)
					return false;

				const JMipmapGenerationDesc& mipmapDesc = createDesc.mipMapDesc;
				JVector2F imageSize = JVector2F(srcInfo->GetWidth(), srcInfo->GetHeight());
				uint mipCount = srcInfo->GetMipmapCount();

				std::unique_ptr<JDownSampleDesc> desc;
				switch (mipmapDesc.type)
				{
				case JinEngine::Graphic::J_GRAPHIC_MIP_MAP_TYPE::BOX:
				{
					desc = std::make_unique<JBoxDownSampleDesc>(imageSize, mipCount);
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_MIP_MAP_TYPE::GAUSSIAN:
				{
					desc = std::make_unique<JGaussianDownSampleDesc>(imageSize, mipmapDesc.kernelSize, mipCount, mipmapDesc.sharpnessFactor);
					break;
				}
				case JinEngine::Graphic::J_GRAPHIC_MIP_MAP_TYPE::KAISER:
				{
					desc = std::make_unique<JKaiserDownSampleDesc>(imageSize, mipmapDesc.kernelSize, mipCount, mipmapDesc.sharpnessFactor);
					break;
				}
				default:
					break;
				}

				device->FlushCommandQueue();
				device->StartPublicCommand();
				std::unique_ptr<JGraphicDownSampleComputeSet> computeSet;
				adapter->SettingMipmapGenerationTask(option.deviceType, *drawRefSet, srcInfo, modInfo, std::move(desc), computeSet);
				if (computeSet != nullptr)
				{
					JDrawHelper helper(info, option, alignedObject);
					imageP->ApplyMipmapGeneration(computeSet.get(), helper);
					device->EndPublicCommand();
					device->FlushCommandQueue();

					device->FlushCommandQueue();
					device->StartPublicCommand();
					graphicResourceM->CopyResource(device.get(), modInfo, srcInfo);
					srcInfo->SetMipmapType(mipmapDesc.type);
				}
				device->EndPublicCommand();
				device->FlushCommandQueue();

				graphicResourceM->DestroyGraphicTextureResource(device.get(), modInfo.Release());
				ClearMipmapBind(computeSet->handle);
				return true;
			}
		public:
			bool DestroyGraphicTextureResource(JGraphicResourceInfo* gInfo)
			{
				if (gInfo == nullptr)
					return false;
 
				if (gInfo->GetGraphicResourceType() == J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP)
					imageP->TryDestructionSsaoIntermediate(device.get(), graphicResourceM.get(), gInfo);
				return graphicResourceM->DestroyGraphicTextureResource(device.get(), gInfo);
			}
			bool DestroyGraphicOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)
			{
				if (info == nullptr)
					return false;

				return graphicResourceM->DestroyGraphicOption(device.get(), info, optype);
			} 
		public:	 
			bool MipmapBindForDebug(const JUserPtr<JGraphicResourceInfo>& gRInfo, _Out_ std::vector<ResourceHandle>& gpuHandle, _Out_ std::vector<Core::JDataHandle>& dataHandle)
			{
				gpuHandle.clear();
				dataHandle.clear();
				if (gRInfo == nullptr)
					return false;

				const uint mipmapCount = gRInfo->GetMipmapCount();
				if (mipmapCount < 2)
					return false;

				if (!graphicResourceM->SettingMipmapBind(device.get(), gRInfo, true, dataHandle))
					return false;

				const uint count = dataHandle.size();
				gpuHandle.resize(count);
				for (uint i = 0; i < count; ++i)
					gpuHandle[i] = graphicResourceM->GetMPBResourceGpuHandle(dataHandle[i], J_GRAPHIC_BIND_TYPE::SRV);
				return true;
			}
			void ClearMipmapBind(_In_ std::vector<Core::JDataHandle>& dataHandle)
			{
				//mipmap은 순차적으로 할당되며 해제는 마지막에 할당된 handle부터 수행한다.(DataStructure valind index가 가장 앞에 index를 가리키도록...)
				const int count = (int)dataHandle.size();
				for (int i = count - 1; i >= 0; --i)
					graphicResourceM->DestroyMPB(device.get(), dataHandle[i]);
			}
		public:
			JUserPtr<JCullingInfo> CreateFrsutumCullingResultBuffer(const J_CULLING_TARGET target, const bool useGpu)
			{
				JCullingCreationDesc desc;
				if (target == J_CULLING_TARGET::RENDERITEM)
					desc.capacity = currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT);
				else
					desc.capacity = currFrameResource->GetLocalLightCount();
				desc.target = target;
				//if graphic update전 초기화 단계일경우 update wait에 진입하자마자 info.currFrameResourceIndex + 1이 되므로
				//미리  info.currFrameResourceIndex + 1값을 할당한다.
				desc.currFrameIndex = IsEntryUpdateLoop() ? info.currFrameResourceIndex : info.currFrameResourceIndex + 1;
				desc.useGpu = useGpu;
				auto res = cullingM->CreateFrsutumData(device.get(), desc);
				if (res != nullptr)
					alignedObject.aligned.push_back(JGameObjectBuffer::OpaqueVec());
				return res;
			}
			JUserPtr<JCullingInfo> CreateHzbCullingResultBuffer()
			{
				JCullingCreationDesc desc;
				desc.capacity = currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT);
				desc.target = J_CULLING_TARGET::RENDERITEM;
				desc.currFrameIndex = IsEntryUpdateLoop() ? info.currFrameResourceIndex : info.currFrameResourceIndex + 1;
				desc.useGpu = true;

				auto user = cullingM->CreateHzbOcclusionData(device.get(), desc);
				if (user.IsValid())
				{
					if (device->CanStartPublicCommand())
					{
						device->FlushCommandQueue();
						device->StartPublicCommand();
						hzbOccHelper->NotifyBuildNewHzbOccBuffer(device.get(), currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT), user);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						hzbOccHelper->NotifyBuildNewHzbOccBuffer(device.get(), currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT), user);
				}
				return user;
			}
			JUserPtr<JCullingInfo> CreateHdCullingResultBuffer()
			{
				JCullingCreationDesc desc;
				desc.capacity = currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT);
				desc.target = J_CULLING_TARGET::RENDERITEM;
				desc.currFrameIndex = IsEntryUpdateLoop() ? info.currFrameResourceIndex : info.currFrameResourceIndex + 1;
				desc.useGpu = true;

				auto user = cullingM->CreateHdOcclusionData(device.get(), desc);
				if (user.IsValid())
				{
					if (device->CanStartPublicCommand())
					{
						device->FlushCommandQueue();
						device->StartPublicCommand();
						hdOccHelper->NotifyBuildNewHdOccBuffer(device.get(), currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT), user);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						hdOccHelper->NotifyBuildNewHdOccBuffer(device.get(), currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT), user);
				}
				return user;
			}
			bool DestroyCullignData(JCullingInfo* cullingInfo)
			{
				if (cullingInfo == nullptr)
					return false;

				const J_CULLING_TYPE cType = cullingInfo->GetCullingType();
				const J_CULLING_TARGET cTarget = cullingInfo->GetCullingTarget();
				bool res = false;
				if (cType == J_CULLING_TYPE::FRUSTUM)
					alignedObject.aligned.pop_back();
				else if (cType == J_CULLING_TYPE::HZB_OCCLUSION)
				{
					if (device->CanStartPublicCommand())
					{
						device->FlushCommandQueue();
						device->StartPublicCommand();
						hzbOccHelper->NotifyDestroyHzbOccBuffer(cullingInfo);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						hzbOccHelper->NotifyDestroyHzbOccBuffer(cullingInfo);
				}
				else if (cType == J_CULLING_TYPE::HD_OCCLUSION)
				{
					if (device->CanStartPublicCommand())
					{
						device->FlushCommandQueue();
						device->StartPublicCommand();
						hdOccHelper->NotifyDestroyHdOccBuffer(cullingInfo);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						hdOccHelper->NotifyDestroyHdOccBuffer(cullingInfo);
				}
				return cullingM->DestroyCullingData(cullingInfo, device.get());
			}
		public:
			bool RegisterHandler(JCsmHandlerInterface* handler)
			{
				return csmM->RegisterHandler(handler);
			}
			bool DeRegisterHandler(JCsmHandlerInterface* handler)
			{
				return csmM->DeRegisterHandler(handler);
			}
			bool RegisterTarget(JCsmTargetInterface* target)
			{
				return csmM->RegisterTarget(target);
			}
			bool DeRegisterTarget(JCsmTargetInterface* target)
			{
				return csmM->DeRegisterTarget(target);
			}
		public:
			JOwnerPtr<JGraphicShaderDataHolderBase> StuffGraphicShaderPso(const JGraphicShaderInitData& shaderData)
			{
				device->FlushCommandQueue();
				device->StartPublicCommand();
				auto result = sceneDraw->CreateGraphicShader(JGraphicShaderCompileSet(device.get(), info, option), shaderData);
				device->EndPublicCommand();
				device->FlushCommandQueue();
				return std::move(result);
			}
			JOwnerPtr<JComputeShaderDataHolderBase> StuffComputeShaderPso(const JComputeShaderInitData& shaderData)
			{
				JOwnerPtr<JComputeShaderDataHolderBase> result = nullptr;
				device->FlushCommandQueue();
				device->StartPublicCommand();
				//앞으로 hzb에 대한 shader option control은 JShader객체를 통해서가아닌 
				//Graphic Option을 통해서 이루어지게 한다.
				//그러므로 JHZBOccCulling 하위 class들이 Shader data를 소유하고 graphic option변경에 따라 graphic이 이들을 호출해
				//shader를 새 option에 따라 재컴파일한다.
				/*
				* 	if (shaderData.cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_COPY ||
					shaderData.cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING ||
					shaderData.cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION)
					result = hzbOccHelper->CreateComputeShader(device.get(), graphicResourceM.get(), shaderData);
				*/
				device->EndPublicCommand();
				device->FlushCommandQueue();
				return std::move(result);
			}
		public:
			void ReBuildFrameResource(const J_UPLOAD_FRAME_RESOURCE_TYPE type)
			{
				auto& uData = updateHelper.uData[(uint)type];
				for (int i = 0; i < Constants::gNumFrameResources; ++i)
					frameResources[i]->ReBuild(device.get(), type, CalculateCapacity(uData));
			}
			void ReBuildLightCullingResource(const bool isCapacityChanged, const bool isClusterOptioChanged)
			{
				if (!isCapacityChanged && !isClusterOptioChanged)
					return;

				const uint pointCapacity = currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT);
				const uint spotCapacity = currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT);
				const uint rectCapacity = currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT);

				if (isCapacityChanged)
				{
					const uint sum = pointCapacity + spotCapacity + rectCapacity;
					cullingM->ReBuildBuffer(J_CULLING_TYPE::FRUSTUM, device.get(), sum, J_CULLING_TARGET::LIGHT);
					litCullHelper->NotifyLocalLightCapacityChanged(device.get(), graphicResourceM.get(), sum);
				}

				JGraphicResourceCreationDesc lightRtDesc;
				lightRtDesc.width = option.GetClusterXCount();
				lightRtDesc.height = option.GetClusterYCount();
				lightRtDesc.arraySize = max(max(pointCapacity, spotCapacity), rectCapacity);

				graphicResourceM->ReAllocTypePerAllResource(device.get(), lightRtDesc, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING);
				if (isClusterOptioChanged)
				{
					JGraphicResourceCreationDesc linkedDesc;
					linkedDesc.width = option.GetClusterIndexCount();

					JGraphicResourceCreationDesc offsetDesc;
					offsetDesc.width = option.GetClusterTotalCount();

					graphicResourceM->ReAllocTypePerAllResource(device.get(), linkedDesc, J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST);
					graphicResourceM->ReAllocTypePerAllResource(device.get(), offsetDesc, J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET);
					litCullHelper->NotifyNewClusterOption(device.get(), JGraphicBaseDataSet(info, option));
				}
			}
		public:
			uint CalNextFrameResourceIndex(const uint currIndex)
			{
				return (currIndex + 1) % Constants::gNumFrameResources;
			}
			uint CalculateCapacity(const JUpdateHelper::UpdateDataBase& uBase)const noexcept
			{
				uint nextCapacity = uBase.capacity;
				if (uBase.reAllocCondition == J_UPLOAD_CAPACITY_CONDITION::UP_CAPACITY)
				{
					while (nextCapacity <= uBase.count)
						nextCapacity *= uBase.upCapacityFactor;
				}
				else if (uBase.reAllocCondition == J_UPLOAD_CAPACITY_CONDITION::DOWN_CAPACITY)
				{
					while ((nextCapacity / uBase.downCapacityFactor) > uBase.count && (nextCapacity / uBase.downCapacityFactor) > info.minCapacity)
						nextCapacity /= uBase.downCapacityFactor;
				}
				return nextCapacity;
			}
			void OnResize()
			{
				const JVector2F clientSize = JWindow::GetClientSize();
				info.width = clientSize.x;
				info.height = clientSize.y;

				// Flush before changing any resources.
				device->FlushCommandQueue();
				device->StartPublicCommand();

				JGraphicBaseDataSet baseSet(info, option);
				device->ResizeWindow({ info, option, graphicResourceM.get() });
				graphicResourceM->ResizeWindow(baseSet, device.get());
				outlineHelper->UpdatePassBuf(info.width, info.height, Constants::commonStencilRef);

				// Wait until resize is complete.
				device->EndPublicCommand();
				device->FlushCommandQueue();
			}
			void FlushCommandQueue()
			{
				device->FlushCommandQueue();
			}
			void AllocateRefSet()
			{
				drawRefSet = std::make_unique<JGraphicDrawReferenceSet>(info, option,
					device.get(),
					graphicResourceM.get(),
					cullingM.get(),
					currFrameResource,
					debug.get(),
					depthTest.get(),
					imageP.get(),
					info.currFrameResourceIndex,
					CalNextFrameResourceIndex(info.currFrameResourceIndex));
			}
		public:
			void UpdateWait()
			{
				info.currFrameResourceIndex = CalNextFrameResourceIndex(info.currFrameResourceIndex);
				currFrameResource = frameResources[info.currFrameResourceIndex].get();

				device->UpdateWait(currFrameResource->GetFenceValue());

				AllocateRefSet();
				adapter->BeginUpdateStart(option.deviceType, *drawRefSet);
			}
			void UpdateFrame()
			{
				updateHelper.Clear();
				for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				{
					if (!updateHelper.uData[i].useGetMultiCount)
						updateHelper.uData[i].count = (*updateHelper.uData[i].getElement)(nullptr);
					updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
					UpdateReAllocCondition(updateHelper.uData[i]);
					updateHelper.hasRebuildCondition |= (bool)updateHelper.uData[i].reAllocCondition;
				}
				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					if (updateHelper.bData[i].HasCallable())
					{
						updateHelper.bData[i].count = (*updateHelper.bData[i].getTextureCount)(nullptr);
						updateHelper.bData[i].capacity = (*updateHelper.bData[i].getTextureCapacity)(nullptr);
						UpdateReAllocCondition(updateHelper.bData[i]);

						updateHelper.hasRecompileShader |= (bool)updateHelper.bData[i].reAllocCondition;
						if (updateHelper.bData[i].reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
							updateHelper.bData[i].capacity = CalculateCapacity(updateHelper.bData[i]);
					}
				}

				updateHelper.WriteGraphicInfo(info);
				if (updateHelper.hasRebuildCondition)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();
					for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
					{
						if (updateHelper.uData[i].reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
						{
							ReBuildFrameResource((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
							updateHelper.uData[i].setDirty = Constants::gNumFrameResources;
							updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
						}
					}
					//Has sequency dependency
					updateHelper.WriteGraphicInfo(info);
					if (updateHelper.hasRecompileShader)
						sceneDraw->RecompileShader(JGraphicShaderCompileSet(device.get(), info, option));		//use graphic info
					updateHelper.NotifyUpdateFrameCapacity(*thisGraphic);	//use graphic info			
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}
				else if (updateHelper.hasRecompileShader)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();
					sceneDraw->RecompileShader(JGraphicShaderCompileSet(device.get(), info, option));
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}

				const uint drawListCount = JGraphicDrawList::GetListCount();
				//update frame resource and decide something drawing
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					const bool isAllowOcclusion = drawTarget->scene->IsMainScene() && option.isOcclusionQueryActivated;
					const uint sceneDrawReqCount = (uint)drawTarget->sceneRequestor.size();
					updateHelper.BeginUpdatingDrawTarget();
					drawTarget->BeginUpdate();
					UpdateSceneObjectCB(drawTarget->scene, drawTarget);
					UpdateSceneAnimationCB(drawTarget->scene, drawTarget);
					UpdateSceneCameraCB(drawTarget->scene, drawTarget);
					UpdateSceneLightCB(drawTarget->scene, drawTarget);
					UpdateSceneRequestor(drawTarget);
					UpdateShadowRequestor(drawTarget);
					UpdateFrustumCullingRequestor(drawTarget);
					UpdateOccCullingRequestor(drawTarget);
					UpdateScenePassCB(drawTarget->scene);	//always update && has order dependency(Light)
					drawTarget->EndUpdate();
					updateHelper.EndUpdatingDrawTarget();
				}
				UpdateEnginePassCB(0);	//always update
				UpdateMaterialCB();

#ifdef GRAPIC_DEBUG
				//Debug
				//if(hzbOccHelper->CanReadBackDebugInfo())
				//	hzbOccHelper->StreamOutDebugInfo(JApplicationProject::LogPath() + L"\\Hzb.txt");
				//JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(0); 
				//cullingM->TryStreamOutCullingBuffer(cullingM->GetCullingInfo(J_CULLING_TYPE::HD_OCCLUSION, 
				//	drawTarget->scene->FindFirstSelectedCamera(false)->CullingUserInterface().GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)).Get(),"LightCullignResult");
				//litCullHelper->StreamOutDebugInfo(JApplicationProject::LogPath(), JGraphicBaseDataSet(info, option));
#endif
			}
			void UpdateReAllocCondition(JUpdateHelper::UpdateDataBase& uBase)const noexcept
			{
				uBase.reAllocCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
				if (uBase.capacity <= uBase.count)
				{
					uBase.reAllocCondition = J_UPLOAD_CAPACITY_CONDITION::UP_CAPACITY;
					uBase.downCapacityCount = 0;
				}
				else if (uBase.count < (uBase.capacity / uBase.downCapacityFactor) && uBase.count > info.minCapacity)
				{
					if (uBase.downCapacityCount >= uBase.downCapacityCountMax)
					{
						uBase.reAllocCondition = J_UPLOAD_CAPACITY_CONDITION::DOWN_CAPACITY;
						uBase.downCapacityCount = 0;
					}
					else
					{
						uBase.reAllocCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
						++uBase.downCapacityCount;
					}
				}
				else
					uBase.downCapacityCount = 0;
			}
		private:
			void UpdateSceneObjectCB(_In_ const JUserPtr<JScene>& scene, _Inout_ JGraphicDrawTarget* target)
			{
				const bool isUpdateBoundingObj = scene->IsMainScene() && option.isOcclusionQueryActivated;
				const std::vector<JUserPtr<JComponent>>& jRvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint renderItemCount = (uint)jRvec.size();

				JObjectConstants objectConstants;
				JBoundingObjectConstants boundingConstants;
				JHzbOccObjectConstants occObjectConstants;

				auto currObjectCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT);
				auto currBoundingObjectCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT);
				auto currOccObjectBuffer = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT);

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].setDirty;

				using FrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
				auto updateInfo = target->updateInfo.get();
				for (uint i = 0; i < renderItemCount; ++i)
				{
					JRenderItem* renderItem = static_cast<JRenderItem*>(jRvec[i].Get());
					if (FrameUpdateInterface::UpdateStart(renderItem, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(renderItem, boundingConstants);
						FrameUpdateInterface::UpdateFrame(renderItem, occObjectConstants);
						currBoundingObjectCB->CopyData(FrameUpdateInterface::GetBoundingFrameIndex(renderItem), &boundingConstants);
						currOccObjectBuffer->CopyData(FrameUpdateInterface::GetOccObjectFrameIndex(renderItem), &occObjectConstants);

						const uint objectFrameIndexOffset = FrameUpdateInterface::GetObjectFrameIndex(renderItem);
						const uint submeshCount = renderItem->GetSubmeshCount();
						for (uint j = 0; j < submeshCount; ++j)
						{
							FrameUpdateInterface::UpdateFrame(renderItem, objectConstants, j);
							currObjectCB->CopyData(objectFrameIndexOffset + j, &objectConstants);
						}
						FrameUpdateInterface::UpdateEnd(renderItem);
						if (FrameUpdateInterface::IsLastFrameHotUpdated(renderItem))
							++updateInfo->hotObjUpdateCount;
						++updateInfo->objUpdateCount;
					}
					else
					{
						if (FrameUpdateInterface::HasObjectRecopyRequest(renderItem))
						{
							const uint objectFrameIndexOffset = FrameUpdateInterface::GetObjectFrameIndex(renderItem);
							const uint submeshCount = renderItem->GetSubmeshCount();
							for (uint j = 0; j < submeshCount; ++j)
							{
								FrameUpdateInterface::UpdateFrame(renderItem, objectConstants, j);
								currObjectCB->CopyData(objectFrameIndexOffset + j, &objectConstants);
							}
							updateInfo->hasObjRecopy = true;
						}
						if (FrameUpdateInterface::HasBoundingRecopyRequest(renderItem))
						{
							FrameUpdateInterface::UpdateFrame(renderItem, boundingConstants);
							currBoundingObjectCB->CopyData(FrameUpdateInterface::GetBoundingFrameIndex(renderItem), &boundingConstants);
							updateInfo->hasObjRecopy = true;
						}
						if (FrameUpdateInterface::HasOccObjectRecopyRequest(renderItem))
						{
							FrameUpdateInterface::UpdateFrame(renderItem, occObjectConstants);
							currOccObjectBuffer->CopyData(FrameUpdateInterface::GetOccObjectFrameIndex(renderItem), &occObjectConstants);
							updateInfo->hasObjRecopy = true;
						}
					}
				}

				target->updateInfo->thisFrameObjCount = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].uploadCountPerTarget = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].uploadCountPerTarget = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].uploadCountPerTarget = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].uploadCountPerTarget = scene->GetMeshCount();
			}
			void UpdateMaterialCB()
			{
				auto currMaterialBuffer = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL);
				auto matVec = JMaterial::StaticTypeInfo().GetInstanceRawPtrVec();
				const uint matCount = matVec.size();
				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].setDirty;

				using FrameUpdateInterface = JMaterialPrivate::FrameUpdateInterface;
				for (uint i = 0; i < matCount; ++i)
				{
					JMaterialConstants materialConstant;
					JMaterial* material = static_cast<JMaterial*>(matVec[i]);

					if (FrameUpdateInterface::UpdateStart(material, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(material, materialConstant);
						currMaterialBuffer->CopyData(FrameUpdateInterface::GetMaterialFrameIndex(material), &materialConstant);
						FrameUpdateInterface::UpdateEnd(material);
					}
					else if (FrameUpdateInterface::HasRecopyRequest(material))
					{
						FrameUpdateInterface::UpdateFrame(material, materialConstant);
						currMaterialBuffer->CopyData(FrameUpdateInterface::GetMaterialFrameIndex(material), &materialConstant);
					}
				};
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].uploadCountPerTarget += matCount;
			}
			void UpdateEnginePassCB(const uint frameIndex)
			{
				contCash.enginePass.appTotalTime = JEngineTimer::Data().TotalTime();
				contCash.enginePass.appDeltaTime = JEngineTimer::Data().DeltaTime();
				if (contCash.missing == nullptr)
					contCash.missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING);
				if (contCash.bluseNoise == nullptr)
					contCash.bluseNoise = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::BLUE_NOISE);
				if (contCash.ltcMat == nullptr)
					contCash.ltcMat = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_MAT);
				if (contCash.ltcAmp == nullptr)
					contCash.ltcAmp = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::LTC_AMP);

				auto missingInterface = contCash.missing->GraphicResourceUserInterface();
				auto blueNoiseInterface = contCash.bluseNoise->GraphicResourceUserInterface();
				auto ltcMatInterface = contCash.ltcMat->GraphicResourceUserInterface();
				auto ltcAmpInterface = contCash.ltcAmp->GraphicResourceUserInterface();

				contCash.enginePass.missingTextureIndex = missingInterface.GetFirstResourceArrayIndex();
				contCash.enginePass.bluseNoiseTextureIndex = blueNoiseInterface.GetFirstResourceArrayIndex();
				contCash.enginePass.bluseNoiseTextureSize = blueNoiseInterface.GetFirstResourceSize();
				contCash.enginePass.invBluseNoiseTextureSize = blueNoiseInterface.GetFirstResourceInvSize();
				contCash.enginePass.ltcMatTextureIndex = ltcMatInterface.GetFirstResourceArrayIndex();
				contCash.enginePass.ltcAmpTextureIndex = ltcAmpInterface.GetFirstResourceArrayIndex();
				contCash.enginePass.clusterMinDepth = std::log2(option.clusterNear);

				auto currPassCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS);
				currPassCB->CopyData(frameIndex, &contCash.enginePass);
				++updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS].uploadCountPerTarget;
			}
			void UpdateScenePassCB(_In_ const JUserPtr<JScene>& scene)
			{
				using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;
				if (scene->IsActivatedSceneTime())
				{
					contCash.scenePass.sceneTotalTime = scene->GetTotalTime();
					contCash.scenePass.sceneDeltaTime = scene->GetDeltaTime();
				}
				else
				{
					contCash.scenePass.sceneTotalTime = 0;
					contCash.scenePass.sceneDeltaTime = 0;
				}

				auto currPassCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS);
				currPassCB->CopyData(SceneFrameIndexInterface::GetPassFrameIndex(scene.Get()), &contCash.scenePass);
			}
			void UpdateSceneAnimationCB(_In_ const JUserPtr<JScene>& scene, _Inout_ JGraphicDrawTarget* target)
			{
				const std::vector<JUserPtr<JComponent>>& jAvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
				const uint animatorCount = (uint)jAvec.size();

				auto currSkinnedCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION);

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].setDirty;
				using FrameUpdateInterface = JAnimatorPrivate::FrameUpdateInterface;

				auto updateInfo = target->updateInfo.get();
				if (scene->IsActivatedSceneTime())
				{
					for (uint i = 0; i < animatorCount; ++i)
					{
						JAnimator* animator = static_cast<JAnimator*>(jAvec[i].Get());
						if (FrameUpdateInterface::UpdateStart(animator))
						{
							FrameUpdateInterface::UpdateFrame(animator, contCash.ani);
							currSkinnedCB->CopyData(FrameUpdateInterface::GetFrameIndex(animator), &contCash.ani);
							FrameUpdateInterface::UpdateEnd(animator);
							++updateInfo->aniUpdateCount;
						}
						else if (FrameUpdateInterface::HasRecopyRequest(animator))
						{
							FrameUpdateInterface::UpdateFrame(animator, contCash.ani);
							currSkinnedCB->CopyData(FrameUpdateInterface::GetFrameIndex(animator), &contCash.ani);
						}
					}
				}
			}
			void UpdateSceneCameraCB(_In_ const JUserPtr<JScene>& scene, _Inout_ JGraphicDrawTarget* target)
			{
				//is same as sceneRequestor 
				const std::vector<JUserPtr<JComponent>>& jCvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
				const uint cameraCount = (uint)jCvec.size();
				const uint hzbOccQueryCount = scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint hzbOccQueryOffset = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].uploadOffset;

				auto currCameraCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA);
				auto currDepthCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS);
				auto currHzbOccReqCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS);
				auto currSsaoCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS);

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS].setDirty;
				using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;

				auto updateInfo = target->updateInfo.get();
				for (uint i = 0; i < cameraCount; ++i)
				{
					JCamera* camera = static_cast<JCamera*>(jCvec[i].Get());
					if (FrameUpdateInterface::UpdateStart(camera, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(camera, contCash.camContants);
						currCameraCB->CopyData(FrameUpdateInterface::GetCamFrameIndex(camera), &contCash.camContants);

						if (camera->AllowHdOcclusionCulling())
						{
							FrameUpdateInterface::UpdateFrame(camera, contCash.depthTest);
							currDepthCB->CopyData(FrameUpdateInterface::GetDepthTestPassFrameIndex(camera), &contCash.depthTest);
							++updateInfo->hdOccUpdateCount;
						}
						if (camera->AllowHzbOcclusionCulling())
						{
							FrameUpdateInterface::UpdateFrame(camera, contCash.depthTest);
							FrameUpdateInterface::UpdateFrame(camera, contCash.hzbOccCmpute, hzbOccQueryCount, hzbOccQueryOffset);
							currDepthCB->CopyData(FrameUpdateInterface::GetDepthTestPassFrameIndex(camera), &contCash.depthTest);
							currHzbOccReqCB->CopyData(FrameUpdateInterface::GetHzbOccComputeFrameIndex(camera), &contCash.hzbOccCmpute);
							++updateInfo->hzbOccUpdateCount;
						}
						if (camera->AllowSsao())
						{
							FrameUpdateInterface::UpdateFrame(camera, contCash.ssao);
							currSsaoCB->CopyData(FrameUpdateInterface::GetSsaoFrameIndex(camera), &contCash.ssao);
						}
						FrameUpdateInterface::UpdateEnd(camera);
						if (FrameUpdateInterface::IsLastFrameHotUpdated(camera))
							++updateInfo->hotCamUpdateCount;
						++updateInfo->camUpdateCount;
					}
					else
					{
						if (FrameUpdateInterface::HasCamRecopyRequest(camera))
						{
							FrameUpdateInterface::UpdateFrame(camera, contCash.camContants);
							currCameraCB->CopyData(FrameUpdateInterface::GetCamFrameIndex(camera), &contCash.camContants);
						}
						if (FrameUpdateInterface::HasDepthTestPassRecopyRequest(camera))
						{
							FrameUpdateInterface::UpdateFrame(camera, contCash.depthTest);
							currDepthCB->CopyData(FrameUpdateInterface::GetDepthTestPassFrameIndex(camera), &contCash.depthTest);
						}
						if (FrameUpdateInterface::HasHzbOccComputeRecopyRequest(camera))
						{
							FrameUpdateInterface::UpdateFrame(camera, contCash.hzbOccCmpute, hzbOccQueryCount, hzbOccQueryOffset);
							currHzbOccReqCB->CopyData(FrameUpdateInterface::GetHzbOccComputeFrameIndex(camera), &contCash.hzbOccCmpute);
						}
						if (FrameUpdateInterface::HasSsaoRecopyRequest(camera))
						{
							FrameUpdateInterface::UpdateFrame(camera, contCash.ssao);
							currSsaoCB->CopyData(FrameUpdateInterface::GetSsaoFrameIndex(camera), &contCash.ssao);
						}
					} 
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].uploadCountPerTarget = cameraCount;
			}
			void UpdateSceneLightCB(_In_ const JUserPtr<JScene>& scene, _Inout_ JGraphicDrawTarget* target)
			{
				const std::vector<JUserPtr<JComponent>>& jLvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
				const uint lightVecCount = (uint)jLvec.size();
				const uint hzbOccQueryCount = scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint hzbOccQueryOffset = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].uploadOffset;

				uint litCount[(uint)J_LIGHT_TYPE::COUNT] = { 0, 0, 0, 0 };
				int minFrameIndex[(uint)J_LIGHT_TYPE::COUNT] = { INT_MAX, INT_MAX, INT_MAX, INT_MAX };

				bool hasLitUpdate = false;
				const bool hasObjectHotUpdate = target->updateInfo->hotObjUpdateCount;
				const bool hasCamHotUpdate = target->updateInfo->hotCamUpdateCount;

				const bool forcedSetFrameDirty[(uint)J_LIGHT_TYPE::COUNT]
				{
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].setDirty |
					hasObjectHotUpdate,

					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW].setDirty |
					hasObjectHotUpdate,

					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].setDirty |
					hasObjectHotUpdate,

					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].setDirty |
					hasObjectHotUpdate,
				};

				using LitFrameUpdateInterface = JLightPrivate::FrameUpdateInterface;
				using DLitFrameUpdateInterface = JDirectionalLightPrivate::FrameUpdateInterface;
				using PLitFrameUpdateInterface = JPointLightPrivate::FrameUpdateInterface;
				using SLitFrameUpdateInterface = JSpotLightPrivate::FrameUpdateInterface;
				using ALitFrameUpdateInterface = JRectLightPrivate::FrameUpdateInterface;
				using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;

				struct LitUpdateHelper
				{
				public:
					JFrameResource& currFrame;
					JConstantCash& contCash;
				public:
					LitUpdateHelper(JFrameResource* currFrame, JConstantCash& contCash)
						:currFrame(*currFrame), contCash(contCash)
					{}
				public:
					void UpdateLitFrame(JLight* light,
						JLightPrivate* lp,
						JGraphicDrawTarget::UpdateInfo* info,
						const bool allowShadowMapUpdate,
						const bool isRecopy)
					{
						auto& frameUpdateInterface = lp->GetFrameUpdateInterface();
						const int litFrameIndex = frameUpdateInterface.GetLitFrameIndex(light);
						const J_LIGHT_TYPE litType = light->GetLightType();

						if (litType == J_LIGHT_TYPE::DIRECTIONAL)
						{
							JDirectionalLight* dLight = static_cast<JDirectionalLight*>(light);
							static_cast<DLitFrameUpdateInterface&>(frameUpdateInterface).UpdateFrame(dLight, contCash.dLight);
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT, litFrameIndex, &contCash.dLight);
						}
						else if (litType == J_LIGHT_TYPE::POINT)
						{
							JPointLight* pLight = static_cast<JPointLight*>(light);
							static_cast<PLitFrameUpdateInterface&>(frameUpdateInterface).UpdateFrame(pLight, contCash.pLight);
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, litFrameIndex, &contCash.pLight);
						}
						else if (litType == J_LIGHT_TYPE::SPOT)
						{
							JSpotLight* sLight = static_cast<JSpotLight*>(light);
							static_cast<SLitFrameUpdateInterface&>(frameUpdateInterface).UpdateFrame(sLight, contCash.sLight);
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, litFrameIndex, &contCash.sLight);
						}
						else if (litType == J_LIGHT_TYPE::RECT)
						{
							JRectLight* rLight = static_cast<JRectLight*>(light);
							static_cast<ALitFrameUpdateInterface&>(frameUpdateInterface).UpdateFrame(rLight, contCash.rLight);
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, litFrameIndex, &contCash.rLight);
						}
						info->lightUpdateCount += !isRecopy;
					}
					void UpdateShadowFrame(JLight* light,
						JLightPrivate* lp,
						JGraphicDrawTarget::UpdateInfo* info,
						const bool isRecopy)
					{
						auto& frameUpdateInterface = lp->GetFrameUpdateInterface();
						const bool canUpdateShdow = light->IsShadowActivated();
						if (!canUpdateShdow)
							return;

						const int shadowMapFrameIndex = frameUpdateInterface.GetShadowMapFrameIndex(light);
						const J_LIGHT_TYPE litType = light->GetLightType();

						if (litType == J_LIGHT_TYPE::DIRECTIONAL)
						{
							JDirectionalLight* dLight = static_cast<JDirectionalLight*>(light);
							auto dLitFrameUpdateInterface = static_cast<DLitFrameUpdateInterface&>(frameUpdateInterface);
							if (dLight->CanAllocateCsm())
							{
								const uint csmTargetCount = dLitFrameUpdateInterface.GetCsmFrameSize(dLight);
								for (uint i = 0; i < csmTargetCount; ++i)
								{
									dLitFrameUpdateInterface.UpdateFrame(dLight, contCash.csmInfo, i);
									dLitFrameUpdateInterface.UpdateFrame(dLight, contCash.csmDraw, i);
									currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO, shadowMapFrameIndex + i, &contCash.csmInfo);
									currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW, shadowMapFrameIndex + i, &contCash.csmDraw);
								}
							}
							else
							{
								dLitFrameUpdateInterface.UpdateFrame(dLight, contCash.normalShadowMap);
								currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, shadowMapFrameIndex, &contCash.normalShadowMap);
							}
						}
						else if (litType == J_LIGHT_TYPE::POINT)
						{
							JPointLight* pLight = static_cast<JPointLight*>(light);
							static_cast<PLitFrameUpdateInterface&>(frameUpdateInterface).UpdateFrame(pLight, contCash.cubeShadowMap);
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW, shadowMapFrameIndex, &contCash.cubeShadowMap);
						}
						else if (litType == J_LIGHT_TYPE::SPOT)
						{
							JSpotLight* sLight = static_cast<JSpotLight*>(light);
							static_cast<SLitFrameUpdateInterface&>(frameUpdateInterface).UpdateFrame(sLight, contCash.normalShadowMap);
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, shadowMapFrameIndex, &contCash.normalShadowMap);
						}
						else if (litType == J_LIGHT_TYPE::RECT)
						{
							JRectLight* sLight = static_cast<JRectLight*>(light);
							static_cast<ALitFrameUpdateInterface&>(frameUpdateInterface).UpdateFrame(sLight, contCash.normalShadowMap);
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, shadowMapFrameIndex, &contCash.normalShadowMap);
						}
						info->shadowMapUpdateCount += !isRecopy;
					}
					void UpdateDepthTestFrame(JLight* light,
						JLightPrivate* lp,
						JGraphicDrawTarget::UpdateInfo* info,
						const bool isRecopy)
					{
						if (!light->AllowHzbOcclusionCulling() && !light->AllowHdOcclusionCulling())
							return;

						int frameIndex = lp->GetFrameUpdateInterface().GetDepthTestPassFrameIndex(light);
						if (frameIndex == -1)
							return;

						auto dLitFrameUpdateInterface = static_cast<DLitFrameUpdateInterface&>(lp->GetFrameUpdateInterface());
						dLitFrameUpdateInterface.UpdateFrame(static_cast<JDirectionalLight*>(light), contCash.depthTest);
						currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, frameIndex, &contCash.depthTest);
					}
					void UpdateOccFrame(JLight* light,
						JLightPrivate* lp,
						JGraphicDrawTarget::UpdateInfo* info,
						const uint hzbOccQueryCount,
						const uint hzbOccQueryOffset,
						const bool isRecopy)
					{
						if (!light->AllowHzbOcclusionCulling())
							return;

						int frameIndex = lp->GetFrameUpdateInterface().GetHzbOccComputeFrameIndex(light);
						if (frameIndex == -1)
							return;

						auto dLitFrameUpdateInterface = static_cast<DLitFrameUpdateInterface&>(lp->GetFrameUpdateInterface());
						dLitFrameUpdateInterface.UpdateFrame(static_cast<JDirectionalLight*>(light), contCash.hzbOccCmpute, hzbOccQueryCount, hzbOccQueryOffset);
						currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, frameIndex, &contCash.hzbOccCmpute);
						info->hzbOccUpdateCount += !isRecopy;
					}
				};

				LitUpdateHelper litUpdateHelper(currFrameResource, contCash);
				auto updateInfo = target->updateInfo.get();
				for (uint i = 0; i < lightVecCount; ++i)
				{
					JLight* light = static_cast<JLight*>(jLvec[i].Get());
					JLightPrivate* lp = static_cast<JLightPrivate*>(&light->PrivateInterface());
					auto& frameUpdateInterface = lp->GetFrameUpdateInterface();

					const J_LIGHT_TYPE litType = light->GetLightType();
					bool forcedSetFrameDirtyValue = forcedSetFrameDirty[(uint)litType];
					if (light->GetShadowMapType() == J_SHADOW_MAP_TYPE::CSM)
						forcedSetFrameDirtyValue |= hasCamHotUpdate;

					if (frameUpdateInterface.UpdateStart(light, forcedSetFrameDirtyValue))
					{
						litUpdateHelper.UpdateLitFrame(light, lp, updateInfo, true, false);
						litUpdateHelper.UpdateShadowFrame(light, lp, updateInfo, false);
						litUpdateHelper.UpdateDepthTestFrame(light, lp, updateInfo, false);
						litUpdateHelper.UpdateOccFrame(light, lp, updateInfo, hzbOccQueryCount, hzbOccQueryOffset, false);
						frameUpdateInterface.UpdateEnd(light);
						if (frameUpdateInterface.IsLastFrameHotUpdated(light))
							++updateInfo->hotLitghtUpdateCount;
					}
					else
					{
						/*
						* 	light constants에는 shadow map index에 대한 변수가 있으므로
						*	shadow map update시 light constants와 shadow constants를 동시에
						*	Update해줄 필요가 있다. ex)graphic resource destroy인한 index변경 반영
						*/
						if (frameUpdateInterface.HasShadowMapRecopyRequest(light))
						{
							litUpdateHelper.UpdateLitFrame(light, lp, updateInfo, false, true);
							litUpdateHelper.UpdateShadowFrame(light, lp, updateInfo, true);
						}
						else if (frameUpdateInterface.HasLitRecopyRequest(light))
							litUpdateHelper.UpdateLitFrame(light, lp, updateInfo, false, true);
						if (frameUpdateInterface.HasHzbOccComputeRecopyRequest(light))
							litUpdateHelper.UpdateDepthTestFrame(light, lp, updateInfo, true);
						if (frameUpdateInterface.HasHzbOccComputeRecopyRequest(light))
							litUpdateHelper.UpdateOccFrame(light, lp, updateInfo, hzbOccQueryCount, hzbOccQueryOffset, true);
					}
					++litCount[(uint)litType];

					int frameIndex = frameUpdateInterface.GetLitFrameIndex(light);
					if (frameIndex < minFrameIndex[(uint)litType])
						minFrameIndex[(uint)litType] = frameIndex;
				}

				contCash.scenePass.directionalLitSt = minFrameIndex[(int)J_LIGHT_TYPE::DIRECTIONAL];
				contCash.scenePass.directionalLitEd = minFrameIndex[(int)J_LIGHT_TYPE::DIRECTIONAL] + litCount[(uint)J_LIGHT_TYPE::DIRECTIONAL];
				contCash.scenePass.pointLitSt = minFrameIndex[(int)J_LIGHT_TYPE::POINT];
				contCash.scenePass.pointLitEd = minFrameIndex[(int)J_LIGHT_TYPE::POINT] + litCount[(uint)J_LIGHT_TYPE::POINT];
				contCash.scenePass.spotLitSt = minFrameIndex[(int)J_LIGHT_TYPE::SPOT];
				contCash.scenePass.spotLitEd = minFrameIndex[(int)J_LIGHT_TYPE::SPOT] + litCount[(uint)J_LIGHT_TYPE::SPOT];
				contCash.scenePass.rectLitSt = minFrameIndex[(int)J_LIGHT_TYPE::RECT];
				contCash.scenePass.rectLitEd = minFrameIndex[(int)J_LIGHT_TYPE::RECT] + litCount[(uint)J_LIGHT_TYPE::RECT];

				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].uploadCountPerTarget = litCount[(uint)J_LIGHT_TYPE::DIRECTIONAL];
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].uploadCountPerTarget = litCount[(uint)J_LIGHT_TYPE::POINT];
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].uploadCountPerTarget = litCount[(uint)J_LIGHT_TYPE::SPOT];
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT].uploadCountPerTarget = litCount[(uint)J_LIGHT_TYPE::RECT];
			}
			void UpdateSceneRequestor(_Inout_ JGraphicDrawTarget* target)
			{
				using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
				for (auto& data : target->sceneRequestor)
				{
					JCamera* cam = data->jCamera.Get();
					if (FrameUpdateInterface::IsLastUpdated(cam))
						data->isUpdated = true;
				}
			}
			void UpdateShadowRequestor(_Inout_ JGraphicDrawTarget* target)
			{		 
				const bool allowLitCluster = target->scene->AllowLightCulling() &&
					option.isLightCullingActivated &&
					option.allowLightCluster && 
					target->shadowRequestor.size() > 0;

				uint litOffset[(uint)J_LIGHT_TYPE::COUNT];
				std::vector<JCullingUserInterface> cullUserVec;
				uint userVecCount = 0;
			
				if (allowLitCluster)
				{
					const size_t sceneGuid = target->scene->GetGuid();
					litOffset[(uint)J_LIGHT_TYPE::POINT] = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, sceneGuid);
					litOffset[(uint)J_LIGHT_TYPE::SPOT] = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sceneGuid);
					litOffset[(uint)J_LIGHT_TYPE::RECT] = JFrameUpdateData::GetAreaRegistedOffset(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, sceneGuid);
				
					auto camVec = target->scene->GetComponentVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
					userVecCount = (uint)camVec.size();
					cullUserVec.resize(userVecCount);
					
					for (uint i = 0; i < userVecCount; ++i)
						cullUserVec[i] = static_cast<JCamera*>(camVec[i].Get())->CullingUserInterface();			 
				} 
				using FrameUpdateInterface = JLightPrivate::FrameUpdateInterface;
				for (auto& data : target->shadowRequestor)
				{
					JLight* lit = data->jLight.Get();
					JLightPrivate* lp = static_cast<JLightPrivate*>(&lit->PrivateInterface());
					if (lp->GetFrameUpdateInterface().IsLastUpdated(lit))
						data->isUpdated = true;
					if (allowLitCluster && data->isUpdated)
					{
						const J_LIGHT_TYPE litType = lit->GetLightType();
						const int frameIndex = lp->GetFrameUpdateInterface().GetLitFrameIndex(lit);
						if (litType != J_LIGHT_TYPE::DIRECTIONAL)
						{
							bool isCull = true;
							for (uint i = 0; i < userVecCount; ++i)
							{
								isCull &= cullUserVec[i].IsCulled(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::LIGHT, frameIndex);
								if (!isCull)
									break;
							}
							if (isCull)
								data->passNextFrame = true;
						}
					}
				}
			}
			void UpdateFrustumCullingRequestor(_Inout_ JGraphicDrawTarget* target)
			{
				using CamFrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
				for (auto& data : target->frustumCullingRequestor)
				{
					JComponent* comp = data->comp.Get();
					J_COMPONENT_TYPE cType = comp->GetComponentType();
					if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					{
						if (CamFrameUpdateInterface::IsLastFrameHotUpdated(static_cast<JCamera*>(comp)))
							data->isUpdated = true;
					}
					else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					{
						JLight* light = static_cast<JLight*>(comp);
						JLightPrivate* lp = static_cast<JLightPrivate*>(&light->PrivateInterface());
						if (lp->GetFrameUpdateInterface().IsLastFrameHotUpdated(light))
							data->isUpdated = true;
					}
				}
			}
			void UpdateOccCullingRequestor(_Inout_ JGraphicDrawTarget* target)
			{
				using CamFrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
				for (auto& data : target->hzbOccCullingRequestor)
				{
					JComponent* comp = data->comp.Get();
					J_COMPONENT_TYPE cType = comp->GetComponentType();
					JCullingUserInterface cInterface;
					if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					{
						if (CamFrameUpdateInterface::IsLastUpdated(static_cast<JCamera*>(comp)))
							data->isUpdated = true;
						cInterface = static_cast<JCamera*>(comp)->CullingUserInterface();
					}
					else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					{
						JLight* light = static_cast<JLight*>(comp);
						JLightPrivate* lp = static_cast<JLightPrivate*>(&light->PrivateInterface());
						if (lp->GetFrameUpdateInterface().IsLastUpdated(light))
							data->isUpdated = true;
						cInterface = static_cast<JLight*>(comp)->CullingUserInterface();
					}
					if (!cInterface.IsUpdateEnd(J_CULLING_TYPE::HZB_OCCLUSION))
						data->isUpdated = true;
				}
				for (auto& data : target->hdOccCullingRequestor)
				{
					JComponent* comp = data->comp.Get();
					J_COMPONENT_TYPE cType = comp->GetComponentType();
					JCullingUserInterface cInterface;

					if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					{
						if (CamFrameUpdateInterface::IsLastUpdated(static_cast<JCamera*>(comp)))
							data->isUpdated = true;
						cInterface = static_cast<JCamera*>(comp)->CullingUserInterface();
					}
					else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					{
						JLight* light = static_cast<JLight*>(comp);
						JLightPrivate* lp = static_cast<JLightPrivate*>(&light->PrivateInterface());
						if (lp->GetFrameUpdateInterface().IsLastUpdated(light))
							data->isUpdated = true;
						cInterface = static_cast<JLight*>(comp)->CullingUserInterface();
					}
					if (!cInterface.IsUpdateEnd(J_CULLING_TYPE::HD_OCCLUSION))
						data->isUpdated = true;
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS].uploadCountPerTarget = target->hzbOccCullingRequestor.size();
			}
		public:
			void Draw(const bool allowDrawScene)
			{
				alignedObject.ClearAlignedVecElement();
				AllocateRefSet();
				if (allowDrawScene)
				{
					if (option.allowMultiThread)
						DrawUseMultiThread();
					else
						DrawUseSingleThread();
				}
				else
					EndFrame(false);
			}
		private:
			void DrawUseSingleThread()
			{ 
				JGraphicDrawSceneSTSet dataSet;
				adapter->BeginDrawSceneSingleThread(option.deviceType, *drawRefSet, dataSet);
				 
				const J_GRAPHIC_RENDERING_PROCESS objRenderingType = option.allowDeferred ? J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY : J_GRAPHIC_RENDERING_PROCESS::FORWARD;
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.scene = drawTarget->scene;
					helper.drawTarget = drawTarget;
					for (const auto& data : drawTarget->frustumCullingRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						frustumHelper->FrustumCulling(drawTarget->scene, JDrawHelper::CreateFrustumCullingHelper(helper, data->comp));
					}
					if (helper.scene->AllowLightCulling())
					{
						litCullHelper->BindDrawResource(dataSet.bind.get());
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							litCullHelper->ExecuteLightClusterTask(dataSet.litCulling.get(), JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
						} 
					}
					if (option.IsOcclusionActivated())
					{
						//1.draw depth map
						//2.create mipmap and compute hzb
						//3.draw debug map
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->DrawOcclusionDepthMap(dataSet.occDraw.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->ComputeOcclusionCulling(dataSet.hzbCompute.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}

					shadowMap->BindResource(dataSet.bind.get());
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						shadowMap->DrawSceneShadowMap(dataSet.shadowMapDraw.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						debug->ComputeLitDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}

					sceneDraw->BindResource(objRenderingType, dataSet.bind.get());
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						sceneDraw->DrawSceneRenderTarget(dataSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					if (option.allowDeferred)
					{
						sceneDraw->BindResource(J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING, dataSet.bind.get());
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							sceneDraw->DrawSceneShade(dataSet.sceneDraw.get(),
								JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
						}
					}
					if (option.CanUseSSAO())
					{
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingDrawScene(data->jCamera);

							imageP->ApplySsao(dataSet.ssao.get(), copiedHelper);
						}
					}
					if (option.IsOcclusionActivated())
					{
						//1.query test
						//2.extract query result
						//3.draw debug map
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hdOccHelper->DrawOcclusionDepthMap(dataSet.occDraw.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hdOccHelper->ExtractHDOcclusionCullingData(dataSet.hdExtract.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}

					sceneDraw->BindResource(J_GRAPHIC_RENDERING_PROCESS::FORWARD, dataSet.bind.get());
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						sceneDraw->DrawSceneDebugUI(dataSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						outlineHelper->DrawCamOutline(dataSet.outline.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;
					 
						debug->ComputeCamDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}

					if (option.allowDebugLightCulling && helper.scene->AllowLightCulling())
					{
						litCullHelper->BindDebugResource(dataSet.bind.get());
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							litCullHelper->ExecuteLightClusterDebug(dataSet.litCullingDebug.get(),
								JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
						}
					}
				}
				adapter->EndDrawSceneSingeThread(option.deviceType, *drawRefSet);
				EndFrame(true);
			}
			void DrawUseMultiThread()
			{
				BeginFrame();
				ComputeCpuFrustumCulling();

				for (uint i = 0; i < info.frameThreadCount; ++i)
					GraphicThreadInteface::CreateDrawThread(Core::JThreadInitInfo{}, UniqueBind(*workerFunctor, std::move(i)));

				adapter->ExecuteDrawOccTask(option.deviceType, *drawRefSet);
				adapter->ExecuteDrawShadowMapTask(option.deviceType, *drawRefSet);
				adapter->ExecuteDrawSceneTask(option.deviceType, *drawRefSet);

				MidFrame();
				EndFrame(true);
			}
		private:
			void ComputeCpuFrustumCulling()
			{
				JDrawHelper helper(info, option, alignedObject);
				const uint drawListCount = JGraphicDrawList::GetListCount();
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					for (const auto& data : drawTarget->frustumCullingRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						frustumHelper->FrustumCulling(drawTarget->scene, JDrawHelper::CreateFrustumCullingHelper(helper, data->comp));
					}
				}
			}
		private:
			void WorkerThread(uint threadIndex)
			{
				//mostly handle drawing object
				JGraphicThreadOccTaskSet occTaskSet;
				JGraphicThreadShadowMapTaskSet shadowMapTaskSet;
				JGraphicThreadSceneTaskSet sceneTaskSet;

				const J_GRAPHIC_RENDERING_PROCESS objRenderingType = option.allowDeferred ? J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY : J_GRAPHIC_RENDERING_PROCESS::FORWARD;
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);
				helper.SetAllowMultithreadDraw(true);
				helper.SetTheadInfo(info.frameThreadCount, threadIndex);

				if (option.IsOcclusionActivated())
				{
					adapter->SettingDrawOccTask(option.deviceType, *drawRefSet, threadIndex, occTaskSet);
					for (uint i = 0; i < drawListCount; ++i)
					{
						JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
						helper.SetDrawTarget(drawTarget);
						 
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->DrawOcclusionDepthMapMultiThread(occTaskSet.occDraw.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
				}
				adapter->NotifyCompleteDrawOccTask(option.deviceType, *drawRefSet, threadIndex);
				adapter->SettingDrawShadowMapTask(option.deviceType, *drawRefSet, threadIndex, shadowMapTaskSet);
				shadowMap->BindResource(shadowMapTaskSet.bind.get());
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						shadowMap->DrawSceneShadowMapMultiThread(shadowMapTaskSet.shadowMapDraw.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
				}

				adapter->NotifyCompleteDrawShadowMapTask(option.deviceType, *drawRefSet, threadIndex);
				adapter->SettingDrawSceneTask(option.deviceType, *drawRefSet, threadIndex, sceneTaskSet);
				sceneDraw->BindResource(objRenderingType, sceneTaskSet.bind.get());

				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						sceneDraw->DrawSceneRenderTargetMultiThread(sceneTaskSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}
				if (option.IsOcclusionActivated())
				{
					for (uint i = 0; i < drawListCount; ++i)
					{
						JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
						helper.SetDrawTarget(drawTarget);
						if (option.IsOcclusionActivated())
						{
							for (const auto& data : drawTarget->hdOccCullingRequestor)
							{
								if (!data->canDrawThisFrame)
									continue;

								hdOccHelper->DrawOcclusionDepthMapMultiThread(sceneTaskSet.occDraw.get(),
									JDrawHelper::CreateOccCullingHelper(helper, data->comp));
							}
						}
					}
				}
				adapter->NotifyCompleteDrawSceneTask(option.deviceType, *drawRefSet, threadIndex);
			}
			//for multi thread
			void BeginFrame()
			{
				//mostly handle binding resource
				JGraphicBeginFrameSet dataSet;
				adapter->SettingBeginFrame(option.deviceType, *drawRefSet, dataSet);

				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);

				litCullHelper->BindDrawResource(dataSet.bind.get());
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (helper.scene->AllowLightCulling())
					{
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							litCullHelper->ExecuteLightClusterTask(dataSet.litCulling.get(),
								JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
						}
					}
				} 
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.IsOcclusionActivated() && hzbOccHelper->HasPreprocessing())
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsOcclusionActivated() && hdOccHelper->HasPreprocessing())
					{
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hdOccHelper->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (shadowMap->HasPreprocessing())
					{
						for (const auto& data : drawTarget->shadowRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							shadowMap->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
						}
					}
					if (sceneDraw->HasPreprocessing())
					{
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							sceneDraw->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
						}
					}
				}
				adapter->ExecuteBeginFrame(option.deviceType, *drawRefSet);
			}
			void MidFrame()
			{
				JGraphicMidFrameSet dataSet;
				adapter->SettingMidFrame(option.deviceType, *drawRefSet, dataSet);
				 
				//mostly handle compute shader task or drawing debug
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);

				if (option.allowDeferred)
				{
					sceneDraw->BindResource(J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING, dataSet.bind.get());
					for (uint i = 0; i < drawListCount; ++i)
					{
						JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
						helper.SetDrawTarget(drawTarget);
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;
							 
							sceneDraw->DrawSceneShadeMultiThread(dataSet.sceneDraw.get(), 
								JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
						}
					}
				}
				sceneDraw->BindResource(J_GRAPHIC_RENDERING_PROCESS::FORWARD, dataSet.bind.get());
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.IsOcclusionActivated() && hzbOccHelper->HasPostprocessing())
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->EndDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsOcclusionActivated() && hdOccHelper->HasPostprocessing())
					{
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hdOccHelper->EndDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (shadowMap->HasPostprocessing())
					{
						for (const auto& data : drawTarget->shadowRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							shadowMap->EndDraw(dataSet.bind.get(), JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
						}
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						if (copiedHelper.allowDrawDebugObject)
							sceneDraw->DrawSceneDebugUIMultiThread(dataSet.sceneDraw.get(), copiedHelper);
						sceneDraw->EndDraw(dataSet.bind.get(), copiedHelper);
					}
				}
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.IsOcclusionActivated())
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->ComputeOcclusionCulling(dataSet.hzbCompute.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsOcclusionActivated())
					{
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hdOccHelper->ExtractHDOcclusionCullingData(dataSet.hdExtract.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						debug->ComputeLitDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
					if (option.CanUseSSAO())
					{
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingDrawScene(data->jCamera);

							imageP->ApplySsao(dataSet.ssao.get(), copiedHelper);
						}
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						outlineHelper->DrawCamOutline(dataSet.outline.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						debug->ComputeCamDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}

				if (option.allowDebugLightCulling)
				{
					litCullHelper->BindDebugResource(dataSet.bind.get());
					for (uint i = 0; i < drawListCount; ++i)
					{
						JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
						helper.SetDrawTarget(drawTarget);

						if (helper.scene->AllowLightCulling())
						{
							for (const auto& data : drawTarget->sceneRequestor)
							{
								if (!data->canDrawThisFrame)
									continue;

								litCullHelper->ExecuteLightClusterDebug(dataSet.litCullingDebug.get(),
									JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
							}
						}
					}
				}
				adapter->ExecuteMidFrame(option.deviceType, *drawRefSet);
			}
			void EndFrame(const bool isSceneDrawn)
			{
				JGraphicEndConditonSet condSet(isSceneDrawn);
				adapter->SettingEndFrame(option.deviceType, *drawRefSet, condSet);

				if (option.allowDrawGui)
				{
					guiBackendInterface->SettingGuiDrawing();
					guiBackendInterface->Draw(guiAdapter->CreateDrawData(device.get(),
						graphicResourceM.get(),
						currFrameResource,
						option.deviceType,
						guiBackendInterface->GetGuiIdentification()));
				}
				adapter->ExecuteEndFrame(option.deviceType, *drawRefSet, condSet);
			}
		public:
			void Initialize(std::unique_ptr<JGraphicAdapter>&& newAdpter,
				std::unique_ptr<JGuiBackendDataAdapter> newGuiAdapter,
				JGuiBackendInterface* newGuiBackendInterface)
			{
				WindowEventListener::AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);
				LoadData();

				const JVector2F clientSize = JWindow::GetClientSize();
				info.width = clientSize.x;
				info.height = clientSize.y;

				adapter = std::move(newAdpter);
				guiAdapter = std::move(newGuiAdapter);
				guiBackendInterface = newGuiBackendInterface;

				device = adapter->CreateDevice(option.deviceType);
				graphicResourceM = adapter->CreateGraphicResourceManager(option.deviceType);
				cullingM = adapter->CreateCullingManager(option.deviceType);
				sceneDraw = adapter->CreateSceneDraw(option.deviceType);
				shadowMap = adapter->CreateShadowMapDraw(option.deviceType);
				hdOccHelper = adapter->CreateHdOcc(option.deviceType);
				hzbOccHelper = adapter->CreateHzbOcc(option.deviceType);
				litCullHelper = adapter->CreateLightCulling(option.deviceType);
				debug = adapter->CreateDebug(option.deviceType);
				depthTest = adapter->CreateDepthTest(option.deviceType);
				outlineHelper = adapter->CreateOutlineDraw(option.deviceType);
				imageP = adapter->CreateImageProcessing(option.deviceType);

				csmM = std::make_unique<JCsmManager>();
				frustumHelper = std::make_unique<JFrustumCulling>(); 

				if (InitializeResource())
					OnResize();
				else
					Clear();
			}
			void Clear()
			{
				if (JApplicationEngine::GetApplicationState() != J_APPLICATION_STATE::PROJECT_SELECT)
					StoreData();

				drawRefSet = nullptr;
				updateHelper.Clear();
				device->FlushCommandQueue();
				device->StartPublicCommand();
				 
				frustumHelper->Clear();
				frustumHelper.reset();

				csmM->Clear();
				csmM.reset();

				imageP->Clear();
				imageP.reset();

				outlineHelper->Clear();
				outlineHelper.reset();

				debug->Clear();
				debug.reset();

				depthTest->Clear();
				depthTest.reset();

				sceneDraw->Clear();
				sceneDraw.reset();

				shadowMap->Clear();
				shadowMap.reset();
 
				hdOccHelper->Clear();
				hdOccHelper.reset();

				hzbOccHelper->Clear();
				hzbOccHelper.reset();

				litCullHelper->Clear();
				litCullHelper.reset();
				 
				currFrameResource = nullptr;
				info.currFrameResourceIndex = 0;

				for (int i = 0; i < Constants::gNumFrameResources; ++i)
					frameResources[i] = nullptr;

				cullingM->Clear();
				cullingM.reset();

				graphicResourceM->Clear();
				graphicResourceM.reset();

				device->EndPublicCommand();
				device->FlushCommandQueue();

				device->Clear();
				device.reset();

				info.width = 0;
				info.height = 0;

				WindowEventListener::RemoveListener(*JWindow::EvInterface(), guid);
			}
		private:
			bool InitializeResource()
			{
				JGraphicBaseDataSet baseDataSet(info, option); 
				bool res = true;
				res &= device->CreateDeviceObject();
				device->FlushCommandQueue();
				device->StartPublicCommand();
				graphicResourceM->Initialize(device.get());
				res &= device->CreateRefResourceObject(JGraphicDeviceInitSet(info, option, graphicResourceM.get()));
				res &= adapter->CreateFrameResource(option.deviceType, frameResources);
				if (res)
				{
					for (uint i = 0; i < Constants::gNumFrameResources; ++i)
						frameResources[i]->Intialize(device.get(), info);
				}
				currFrameResource = frameResources[info.currFrameResourceIndex].get();
				sceneDraw->Initialize(device.get(), graphicResourceM.get(), baseDataSet);
				shadowMap->Initialize(device.get(), graphicResourceM.get(), info);
				frustumHelper->Initialize();
				hzbOccHelper->Initialize(device.get(), graphicResourceM.get(), info);
				hdOccHelper->Initialize(device.get(), graphicResourceM.get(), info);
				litCullHelper->Initialize(device.get(), graphicResourceM.get(), baseDataSet);
				debug->Initialize(device.get(), graphicResourceM.get(), info);
				depthTest->Initialize(device.get(), graphicResourceM.get(), info);
				outlineHelper->Initialize(device.get(), graphicResourceM.get(), info);
				imageP->Initialize(device.get(), graphicResourceM.get(), baseDataSet);
				device->EndPublicCommand();
				device->FlushCommandQueue();
				return res;
			}
		public:
			void LoadData()
			{
				JFileIOTool tool;
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicOption.txt");
				if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
					return;

				tool.PushExistStack("--Info--");
				JFileIOHelper::LoadAtomicData(tool, info.upObjCount, "UploadObjCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upAniCount, "UploadAniCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upEnginePassCount, "UploadEnginePassCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upScenePassCount, "UploadScenePassCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upCameraCount, "UploadCameraCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upDLightCount, "UploadDirectionalLightCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upPLightCount, "UploadPointLightCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upSLightCount, "UploadSpotLightCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upRLightCount, "UploadRectLightCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upCsmCount, "UploadCsmCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upCubeShadowMapCount, "UploadCubeShadowMapCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upNormalShadowMapCount, "UploadNormalShadowMapCount:");
				JFileIOHelper::LoadAtomicData(tool, info.upMaterialCount, "UploadMaterialCount:");
				JFileIOHelper::LoadAtomicData(tool, info.binding2DTextureCapacity, "Bind2DTextureCount:");
				JFileIOHelper::LoadAtomicData(tool, info.bindingCubeMapCapacity, "BindCubeMapCount:");
				JFileIOHelper::LoadAtomicData(tool, info.bindingShadowTextureCapacity, "BindShadowTextureCount:");
				JFileIOHelper::LoadAtomicData(tool, info.bindingShadowTextureArrayCapacity, "BindShadowTextureArrayCount:");
				JFileIOHelper::LoadAtomicData(tool, info.bindingShadowTextureCubeCapacity, "BindShadowTextureCubeCount:");
				tool.PopStack();

				tool.PushExistStack("--Option--");
				JGraphicOption newOption;
				JFileIOHelper::LoadAtomicData(tool, newOption.isOcclusionQueryActivated, "AllowOcclusionQuery:");
				JFileIOHelper::LoadAtomicData(tool, newOption.isLightCullingActivated, "LightCullingActivated:");

				JFileIOHelper::LoadAtomicData(tool, newOption.allowHZBCorrectFail, "AllowHZBCorrectFail:");
				JFileIOHelper::LoadAtomicData(tool, newOption.allowHDOcclusionUseOccluder, "AllowHDOcclusionUseOccluder:");

				JFileIOHelper::LoadAtomicData(tool, newOption.allowDebugOutline, "AllowOutline:");
				JFileIOHelper::LoadAtomicData(tool, newOption.allowDebugLightCulling, "AllowDebugLightCulling:");

				JFileIOHelper::LoadAtomicData(tool, newOption.allowMultiThread, "AllowMultiThread:");
				JFileIOHelper::LoadAtomicData(tool, newOption.allowDrawGui, "AllowDrawGui:");

				JFileIOHelper::LoadAtomicData(tool, newOption.allowLightCluster, "AllowLightCluster:");
				JFileIOHelper::LoadAtomicData(tool, newOption.allowDeferred, "AllowDeferred:");

				JFileIOHelper::LoadAtomicData(tool, newOption.useHighQualityShadow, "UseHighQualityShadow:");
				JFileIOHelper::LoadAtomicData(tool, newOption.useMiddleQualityShadow, "UseMiddleQualityShadow:");
				JFileIOHelper::LoadAtomicData(tool, newOption.useLowQualityShadow, "UseLowQualityShadow:");

				JFileIOHelper::LoadAtomicData(tool, newOption.useSsao, "UseSsao:"); 

				JFileIOHelper::LoadAtomicData(tool, newOption.clusterXIndex, "ClusterXIndex:");
				JFileIOHelper::LoadAtomicData(tool, newOption.clusterYIndex, "ClusterYIndex:");
				JFileIOHelper::LoadAtomicData(tool, newOption.clusterZIndex, "ClusterZIndex:");
				JFileIOHelper::LoadAtomicData(tool, newOption.lightPerClusterIndex, "LightPerClusterIndex:");
				JFileIOHelper::LoadAtomicData(tool, newOption.clusterNear, "ClusterNear:");
				JFileIOHelper::LoadAtomicData(tool, newOption.clusterPointLightRangeOffset, "ClusterPointLightRangeOffset:");
				JFileIOHelper::LoadAtomicData(tool, newOption.clusterSpotLightRangeOffset, "ClusterSpotLightRangeOffset:");
				JFileIOHelper::LoadAtomicData(tool, newOption.clusterRectLightRangeOffset, "ClusterRectLightRangeOffset:");
	 
				tool.PopStack();
				tool.Close();
				SetOption(newOption);
			}
			void StoreData()
			{
				JFileIOTool tool;
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicOption.txt");
				if (!tool.Begin(path, JFileIOTool::TYPE::JSON))
					return;

				tool.PushMapMember("--Info--");
				JFileIOHelper::StoreAtomicData(tool, info.upObjCount, "UploadObjCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upAniCount, "UploadAniCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upEnginePassCount, "UploadEnginePassCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upScenePassCount, "UploadScenePassCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upCameraCount, "UploadCameraCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upDLightCount, "UploadDirectionalLightCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upPLightCount, "UploadPointLightCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upSLightCount, "UploadSpotLightCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upRLightCount, "UploadRectLightCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upCsmCount, "UploadCsmCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upCubeShadowMapCount, "UploadCubeShadowMapCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upNormalShadowMapCount, "UploadNormalShadowMapCount:");
				JFileIOHelper::StoreAtomicData(tool, info.upMaterialCount, "UploadMaterialCount:");
				JFileIOHelper::StoreAtomicData(tool, info.binding2DTextureCapacity, "Bind2DTextureCount:");
				JFileIOHelper::StoreAtomicData(tool, info.bindingCubeMapCapacity, "BindCubeMapCount:");
				JFileIOHelper::StoreAtomicData(tool, info.bindingShadowTextureCapacity, "BindShadowTextureCount:");
				JFileIOHelper::StoreAtomicData(tool, info.bindingShadowTextureArrayCapacity, "BindShadowTextureArrayCount:");
				JFileIOHelper::StoreAtomicData(tool, info.bindingShadowTextureCubeCapacity, "BindShadowTextureCubeCount:");

				tool.PopStack();
				tool.PushMapMember("--Option--");
				JFileIOHelper::StoreAtomicData(tool, option.isOcclusionQueryActivated, "AllowOcclusionQuery:");
				JFileIOHelper::StoreAtomicData(tool, option.isLightCullingActivated, "LightCullingActivated:");

				JFileIOHelper::StoreAtomicData(tool, option.allowHZBCorrectFail, "AllowHZBCorrectFail:");
				JFileIOHelper::StoreAtomicData(tool, option.allowHDOcclusionUseOccluder, "AllowHDOcclusionUseOccluder:");
			
				JFileIOHelper::StoreAtomicData(tool, option.allowDebugOutline, "AllowOutline:");
				JFileIOHelper::StoreAtomicData(tool, option.allowDebugLightCulling, "AllowDebugLightCulling:");

				JFileIOHelper::StoreAtomicData(tool, option.allowMultiThread, "AllowMultiThread:");
				JFileIOHelper::StoreAtomicData(tool, option.allowDrawGui, "AllowDrawGui:");

				JFileIOHelper::StoreAtomicData(tool, option.allowLightCluster, "AllowLightCluster:");
				JFileIOHelper::StoreAtomicData(tool, option.allowDeferred, "AllowDeferred:");

				JFileIOHelper::StoreAtomicData(tool, option.useHighQualityShadow, "UseHighQualityShadow:");
				JFileIOHelper::StoreAtomicData(tool, option.useMiddleQualityShadow, "UseMiddleQualityShadow:");
				JFileIOHelper::StoreAtomicData(tool, option.useLowQualityShadow, "UseLowQualityShadow:");

				JFileIOHelper::StoreAtomicData(tool, option.useSsao, "UseSsao:"); 

				JFileIOHelper::StoreAtomicData(tool, option.clusterXIndex, "ClusterXIndex:");
				JFileIOHelper::StoreAtomicData(tool, option.clusterYIndex, "ClusterYIndex:");
				JFileIOHelper::StoreAtomicData(tool, option.clusterZIndex, "ClusterZIndex:");
				JFileIOHelper::StoreAtomicData(tool, option.lightPerClusterIndex, "LightPerClusterIndex:");
				JFileIOHelper::StoreAtomicData(tool, option.clusterNear, "ClusterNear:");
				JFileIOHelper::StoreAtomicData(tool, option.clusterPointLightRangeOffset, "ClusterPointLightRangeOffset:");
				JFileIOHelper::StoreAtomicData(tool, option.clusterSpotLightRangeOffset, "ClusterSpotLightRangeOffset:");
				JFileIOHelper::StoreAtomicData(tool, option.clusterRectLightRangeOffset, "ClusterRectLightRangeOffset:");

				tool.PopStack();
				tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
			}
			void WriteLastRsTexture()
			{
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				{
					// 0 is main camera
					JApplicationProjectInfo* opendInfo = JApplicationProject::GetOpenProjectInfo();
					if (opendInfo == nullptr)
						MessageBox(0, L"get open proejct info error", 0, 0);
					 
					//main scene
					auto firstDrawTarget = JGraphicDrawList::GetDrawScene(0);
					auto firstCam = firstDrawTarget->scene->FindFirstSelectedCamera(false);

					if (firstCam != nullptr)
					{
						auto gUser = firstCam->GraphicResourceUserInterface();
						const int index = gUser.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
						graphicResourceM->StoreTexture(device.get(), J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, index, opendInfo->lastRsPath());
					}
				}
			}
		public:
			JEventInterface* EvInterface()final
			{
				return this;
			}
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
			{
				if (senderGuid == guid)
					return;

				if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
					OnResize();
			}
		private:
			template<J_GRAPHIC_EVENT_TYPE evType, typename ...Param>
			void NotifyEvent(const size_t guid, Param... var)
			{
				std::unique_ptr<JGraphicEventStruct> evStruct = nullptr;
				if constexpr (evType == J_GRAPHIC_EVENT_TYPE::OPTION_CHANGED)
					evStruct = std::make_unique<JGraphicOptionChangedEvStruct>(var...);

				if (evStruct == nullptr)
					return;

				GraphicEventManager::NotifyEvent(guid, evType, evStruct.get());
			}
		};
#pragma endregion


#pragma region Interface

		JGraphicInfo JGraphic::GetGraphicInfo()const noexcept
		{
			return impl->GetGraphicInfo();
		}
		JGraphicOption JGraphic::GetGraphicOption()const noexcept
		{
			return impl->GetGraphicOption();
		}
		void JGraphic::GetLastDeviceErrorInfo(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)const noexcept
		{
			impl->GetLastDeviceErrorInfo(errorCode, errorMsg);
		}
		void JGraphic::SetGraphicOption(JGraphicOption newGraphicOption)noexcept
		{
			impl->SetOption(newGraphicOption);
		}
		GraphicEventInterface* JGraphic::EventInterface()noexcept
		{
			return impl->EvInterface();
		}
		JGraphic::JGraphic()
			:impl(std::make_unique<JGraphicImpl>(Core::MakeGuid(), this))
		{}
		JGraphic::~JGraphic()
		{
			impl.reset();
		}

		using ResourceInterface = JGraphicPrivate::ResourceInterface;
		using CullingInterface = JGraphicPrivate::CullingInterface;
		using CsmInterface = JGraphicPrivate::CsmInterface;
		using DebugInterface = JGraphicPrivate::DebugInterface;
		using MainAccess = JGraphicPrivate::MainAccess;

		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateResource(const JGraphicResourceCreationDesc& createDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			return JinEngine::JGraphic::Instance().impl->CreateResource(createDesc, rType);
		}
		bool ResourceInterface::CreateOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
		{
			return JinEngine::JGraphic::Instance().impl->CreateOption(info, opType);
		}
		bool ResourceInterface::DestroyGraphicTextureResource(JGraphicResourceInfo* info)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyGraphicTextureResource(info);
		}
		bool ResourceInterface::DestroyGraphicOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyGraphicOption(info, optype);
		}
		bool ResourceInterface::SetMipmap(const JUserPtr<JGraphicResourceInfo>& info, JTextureCreationDesc createDesc)
		{
			return JinEngine::JGraphic::Instance().impl->CreateMipmap(info, createDesc);
		}
		JOwnerPtr<JGraphicShaderDataHolderBase> ResourceInterface::StuffGraphicShaderPso(const JGraphicShaderInitData& shaderData)
		{
			return JinEngine::JGraphic::Instance().impl->StuffGraphicShaderPso(shaderData);
		}
		JOwnerPtr<JComputeShaderDataHolderBase> ResourceInterface::StuffComputeShaderPso(const JComputeShaderInitData& shaderData)
		{
			return JinEngine::JGraphic::Instance().impl->StuffComputeShaderPso(shaderData);
		}
		bool ResourceInterface::MipmapBindForDebug(const JUserPtr<JGraphicResourceInfo>& info, _Out_ std::vector<ResourceHandle>& gpuHandle, _Out_ std::vector<Core::JDataHandle>& dataHandle)
		{
			return JinEngine::JGraphic::Instance().impl->MipmapBindForDebug(info, gpuHandle, dataHandle);
		}
		void ResourceInterface::ClearMipmapBind(_In_ std::vector<Core::JDataHandle>& dataHandle)
		{
			JinEngine::JGraphic::Instance().impl->ClearMipmapBind(dataHandle);
		}

		JUserPtr<JCullingInfo> CullingInterface::CreateFrsutumCullingResultBuffer(const J_CULLING_TARGET target, const bool useGpu)
		{
			return JinEngine::JGraphic::Instance().impl->CreateFrsutumCullingResultBuffer(target, useGpu);
		}
		JUserPtr<JCullingInfo> CullingInterface::CreateHzbOccCullingResultBuffer()
		{
			return JinEngine::JGraphic::Instance().impl->CreateHzbCullingResultBuffer();
		}
		JUserPtr<JCullingInfo> CullingInterface::CreateHdOccCullingResultBuffer()
		{
			return JinEngine::JGraphic::Instance().impl->CreateHdCullingResultBuffer();
		}
		bool CullingInterface::DestroyCullignData(JCullingInfo* cullingInfo)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyCullignData(cullingInfo);
		}

		bool CsmInterface::RegisterHandler(JCsmHandlerInterface* handler)
		{
			return JinEngine::JGraphic::Instance().impl->RegisterHandler(handler);
		}
		bool CsmInterface::DeRegisterHandler(JCsmHandlerInterface* handler)
		{
			return JinEngine::JGraphic::Instance().impl->DeRegisterHandler(handler);
		}
		bool CsmInterface::RegisterTarget(JCsmTargetInterface* target)
		{
			return JinEngine::JGraphic::Instance().impl->RegisterTarget(target);
		}
		bool CsmInterface::DeRegisterTarget(JCsmTargetInterface* target)
		{
			return JinEngine::JGraphic::Instance().impl->DeRegisterTarget(target);
		}

		JGraphicResourceManager* DebugInterface::GetGraphicResourceManager()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetGraphicResourceManager();
		}

		void MainAccess::Initialize(std::unique_ptr<JGraphicAdapter>&& adapter,
			std::unique_ptr<JGuiBackendDataAdapter>&& guiAdapter,
			JGuiBackendInterface* JGuiBackendInterface)
		{
			JinEngine::JGraphic::Instance().impl->Initialize(std::move(adapter), std::move(guiAdapter), JGuiBackendInterface);
		}
		void MainAccess::Clear()
		{
			JinEngine::JGraphic::Instance().impl->Clear();
		}
		std::unique_ptr<JGuiInitData> MainAccess::GetGuiInitData()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetGuiInitData();
		}
		void MainAccess::UpdateWait()
		{
			JinEngine::JGraphic::Instance().impl->UpdateWait();
		}
		void MainAccess::UpdateFrame()
		{
			JinEngine::JGraphic::Instance().impl->UpdateFrame();
		}
		void MainAccess::Draw(const bool allowDrawScene)
		{
			JinEngine::JGraphic::Instance().impl->Draw(allowDrawScene);
		}
		void MainAccess::FlushCommandQueue()
		{
			JinEngine::JGraphic::Instance().impl->FlushCommandQueue();
		}
		void MainAccess::WriteLastRsTexture()
		{
			JinEngine::JGraphic::Instance().impl->WriteLastRsTexture();
		}
#pragma endregion
	}
}