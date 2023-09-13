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
#include"DepthMap/JDepthMapDebug.h"
#include"DepthMap/JDepthTest.h" 
#include"Culling/JCullingInfo.h"
#include"Culling/JCullingManager.h"
#include"Culling/Frustum/JFrustumCulling.h"
#include"Culling/Occlusion/JHardwareOccCulling.h"
#include"Culling/Occlusion/JHZBOccCulling.h"
#include"Outline/JOutline.h"
 
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
#include"Buffer/JGraphicBuffer.h" 

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


#include"../Develop/Debug/JDevelopDebug.h"
namespace JinEngine
{
	using namespace DirectX;
	namespace Graphic
	{
		class JConstantCash
		{
		public:
			JScenePassConstants scenePass;
			JAnimationConstants ani;
		public:
			JCameraConstants camContants;
		public:
			JDirectionalLightConstants dLight;
			JPointLightConstants pLight;
			JSpotLightConstants sLight;
			JCsmConstants csmInfo;
			JShadowMapArrayDrawConstants csmDraw;				//cascade shadow map
			JShadowMapCubeDrawConstants cubeShadowMap;	//cube shadow map
			JShadowMapDrawConstants normalShadowMap;	//normal shadow map
			JHzbOccComputeConstants hzbOccCmpute;
		public:
			JDepthTestPassConstants depthTest;
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
		} 

#pragma region Impl
		class JGraphic::JGraphicImpl : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>
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
			int currFrameResourceIndex = 0;
		public:
			JGraphicInfo info;
			JGraphicOption option;
			JUpdateHelper updateHelper;
			JAlignedObject alignedObject;
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
			std::unique_ptr<JDepthMapDebug> depthMapDebug;
			std::unique_ptr<JDepthTest> depthTest;
			std::unique_ptr<JOutline> outlineHelper; 
			std::unique_ptr<JGraphicDrawReferenceSet> drawRefSet;
		private:
			std::unique_ptr<WorkerThreadF::Functor> workerFunctor;
		private:
			JGuiBackendInterface* guiBackendInterface;
			std::unique_ptr<JGuiBackendDataAdapter> guiAdapter;
		public:
			JGraphicImpl(const size_t guid, JGraphic* thisGraphic)
				:guid(guid), thisGraphic(thisGraphic)
			{
				IntializeGraphicInfo();
				RegisterResouceNotifyFunc();
				workerFunctor = std::make_unique<WorkerThreadF::Functor>(&JGraphicImpl::WorkerThread, this);
			}
			~JGraphicImpl()
			{ }
		private:
			//CallOnece
			void IntializeGraphicInfo()
			{
				const uint occMipMapViewCapa = JGraphicResourceManager::GetOcclusionMipMapViewCapacity();
				info.occlusionWidth = std::pow(2, occMipMapViewCapa - 1);
				info.occlusionHeight = std::pow(2, occMipMapViewCapa - 1);
				info.occlusionMinSize = JGraphicResourceManager::GetOcclusionMinSize();
				info.occlusionMapCapacity = occMipMapViewCapa;
				info.occlusionMapCount = JMathHelper::Log2Int(info.occlusionWidth) - JMathHelper::Log2Int(JGraphicResourceManager::GetOcclusionMinSize()) + 1;
				info.frameThreadCount = _JThreadManager::Instance().GetReservedSpaceCount(Core::J_THREAD_USE_CASE_TYPE::GRAPHIC_DRAW);
			}
			//CallOnece
			void RegisterResouceNotifyFunc()
			{
				auto objGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetMeshCount();
					return sum;
				};
				auto enginePassGetElemenLam = []()->uint {return 1; };
				auto scenePassGetElementLam = []() {return _JSceneManager::Instance().GetActivatedSceneCount(); };
				auto aniGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
					return sum;
				};
				auto camGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
					return sum;
				};
				auto materialGetElementLam = []() {return _JResourceManager::Instance().GetResourceCount<JMaterial>(); };
				auto boundObjGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
					return sum;
				};
				auto hzbRequestorGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->hzbOccCullingRequestor.size();
					return sum;
				};
				auto hzbObjectGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
					return sum;
				};
				auto litAndShadowCountLam = []()
				{
					uint dLitCount = 0;
					uint pLitCount = 0;
					uint sLitCount = 0;

					uint csmCount = 0;
					uint pShadowCount = 0;
					uint normalShadowCount = 0;

					using GCash = JScenePrivate::CashInterface;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
					{
						auto scene = JGraphicDrawList::GetDrawScene(i)->scene;
						auto& vec = GCash::GetComponentCashVec(scene.Get(), J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
						for (const auto& data : vec)
						{
							auto lit = static_cast<JLight*>(data.Get());
							const J_LIGHT_TYPE lType = lit->GetLightType();
							const J_SHADOW_MAP_TYPE smType = lit->GetShadowMapType();

							if (lType == J_LIGHT_TYPE::DIRECTIONAL)
								++dLitCount;
							else if (lType == J_LIGHT_TYPE::POINT)
								++pLitCount;
							else if (lType == J_LIGHT_TYPE::SPOT)
								++sLitCount;

							if (smType == J_SHADOW_MAP_TYPE::CSM)
								++csmCount;
							else if (smType == J_SHADOW_MAP_TYPE::CUBE)
								++pShadowCount;
							else if (smType == J_SHADOW_MAP_TYPE::NORMAL)
								++normalShadowCount;
						}
					}
					
					auto& helper = _JGraphic::Instance().impl->updateHelper;
					helper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].count = dLitCount;
					helper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].count = pLitCount;
					helper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].count = sLitCount;
					helper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO].count = csmCount;
					helper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW].count = csmCount;
					helper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW].count = pShadowCount;
					helper.uData[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].count = normalShadowCount;
				};
				auto depthTestGetElementLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					return impl->cullingM->GetCullingInfoCount(J_CULLING_TYPE::HZB_OCCLUSION) +
						impl->cullingM->GetCullingInfoCount(J_CULLING_TYPE::HD_OCCLUSION);
				};

				using GetElementCount = JUpdateHelper::GetElementCountT::Ptr;
				std::unordered_map<J_UPLOAD_FRAME_RESOURCE_TYPE, GetElementCount> uGetCountFunc
				{
					{J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, objGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS, enginePassGetElemenLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS, scenePassGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, aniGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, camGetElementLam},{J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, materialGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, boundObjGetElementLam},{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, hzbRequestorGetElementLam}, 	
					{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, hzbObjectGetElementLam},{J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, depthTestGetElementLam}
				}; 

				using NotifyUpdateCapacity = JUpdateHelper::NotifyUpdateCapacityT::Callable;
				auto updateFrustumAndHdCullingResultCapaLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					const uint newCapa = impl->currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT);
					impl->cullingM->ReBuildBuffer(J_CULLING_TYPE::FRUSTUM, impl->device.get(), newCapa);
					impl->cullingM->ReBuildBuffer(J_CULLING_TYPE::HD_OCCLUSION, impl->device.get(), newCapa);
					impl->hdOccHelper->NotifyReBuildHdOccBuffer(impl->device.get(), newCapa, impl->cullingM->GetCullingInfoVec(J_CULLING_TYPE::HD_OCCLUSION));
				};
				auto updateHzbOccResultCapaLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					const uint newCapa = impl->currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT);
					
					impl->cullingM->ReBuildBuffer(J_CULLING_TYPE::HZB_OCCLUSION, impl->device.get(), newCapa); 
					impl->hzbOccHelper->NotifyReBuildHzbOccBuffer(impl->device.get(), newCapa, impl->cullingM->GetCullingInfoVec(J_CULLING_TYPE::HZB_OCCLUSION));
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
				//updateHelper.RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, std::make_unique<NotifyUpdateCapacity>(updateHdOccResultCapaLam));
				updateHelper.RegisterCallable(litAndShadowCountLam);

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
				if (newGraphicOption.isHDOcclusionAcitvated && newGraphicOption.isHZBOcclusionActivated)
				{
					if (!option.isHDOcclusionAcitvated)
						newGraphicOption.isHZBOcclusionActivated = false;
					else
						newGraphicOption.isHDOcclusionAcitvated = false;
				}
				option = newGraphicOption;
			}
		public:
			JUserPtr<JGraphicResourceInfo> CreateSceneDepthStencilResource()
			{
				return graphicResourceM->CreateSceneDepthStencilResource(device.get(), info.width, info.height);
			}
			JUserPtr<JGraphicResourceInfo> CreateDebugDepthStencilResource()
			{
				return graphicResourceM->CreateDebugDepthStencilResource(device.get(), info.width, info.height);
			}
			JUserPtr<JGraphicResourceInfo> CreateLayerDepthDebugResource(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0 )
					textureWidth = info.width; 
				if (textureHeight == 0)
					textureHeight = info.height;

				return graphicResourceM->CreateLayerDepthDebugResource(device.get(), info.width, info.height);
			}
			void CreateHZBOcclusionResource(_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo, _Out_ JUserPtr<JGraphicResourceInfo>& outOccMipMapInfo)
			{
				graphicResourceM->CreateHZBOcclusionResource(device.get(), info.occlusionWidth, 
					info.occlusionHeight,
					outOccDsInfo, outOccMipMapInfo);
			}
			JUserPtr<JGraphicResourceInfo> CreateOcclusionResourceDebug(const bool isHzb)
			{
				return graphicResourceM->CreateOcclusionResourceDebug(device.get(), info.occlusionWidth, info.occlusionHeight, isHzb);
			}
			JUserPtr<JGraphicResourceInfo> Create2DTexture(const std::wstring& path, const std::wstring& oriFormat)
			{
				return graphicResourceM->Create2DTexture(device.get(), path, oriFormat);
			}
			JUserPtr<JGraphicResourceInfo> CreateCubeMap(const std::wstring& path, const std::wstring& oriFormat)
			{
				return graphicResourceM->CreateCubeMap(device.get(), path, oriFormat);
			}
			JUserPtr<JGraphicResourceInfo> CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0)
					textureWidth = info.width;
				if (textureHeight == 0)
					textureHeight = info.height;

				return graphicResourceM->CreateRenderTargetTexture(device.get(), textureWidth, textureHeight);
			}
			JUserPtr<JGraphicResourceInfo> CreateShadowMapTexture(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0 || textureHeight == 0)
					return nullptr;

				return graphicResourceM->CreateShadowMapTexture(device.get(), textureWidth, textureHeight);
			}
			JUserPtr<JGraphicResourceInfo> CreateShadowMapArrayTexture(uint textureWidth, uint textureHeight, const uint count)
			{
				if (textureWidth == 0 || textureHeight == 0 || count == 0)
					return nullptr;

				return graphicResourceM->CreateShadowMapArrayTexture(device.get(), textureWidth, textureHeight, count);
			}
			JUserPtr<JGraphicResourceInfo> CreateShadowMapCubeTexture(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0 || textureHeight == 0)
					return nullptr;

				return graphicResourceM->CreateShadowMapCubeTexture(device.get(), textureWidth, textureHeight);
			}
			JUserPtr<JGraphicResourceInfo> CreateVertexBuffer(const std::vector<JStaticMeshVertex>& vertex)
			{
				return graphicResourceM->CreateVertexBuffer(device.get(), vertex); 
			}
			JUserPtr<JGraphicResourceInfo> CreateVertexBuffer(const std::vector<JSkinnedMeshVertex>& vertex)
			{
				return graphicResourceM->CreateVertexBuffer(device.get(), vertex);
			}
			JUserPtr<JGraphicResourceInfo> CreateIndexBuffer(std::vector<uint32>& index)
			{
				return graphicResourceM->CreateIndexBuffer(device.get(), index);
			}
			JUserPtr<JGraphicResourceInfo> CreateIndexBuffer(std::vector<uint16>& index)
			{
				return graphicResourceM->CreateIndexBuffer(device.get(), index);
			}
			bool DestroyGraphicTextureResource(JGraphicResourceInfo* info)
			{
				if (info == nullptr)
					return false;

				return graphicResourceM->DestroyGraphicTextureResource(device.get(), info);
			}
		public:
			JUserPtr<JCullingInfo> CreateFrsutumCullingResultBuffer()
			{ 
				return cullingM->CreateFrsutumData(currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT)); 
			}
			JUserPtr<JCullingInfo> CreateHzbCullingResultBuffer()
			{
				auto user = cullingM->CreateHzbOcclusionData(device.get(), currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT));
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
				auto user = cullingM->CreateHdOcclusionData(device.get(), currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT));
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
				alignedObject.opaqueVec.push_back(JAlignedObject::ObjectVec());
				return user; 
			}
			bool DestroyCullignData(JCullingInfo* cullingInfo)
			{
				if (cullingInfo == nullptr)
					return false;

				const J_CULLING_TYPE cType = cullingInfo->GetCullingType();
				bool res = false;
				if (cType == J_CULLING_TYPE::HZB_OCCLUSION)
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
					alignedObject.opaqueVec.erase(alignedObject.opaqueVec.begin() + cullingInfo->GetArrayIndex());
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
				auto result = sceneDraw->CreateGraphicShader(device.get(), graphicResourceM.get(), shaderData);
				device->EndPublicCommand();
				device->FlushCommandQueue();

				return std::move(result);
			}
			JOwnerPtr<JComputeShaderDataHolderBase> StuffComputeShaderPso(const JComputeShaderInitData& shaderData)
			{
				JOwnerPtr<JComputeShaderDataHolderBase> result = nullptr;
				device->FlushCommandQueue();
				device->StartPublicCommand();
				if (shaderData.cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_COPY ||
					shaderData.cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING ||
					shaderData.cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION)
					result = hzbOccHelper->CreateComputeShader(device.get(), graphicResourceM.get(), shaderData);			 
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
			void ReCompileGraphicShader()
			{  
				sceneDraw->NotifyChangeGraphicShaderMacro(device.get(), info);
				auto shaderVec = JShader::StaticTypeInfo().GetInstanceRawPtrVec();
				for (auto& data : shaderVec)
				{
					JShader* shader = static_cast<JShader*>(data);
					if (!shader->IsComputeShader())
						JShaderPrivate::CompileInterface::RecompileGraphicShader(shader);
				}
			}
		public:
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
				device->ResizeWindow({ info, option, graphicResourceM .get()});
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
		public:
			void UpdateWait()
			{
				currFrameResourceIndex = (currFrameResourceIndex + 1) % Constants::gNumFrameResources;
				currFrameResource = frameResources[currFrameResourceIndex].get();
				
				device->UpdateWait(currFrameResource->GetFenceValue());
			}
			void UpdateFrame()
			{
				updateHelper.Clear();
				for (const auto& data : updateHelper.getElementMultiCount)
					(*data)(nullptr);

				for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				{
					if(!updateHelper.uData[i].useGetMultiCount)
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
						ReCompileGraphicShader();		//use graphic info
					updateHelper.NotifyUpdateFrameCapacity(*thisGraphic);	//use graphic info				 
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}
				else if (updateHelper.hasRecompileShader)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();
					ReCompileGraphicShader();
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}

				const uint drawListCount = JGraphicDrawList::GetListCount();
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					const bool isAllowOcclusion = drawTarget->scene->IsMainScene() && option.isOcclusionQueryActivated;
					const uint sceneDrawReqCount = (uint)drawTarget->sceneRequestor.size();
					drawTarget->UpdateStart();
					UpdateSceneAnimationCB(drawTarget->scene, drawTarget);
					UpdateSceneCameraCB(drawTarget->scene, drawTarget);
					UpdateSceneLightCB(drawTarget->scene, drawTarget);	 
					UpdateSceneObjectCB(drawTarget->scene, drawTarget);
					UpdateSceneRequestor(drawTarget);
					UpdateShadowRequestor(drawTarget);
					UpdateFrustumCullingRequestor(drawTarget);
					UpdateOccCullingRequestor(drawTarget);
					UpdateScenePassCB(drawTarget->scene);	//always update && has order dependency(Light)
					drawTarget->UpdateEnd();
				}
				UpdateEnginePassCB(0);	//always update
				UpdateMaterialCB();

				//Debug
				if(hzbOccHelper->CanReadBackDebugInfo())
					hzbOccHelper->StreamOutDebugInfo(JApplicationProject::RootPath() + L"\\Hzb.txt");
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

				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].offset += renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].offset += renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].offset += scene->GetMeshCount();
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
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].offset += matCount;
			}
			void UpdateEnginePassCB(const uint frameIndex)
			{
				JEnginePassConstants passContants;
				passContants.appTotalTime = JEngineTimer::Data().TotalTime();
				passContants.appDeltaTime = JEngineTimer::Data().DeltaTime();

				auto currPassCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS);
				currPassCB->CopyData(frameIndex, &passContants);
				++updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS].offset;
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
				const uint hzbOccQueryOffset = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].offset;

				auto currCameraCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA);
				auto currDepthCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS);
				auto currHzbOccReqCB = currFrameResource->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS);
				 
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
					}
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].offset += cameraCount;
			}
			void UpdateSceneLightCB(_In_ const JUserPtr<JScene>& scene, _Inout_ JGraphicDrawTarget* target)
			{
				const std::vector<JUserPtr<JComponent>>& jLvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
				const uint lightVecCount = (uint)jLvec.size();
				const uint hzbOccQueryCount = scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint hzbOccQueryOffset = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].offset;

				uint litCount[(uint)J_LIGHT_TYPE::COUNT] = {0, 0, 0}; 

				bool hasLitUpdate = false;
				const bool forcedSetFrameDirty[(uint)J_LIGHT_TYPE::COUNT]
				{
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].setDirty,

					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].setDirty | 
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW].setDirty,

					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].setDirty | 
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].setDirty,
				};
			  
				using LitFrameUpdateInterface = JLightPrivate::FrameUpdateInterface;
				using DLitFrameUpdateInterface = JDirectionalLightPrivate::FrameUpdateInterface;
				using PLitFrameUpdateInterface = JPointLightPrivate::FrameUpdateInterface;
				using SLitFrameUpdateInterface = JSpotLightPrivate::FrameUpdateInterface;
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
					if (frameUpdateInterface.UpdateStart(light, forcedSetFrameDirty[(uint)litType]))
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
						if (frameUpdateInterface.HasLitRecopyRequest(light))
							litUpdateHelper.UpdateLitFrame(light, lp, updateInfo, false, true);
						if (frameUpdateInterface.HasShadowMapRecopyRequest(light))
							litUpdateHelper.UpdateShadowFrame(light, lp, updateInfo, true);
						if (frameUpdateInterface.HasHzbOccComputeRecopyRequest(light))
							litUpdateHelper.UpdateDepthTestFrame(light, lp, updateInfo, true);
						if (frameUpdateInterface.HasHzbOccComputeRecopyRequest(light))
							litUpdateHelper.UpdateOccFrame(light, lp, updateInfo, hzbOccQueryCount, hzbOccQueryOffset, true);
					}
					++litCount[(uint)litType];
				}
				 
				contCash.scenePass.directionalLitSt = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].offset;
				contCash.scenePass.directionalLitEd = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].offset + litCount[(uint)J_LIGHT_TYPE::DIRECTIONAL];
				contCash.scenePass.pointLitSt = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].offset;
				contCash.scenePass.pointLitEd = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].offset + litCount[(uint)J_LIGHT_TYPE::POINT];
				contCash.scenePass.spotLitSt = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].offset;
				contCash.scenePass.spotLitEd = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].offset + litCount[(uint)J_LIGHT_TYPE::SPOT];
	 
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].offset += litCount[(uint)J_LIGHT_TYPE::DIRECTIONAL];
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].offset += litCount[(uint)J_LIGHT_TYPE::POINT];
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].offset += litCount[(uint)J_LIGHT_TYPE::SPOT];
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
				using FrameUpdateInterface = JLightPrivate::FrameUpdateInterface;
				for (auto& data : target->shadowRequestor)
				{
					JLight* lit = data->jLight.Get();
					JLightPrivate* lp = static_cast<JLightPrivate*>(&lit->PrivateInterface());
					if (lp->GetFrameUpdateInterface().IsLastUpdated(lit))
						data->isUpdated = true;
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
						if (CamFrameUpdateInterface::IsLastFrameHotUpdated(static_cast<JCamera*>(comp)))
							data->isUpdated = true;
						cInterface = static_cast<JCamera*>(comp)->CullingUserInterface();
					}
					else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					{
						JLight* light = static_cast<JLight*>(comp);
						JLightPrivate* lp = static_cast<JLightPrivate*>(&light->PrivateInterface());
						if (lp->GetFrameUpdateInterface().IsLastFrameHotUpdated(light))
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
						if (CamFrameUpdateInterface::IsLastFrameHotUpdated(static_cast<JCamera*>(comp)))
							data->isUpdated = true;
						cInterface = static_cast<JCamera*>(comp)->CullingUserInterface();
					}
					else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					{
						JLight* light = static_cast<JLight*>(comp);
						JLightPrivate* lp = static_cast<JLightPrivate*>(&light->PrivateInterface());
						if (lp->GetFrameUpdateInterface().IsLastFrameHotUpdated(light))
							data->isUpdated = true;
						cInterface = static_cast<JLight*>(comp)->CullingUserInterface();
					}
					if(!cInterface.IsUpdateEnd(J_CULLING_TYPE::HD_OCCLUSION))
						data->isUpdated = true;
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS].offset += target->hzbOccCullingRequestor.size();
			}
		public:
			void Draw(const bool allowDrawScene)
			{
				drawRefSet = std::make_unique<JGraphicDrawReferenceSet>(info, option,
					device.get(),
					graphicResourceM.get(),
					cullingM.get(),
					currFrameResource,
					depthMapDebug.get(),
					depthTest.get());

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
				//AlignObject();

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

						frustumHelper->FrustumCulling(drawTarget->scene, data->comp);
					} 
					if (option.IsHZBOccActivated())
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
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->DrawOcclusionDebugMap(dataSet.occDebug.get(),
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

						depthMapDebug->DrawLitDepthDebug(dataSet.depthMapDebug.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
					sceneDraw->BindResource(dataSet.bind.get());
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						sceneDraw->DrawSceneRenderTarget(dataSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					if (option.IsHDOccActivated())
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

						depthMapDebug->DrawCamDepthDebug(dataSet.depthMapDebug.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}

				adapter->EndDrawSceneSingeThread(option.deviceType, *drawRefSet);
				EndFrame(true);
			}
			void DrawUseMultiThread()
			{ 
				BeginFrame();
				ComputeFrustumCulling();

				for (uint i = 0; i < info.frameThreadCount; ++i)
					GraphicThreadInteface::CreateDrawThread(Core::JThreadInitInfo{}, UniqueBind(*workerFunctor, std::move(i)));

				adapter->ExecuteDrawOccTask(option.deviceType, *drawRefSet);
				adapter->ExecuteDrawShadowMapTask(option.deviceType, *drawRefSet);
				adapter->ExecuteDrawSceneTask(option.deviceType, *drawRefSet);
 
				MidFrame();
				EndFrame(true);
			} 	
		private:
			void ComputeFrustumCulling()
			{
				const uint drawListCount = JGraphicDrawList::GetListCount();
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					for (const auto& data : drawTarget->frustumCullingRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;
						 
						frustumHelper->FrustumCulling(drawTarget->scene, data->comp);
					}
				}
			}
		private:
			void WorkerThread(uint threadIndex)
			{
				JGraphicThreadOccTaskSet occTaskSet;
				JGraphicThreadShadowMapTaskSet shadowMapTaskSet;
				JGraphicThreadSceneTaskSet sceneTaskSet;

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

						if (option.IsHZBOccActivated())
						{
							for (const auto& data : drawTarget->hzbOccCullingRequestor)
							{
								if (!data->canDrawThisFrame)
									continue;

								hzbOccHelper->DrawOcclusionDepthMapMultiThread(occTaskSet.occDraw.get(),
									JDrawHelper::CreateOccCullingHelper(helper, data->comp));
							}
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
				sceneDraw->BindResource(sceneTaskSet.bind.get());

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
						if (option.IsHDOccActivated())
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
				JGraphicBeginFrameSet dataSet;
				adapter->SettingBeginFrame(option.deviceType, *drawRefSet, dataSet);
	 
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);

				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);
			
					if (option.IsHZBOccActivated() && hzbOccHelper->HasPreprocessing())
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;
		 
							hzbOccHelper->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsHDOccActivated() && hdOccHelper->HasPreprocessing())
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
				sceneDraw->BindResource(dataSet.bind.get());
				 
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.IsHZBOccActivated() && hzbOccHelper->HasPostprocessing())
					{ 
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->EndDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsHDOccActivated() && hdOccHelper->HasPostprocessing())
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

						if (copiedHelper.allowDrawDebug)
							sceneDraw->DrawSceneDebugUI(dataSet.sceneDraw.get(), copiedHelper);
						sceneDraw->EndDraw(dataSet.bind.get(), copiedHelper);
					}
				}
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.IsHZBOccActivated())
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->ComputeOcclusionCulling(dataSet.hzbCompute.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							hzbOccHelper->DrawOcclusionDebugMap(dataSet.occDebug.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsHDOccActivated())
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

						depthMapDebug->DrawLitDepthDebug(dataSet.depthMapDebug.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
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

						depthMapDebug->DrawCamDepthDebug(dataSet.depthMapDebug.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
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
				AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);
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
				depthMapDebug = adapter->CreateDepthMapDebug(option.deviceType);
				depthTest = adapter->CreateDepthTest(option.deviceType);
				outlineHelper = adapter->CreateOutlineDraw(option.deviceType);

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
 
				outlineHelper->Clear();
				outlineHelper.reset();

				depthMapDebug->Clear();
				depthMapDebug.reset();

				depthTest->Clear();
				depthTest.reset();
				 
				sceneDraw->Clear();
				sceneDraw.reset();

				shadowMap->Clear();
				shadowMap.reset();

				frustumHelper->Clear();
				frustumHelper.reset();

				hdOccHelper->Clear();
				hdOccHelper.reset();

				hzbOccHelper->Clear();
				hzbOccHelper.reset();

				currFrameResource = nullptr;
				currFrameResourceIndex = 0;

				for (int i = 0; i < Constants::gNumFrameResources; ++i)
					frameResources[i] = nullptr;

				graphicResourceM->Clear();
				graphicResourceM.reset();

				cullingM->Clear();
				cullingM.reset();

				csmM->Clear();
				csmM.reset();

				device->EndPublicCommand();
				device->FlushCommandQueue();

				device->Clear();
				device.reset();

				info.width = 0;
				info.height = 0;

				RemoveListener(*JWindow::EvInterface(), guid);
			}
		private:
			bool InitializeResource()
			{ 
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
				currFrameResource = frameResources[currFrameResourceIndex].get();
				sceneDraw->Initialize(device.get(), graphicResourceM.get(), info);
				shadowMap->Initialize(device.get(), graphicResourceM.get(), info);
				frustumHelper->Initialize();
				hzbOccHelper->Initialize(device.get(), graphicResourceM.get(), info);
				hdOccHelper->Initialize(device.get(), graphicResourceM.get(), info);
				depthMapDebug->Initialize(device.get(), graphicResourceM.get(), info);
				depthTest->Initialize(device.get(), graphicResourceM.get(), info);
				outlineHelper->Initialize(device.get(), graphicResourceM.get(), info);
				device->EndPublicCommand();
				device->FlushCommandQueue(); 
				return res;
			}      
		public:		
			void LoadData()
			{
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicOption.txt");
				std::wifstream stream;
				stream.open(path, std::ios::binary | std::ios::in);
				if (!stream.is_open())
					return;

				std::wstring guide;
				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, info.upObjCount);
				JFileIOHelper::LoadAtomicData(stream, info.upAniCount);
				JFileIOHelper::LoadAtomicData(stream, info.upEnginePassCount);
				JFileIOHelper::LoadAtomicData(stream, info.upScenePassCount);
				JFileIOHelper::LoadAtomicData(stream, info.upCameraCount);
				JFileIOHelper::LoadAtomicData(stream, info.updLightCount);
				JFileIOHelper::LoadAtomicData(stream, info.uppLightCount);
				JFileIOHelper::LoadAtomicData(stream, info.upsLightCount);
				JFileIOHelper::LoadAtomicData(stream, info.upCsmCount);
				JFileIOHelper::LoadAtomicData(stream, info.upCubeShadowMapCount);
				JFileIOHelper::LoadAtomicData(stream, info.upNormalShadowMapCount);
				JFileIOHelper::LoadAtomicData(stream, info.upMaterialCount);
				JFileIOHelper::LoadAtomicData(stream, info.binding2DTextureCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingCubeMapCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingShadowTextureCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingShadowTextureArrayCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingShadowTextureCubeCapacity);

				JGraphicOption newOption;
				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, newOption.isOcclusionQueryActivated);
				JFileIOHelper::LoadAtomicData(stream, newOption.isHDOcclusionAcitvated);
				JFileIOHelper::LoadAtomicData(stream, newOption.isHZBOcclusionActivated);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowHZBCorrectFail);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowDebugOutline);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowMultiThread);
				stream.close();
				SetOption(newOption);
			}
			void StoreData()
			{
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicOption.txt");
				std::wofstream stream;
				stream.open(path, std::ios::binary | std::ios::out);
				if (!stream.is_open())
					return;

				JFileIOHelper::StoreJString(stream, L"--Info--", L"");
				JFileIOHelper::StoreAtomicData(stream, L"UploadObjCount:", info.upObjCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadAniCount:", info.upAniCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadEnginePassCount:", info.upEnginePassCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadScenePassCount:", info.upScenePassCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadCameraCount:", info.upCameraCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadDirectionalLightCount:", info.updLightCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadPointLightCount:", info.uppLightCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadSpotLightCount:", info.upsLightCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadCsmCount:", info.upCsmCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadCubeShadowMapCount:", info.upCubeShadowMapCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadNormalShadowMapCount:", info.upNormalShadowMapCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadMaterialCount:", info.upMaterialCount);
				JFileIOHelper::StoreAtomicData(stream, L"Bind2DTextureCount:", info.binding2DTextureCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindCubeMapCount:", info.bindingCubeMapCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindShadowTextureCount:", info.bindingShadowTextureCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindShadowTextureArrayCount:", info.bindingShadowTextureArrayCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindShadowTextureCubeCount:", info.bindingShadowTextureCubeCapacity);

				JFileIOHelper::StoreJString(stream, L"--Option--", L"");
				JFileIOHelper::StoreAtomicData(stream, L"AllowOcclusionQuery:", option.isOcclusionQueryActivated);
				JFileIOHelper::StoreAtomicData(stream, L"HardwareOcclusionAcitvated:", option.isHDOcclusionAcitvated);
				JFileIOHelper::StoreAtomicData(stream, L"HZBOcclusionAcitvated:", option.isHZBOcclusionActivated);
				JFileIOHelper::StoreAtomicData(stream, L"AllowHZBCorrectFail:", option.allowHZBCorrectFail);
				JFileIOHelper::StoreAtomicData(stream, L"AllowOutline:", option.allowDebugOutline);
				JFileIOHelper::StoreAtomicData(stream, L"AllowMultiThread:", option.allowMultiThread);
				stream.close();
			}
			void WriteLastRsTexture()
			{ 
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				{
					// 0 is main camera
			 
					JApplicationProjectInfo* opendInfo = JApplicationProject::GetOpenProjectInfo();
					if (opendInfo == nullptr)
						MessageBox(0, L"get open proejct info error", 0, 0);

					graphicResourceM->StoreTexture(device.get(), J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0, opendInfo->lastRsPath());
				}
			}
		public:
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
			{
				if (senderGuid == guid)
					return;

				if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
					OnResize();
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

		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateSceneDepthStencilResource()
		{
			return JinEngine::JGraphic::Instance().impl->CreateSceneDepthStencilResource();
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateDebugDepthStencilResource()
		{
			return JinEngine::JGraphic::Instance().impl->CreateDebugDepthStencilResource();
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateLayerDepthDebugResource(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateLayerDepthDebugResource(textureWidth, textureHeight);
		}
		void ResourceInterface::CreateHZBOcclusionResource(_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo, _Out_ JUserPtr<JGraphicResourceInfo>& outOccMipMapInfo)
		{
			JinEngine::JGraphic::Instance().impl->CreateHZBOcclusionResource(outOccDsInfo, outOccMipMapInfo);
		} 
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateOcclusionResourceDebug(const bool isHzb)
		{
			return JinEngine::JGraphic::Instance().impl->CreateOcclusionResourceDebug(isHzb);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::Create2DTexture(const std::wstring& path, const std::wstring& oriFormat)
		{
			return JinEngine::JGraphic::Instance().impl->Create2DTexture(path, oriFormat);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateCubeMap(const std::wstring& path, const std::wstring& oriFormat)
		{
			return JinEngine::JGraphic::Instance().impl->CreateCubeMap(path, oriFormat);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateRenderTargetTexture(textureWidth, textureHeight);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateShadowMapTexture(textureWidth, textureHeight);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateShadowMapArrayTexture(uint textureWidth, uint textureHeight, const uint count)
		{
			return JinEngine::JGraphic::Instance().impl->CreateShadowMapArrayTexture(textureWidth, textureHeight, count);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateShadowMapCubeTexture(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateShadowMapCubeTexture(textureWidth, textureHeight);
		} 
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateVertexBuffer(const std::vector<JStaticMeshVertex>& vertex)
		{
			return JinEngine::JGraphic::Instance().impl->CreateVertexBuffer(vertex);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateVertexBuffer(const std::vector<JSkinnedMeshVertex>& vertex)
		{
			return JinEngine::JGraphic::Instance().impl->CreateVertexBuffer(vertex);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateIndexBuffer(std::vector<uint32>& index)
		{
			return JinEngine::JGraphic::Instance().impl->CreateIndexBuffer(index);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateIndexBuffer(std::vector<uint16>& index)
		{
			return JinEngine::JGraphic::Instance().impl->CreateIndexBuffer(index);
		}
		bool ResourceInterface::DestroyGraphicTextureResource(JGraphicResourceInfo* info)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyGraphicTextureResource(info);
		}
		JOwnerPtr<JGraphicShaderDataHolderBase> ResourceInterface::StuffGraphicShaderPso(const JGraphicShaderInitData& shaderData)
		{
			return JinEngine::JGraphic::Instance().impl->StuffGraphicShaderPso(shaderData);
		}
		JOwnerPtr<JComputeShaderDataHolderBase> ResourceInterface::StuffComputeShaderPso(const JComputeShaderInitData& shaderData)
		{
			return JinEngine::JGraphic::Instance().impl->StuffComputeShaderPso(shaderData);
		}

		JUserPtr<JCullingInfo> CullingInterface::CreateFrsutumCullingResultBuffer()
		{
			return JinEngine::JGraphic::Instance().impl->CreateFrsutumCullingResultBuffer();
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