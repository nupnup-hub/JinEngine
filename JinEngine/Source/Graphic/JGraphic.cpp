#pragma once
#include"JGraphic.h" 
#include"JGraphicPrivate.h" 
#include"JGraphicConstants.h"
#include"JGraphicDrawList.h" 
#include"JGraphicUpdateHelper.h"
#include"GraphicResource/JGraphicResourceManager.h"
#include"GraphicResource/JGraphicResourceInfo.h"
#include"GraphicResource/JGraphicResourceInterface.h"
#include"GraphicResource/JGraphicResourceShareData.h"
#include"Device/JGraphicDevice.h"
#include"DataSet/JGraphicSubclassDataSet.h"
#include"Scene/JSceneDraw.h"
#include"ShadowMap/JShadowMap.h"
#include"ShadowMap/JCsmManager.h"
#include"Debug/JGraphicDebug.h"
#include"DepthMap/JDepthTest.h" 
#include"Buffer/JGraphicBuffer.h"  
#include"Image/JBlur.h"
#include"Image/JDownSampling.h"
#include"Image/JSsao.h"
#include"Image/JToneMapping.h"
#include"Image/JBloom.h"
#include"Image/JAntialise.h"
#include"Image/JConvertColor.h"
#include"Image/JPostProcessExposure.h"
#include"Image/JPostProcessHistogram.h"
#include"Image/JPostProcessPipeline.h"
#include"Culling/JCullingInfo.h"
#include"Culling/JCullingManager.h"
#include"Culling/Frustum/JFrustumCulling.h"
#include"Culling/Occlusion/JHardwareOccCulling.h"
#include"Culling/Occlusion/JHZBOccCulling.h"
#include"Culling/Light/JLightCulling.h"
#include"Accelerator/JGpuAcceleratorManager.h"
#include"Accelerator/JGpuAcceleratorInfo.h"
#include"Command/JCommandContext.h"
#include"Outline/JOutline.h"
#include"Raytracing/Light/Global/JRaytracingGI.h"
#include"Raytracing/Occlusion/JRaytracingAmbientOcclusion.h" 
#include"Raytracing/Denoiser/JRaytracingDenoiser.h" 

#include"FrameResource/JFrameUpdate.h" 
#include"FrameResource/JFrameResource.h" 
#include"FrameResource/JObjectConstants.h" 
#include"FrameResource/JAnimationConstants.h" 
#include"FrameResource/JMaterialConstants.h" 
#include"FrameResource/JCameraConstants.h" 
#include"FrameResource/JLightConstants.h"  
#include"FrameResource/JOcclusionConstants.h"
#include"FrameResource/JRaytracingConstants.h"
#include"FrameResource/JFrameIndexAccess.h"

#include"Gui/JGuiBackendInterface.h"
#include"Gui/JGuiBackendDataAdapter.h"
#include"Gui/JGuiBackendDataAdaptee.h"
#include"Adapter/JGraphicAdapter.h"

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
#include"../Core/Log/JLogMacro.h" 

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

#include"../Application/Engine/JApplicationEngine.h"
#include"../Application/Engine/JApplicationEnginePrivate.h"
#include"../Application/Project/JApplicationProject.h"  

#ifdef DEVELOP
#include"../Develop/Debug/JDevelopDebug.h"
#endif

namespace JinEngine
{
	using namespace DirectX;
	namespace Graphic
	{
		//기능을 추가할때 코드 수정 및 추가를 최소화 하기위해 되도록 직접적인 data를 전달하기보다는
		//구조체를 이용하며 직접으로 구현에 대해 호출하기 보다는 인터페이스를 통해서 호출하도록한다.
		//+ 데이터 생성 삭제를 하나에 클래스에서 관리하도록한다.(manager)
		//data set, subclass interface, adater 등

		//새로운 기능 추가 시나리오
		//Interface, implement(has graphic api dependency) class 작성
		//Subclass Set에 추가 및 Initialize, Clear등 .class에대 호출코드 작성
		//Adaptee에서 생성 및 Draw sequence중에 호출
		void JResourceManageSubclassSet::Initialize(JGraphicDevice* device, const JGraphicInfo& info)
		{
			graphic->Initialize(device);
			device->CreateRefResourceObject(JGraphicDeviceInitSet(graphic.get()));

			for (uint i = 0; i < Constants::gNumFrameResources; ++i)
				frame[i]->Intialize(device);
			currFrame = frame[info.frame.currIndex].get();
		}
		void JResourceManageSubclassSet::Clear()
		{
			currFrame = nullptr;
			for (int i = 0; i < Constants::gNumFrameResources; ++i)
				frame[i] = nullptr;

			shareData = nullptr;
			accelerator = nullptr;
			csm = nullptr;
			culling = nullptr;
			graphic = nullptr;
			context = nullptr;
		}
		void JResourceManageSubclassSet::GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV)
		{
			outV.push_back(graphic.get());
			outV.push_back(culling.get());
			outV.push_back(csm.get());
			outV.push_back(accelerator.get());
			outV.push_back(shareData.get());
			for (int i = 0; i < Constants::gNumFrameResources; ++i)
				outV.push_back(frame[i].get());
		}
		void JDrawingSubclassSet::Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage)
		{
			scene->Initialize(device, resourceManage->graphic.get());
			shadowMap->Initialize(device, resourceManage->graphic.get());
			depthTest->Initialize(device, resourceManage->graphic.get());
		}
		void JDrawingSubclassSet::Clear()
		{
			scene = nullptr;
			shadowMap = nullptr;
			depthTest = nullptr;
		}
		void JDrawingSubclassSet::GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV)
		{
			outV.push_back(scene.get());
			outV.push_back(shadowMap.get());
			outV.push_back(depthTest.get());
		}
		void JCullingSubclassSet::Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage)
		{
			frustum->Initialize();
			hd->Initialize(device, resourceManage->graphic.get());
			hzb->Initialize(device, resourceManage->graphic.get());
			lit->Initialize(device, resourceManage->graphic.get());
		}
		void JCullingSubclassSet::Clear()
		{
			lit = nullptr;
			hzb = nullptr;
			hd = nullptr;
			frustum = nullptr;
		}
		void JCullingSubclassSet::GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV)
		{
			outV.push_back(lit.get());
			outV.push_back(hzb.get());
			outV.push_back(hd.get());
			outV.push_back(frustum.get());
		}
		void JImageProcessingSubclassSet::Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage)
		{
			debug->Initialize(device, resourceManage->graphic.get());
			outline->Initialize(device, resourceManage->graphic.get());
			blur->Initialize(device, resourceManage->graphic.get());
			downSampling->Initialize(device, resourceManage->graphic.get());
			ssao->Initialize(device, resourceManage->graphic.get());
			tm->Initialize(device, resourceManage->graphic.get());
			bloom->Initialize(device, resourceManage->graphic.get());
			aa->Initialize(device, resourceManage->graphic.get());
			histogram->Initialize(device, resourceManage->graphic.get());
			exposure->Initialize(device, resourceManage->graphic.get());
			convertColor->Initialize(device, resourceManage->graphic.get());
		}
		void JImageProcessingSubclassSet::Clear()
		{
			ppPipeline = nullptr;
			ppEffectSet = nullptr;
			convertColor = nullptr;
			exposure = nullptr;
			histogram = nullptr;
			aa = nullptr;
			bloom = nullptr;
			tm = nullptr;
			ssao = nullptr;
			downSampling = nullptr;
			blur = nullptr;
			outline = nullptr;
			debug = nullptr;
		}
		void JImageProcessingSubclassSet::GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV)
		{
			outV.push_back(debug.get());
			outV.push_back(outline.get());
			outV.push_back(blur.get());
			outV.push_back(downSampling.get());
			outV.push_back(ssao.get());
			outV.push_back(tm.get());
			outV.push_back(bloom.get());
			outV.push_back(aa.get());
			outV.push_back(histogram.get());
			outV.push_back(exposure.get());
			outV.push_back(convertColor.get());
		}
		void JRaytracingSubclassSet::Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage)
		{
			gi->Initialize(device, resourceManage->graphic.get());
			denoiser->Initialize(device, resourceManage->graphic.get());
			//unuse
			//ao->Initialize(device, resourceManage->graphic.get());
		}
		void JRaytracingSubclassSet::Clear()
		{
			gi = nullptr;
			ao = nullptr;
			denoiser = nullptr;
		}
		void JRaytracingSubclassSet::GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV)
		{
			outV.push_back(gi.get());
			//outV.push_back(ao.get()); 
			outV.push_back(denoiser.get());
		}

		class JConstantCache
		{
		public:
			JScenePassConstants scenePass;
			JAnimationConstants ani;
		public:
			JObjectConstantsSet objSet;
			JCameraConstantsSet camSet;
			JLightConstantsSet litSet;
		public:
			//used by pass
			//initialize first update constants buffer after initialize graphic class 
			//always exist until enigne end
			JUserPtr<JTexture> missing;
			JUserPtr<JTexture> bluseNoise;
		public:
			void Clear()
			{
				scenePass = JScenePassConstants();
				ani = JAnimationConstants();
			}
		};

		template<size_t ...Is>
		static void StuffGetElementLam(std::unordered_map<J_UPLOAD_FRAME_RESOURCE_TYPE, JUpdateHelper::GetElementCountT::Ptr>& uGetCountFunc,
			std::index_sequence<Is...>)
		{
			using Type = J_UPLOAD_FRAME_RESOURCE_TYPE;
			((uGetCountFunc.emplace((Type)Is, []() {return JFrameUpdateData::GetTotalFrameCount((Type)Is); })), ...);
		}

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
			template<uint count>
			static void SwitchBoolValue(bool* newPtr[count], bool* oldPtr[count])
			{
				if constexpr (count == 0)
					return;

				uint trueCount = 0;
				for (uint i = 0; i < count; ++i)
					trueCount += *newPtr[i];
				if (trueCount > 1)
				{
					for (uint i = 0; i < count; ++i)
					{
						if (!(*oldPtr[i]) && (*newPtr[i]))
						{
							uint mask = i;
							for (uint j = 0; j < count; ++j)
							{
								if (j != mask)
									*newPtr[j] = false;
							}
							break;
						}
					}
				}
				else
				{
					int newTrueIndex = invalidIndex;
					for (uint i = 0; i < count; ++i)
					{
						if (*oldPtr[i])
						{
							newTrueIndex = i;
							break;
						}
					}
					if (newTrueIndex == invalidIndex)
						newTrueIndex = 0;
					*newPtr[newTrueIndex] = true;
				}
			}
		}
#pragma region Impl
		class JGraphic::JGraphicImpl : public WindowEventListener, public GraphicEventManager
		{
		private:
			using WorkerThreadF = Core::JMFunctorType<JGraphicImpl, void, uint>;
			using InnerEventF = Core::JSFunctorType<void, JGraphicImpl*>;
		public:
			JGraphic* thisGraphic;
		public:
			const size_t guid;
		public:
			JGraphicInfo info;
			JGraphicOption option;
			JUpdateHelper updateHelper;
			//전체 opaque object 만큼 할당된 object vec
			//hard ware occlusion이나 object align이 필요할때 결과를 담을 vector로써 사용된다.
			JGameObjectBuffer alignedObject;
			JConstantCache contCache;
			JFrameIndexAccess frameAccess;
		private:
			std::unique_ptr<JGraphicAdapter> adapter;
			std::unique_ptr<JGraphicDevice> device;
		private:
			JResourceManageSubclassSet resourceManage;
			JDrawingSubclassSet drawing;
			JCullingSubclassSet culling;
			JImageProcessingSubclassSet imageProcessing;
			JRaytracingSubclassSet raytracing;
		private:
			std::unique_ptr<JGraphicDrawReferenceSet> drawRefSet;
			std::unique_ptr<WorkerThreadF::Functor> workerFunctor;
		private:
			JGuiBackendInterface* guiBackendInterface;
			//graphic api data
			std::unique_ptr<JGuiBackendDataAdapter> guiAdapter;
		private:
			std::vector<JGraphicSubClassInterface*> infoChangedListener[(uint)JGraphicInfo::TYPE::COUNT];
			std::vector<JGraphicSubClassInterface*> optionChangedListener[(uint)JGraphicOption::TYPE::COUNT];
			//std::vector<std::unique_ptr<InnerEventF::CompletelyBind>> innerEvent;
			std::vector<std::unique_ptr<Core::JBindHandleBase>> innerEvent;
		private:
			bool canDraw = true;
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
				const uint occMipmapViewCapa = Constants::occlusionMipmapViewCapacity;
				info.resource.occlusionWidth = std::pow(2, occMipmapViewCapa - 1);
				info.resource.occlusionHeight = std::pow(2, occMipmapViewCapa - 1);
				info.resource.occlusionMinSize = Constants::minOcclusionSize;
				info.resource.occlusionMapCapacity = occMipmapViewCapa;
				info.resource.occlusionMapCount = JMathHelper::Log2Int(info.resource.occlusionWidth) - JMathHelper::Log2Int(Constants::minOcclusionSize) + 1;
				info.frame.threadCount = _JThreadManager::Instance().GetReservedSpaceCount(Core::J_THREAD_USE_CASE_TYPE::GRAPHIC_DRAW);
			}
			//CallOnece
			void InitializeGameObjectBuffer()
			{
				alignedObject.common.resize(info.minCapacity);
			}
			//CallOnece
			void RegisterResouceNotifyFunc()
			{
				using GetElementCount = JUpdateHelper::GetElementCountT::Ptr;
				std::unordered_map<J_UPLOAD_FRAME_RESOURCE_TYPE, GetElementCount> uGetCountFunc;
				StuffGetElementLam(uGetCountFunc, std::make_index_sequence<(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT>());

				for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				{
					J_UPLOAD_FRAME_RESOURCE_TYPE type = (J_UPLOAD_FRAME_RESOURCE_TYPE)i;
					auto data = uGetCountFunc.find(type);
					if (data == uGetCountFunc.end())
						continue;

					updateHelper.RegisterCallable(type, uGetCountFunc.find(type)->second);
				}
				//updateHelper.RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, std::make_unique<NotifyUpdateCapacity>(updateHdOccResultCapaLam));

				auto texture2DGetCountLam = []() {return _JGraphic::Instance().impl->resourceManage.graphic->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D); };
				auto cubeMapGetCountLam = []() {return _JGraphic::Instance().impl->resourceManage.graphic->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE); };
				auto shadowMapGetCountLam = []() {return _JGraphic::Instance().impl->resourceManage.graphic->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP); };
				auto shadowMapArrayGetCountLam = []() {return _JGraphic::Instance().impl->resourceManage.graphic->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY); };
				auto shadowMapCubeGetCountLam = []() {return _JGraphic::Instance().impl->resourceManage.graphic->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE); };

				auto texture2DGetCapacityLam = []() {return _JGraphic::Instance().impl->info.resource.binding2DTextureCapacity; };
				auto cubeMapGetCapacityLam = []() {return _JGraphic::Instance().impl->info.resource.bindingCubeMapCapacity; };
				auto shadowMapGetCapacityLam = []() {return _JGraphic::Instance().impl->info.resource.bindingShadowTextureCapacity; };
				auto shadowMapArrayGetCapacityLam = []() {return _JGraphic::Instance().impl->info.resource.bindingShadowTextureArrayCapacity; };
				auto shadowMapCubeGetCapacityLam = []() {return _JGraphic::Instance().impl->info.resource.bindingShadowTextureCubeCapacity; };

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
			std::vector<JCommandContextLog> GetCpuDrawingLog()const noexcept
			{
				return resourceManage.context != nullptr ? resourceManage.context->GetLog() : std::vector<JCommandContextLog>();
			}
			JGraphicResourceManager* GetGraphicResourceManager()const noexcept
			{
				return resourceManage.graphic.get();
			}
			std::unique_ptr<JGuiInitData> GetGuiInitData()const noexcept
			{
				return guiAdapter->CreateInitData(device.get(),
					resourceManage.graphic.get(),
					option,
					guiBackendInterface->GetGuiIdentification());
			}
		public:
			void SetGraphicInfo(const JGraphicInfo& newInfo, bool isFrameDirty, bool isResourceDirty)
			{
				JGraphicInfo preInfo = info;
				info = newInfo;
				auto notifySet = adapter->CreateInfoChangedSet(option.deviceType, preInfo, *drawRefSet);
				if (isResourceDirty)
				{
					notifySet->changedPart = JGraphicInfo::TYPE::RESOURCE;
					for (const auto& data : infoChangedListener[(uint)JGraphicInfo::TYPE::RESOURCE])
						data->NotifyGraphicInfoChanged(*notifySet);
				}

				if (isFrameDirty)
				{
					notifySet->changedPart = JGraphicInfo::TYPE::FRAME;
					for (const auto& data : infoChangedListener[(uint)JGraphicInfo::TYPE::FRAME])
						data->NotifyGraphicInfoChanged(*notifySet);
				}

				if (alignedObject.common.size() != info.frame.upBoundingObjCapacity)
					alignedObject.common.resize(info.frame.upBoundingObjCapacity);
			}
			void SetOption(JGraphicOption newGraphicOption)noexcept
			{
				bool changedMask[(uint)JGraphicOption::TYPE::COUNT];
				memset(&changedMask, false, sizeof(bool) * SIZE_OF_ARRAY(changedMask));

				newGraphicOption.culling.clusterXIndex = std::clamp(newGraphicOption.culling.clusterXIndex, (uint)0, Constants::litClusterXVariation - 1);
				newGraphicOption.culling.clusterYIndex = std::clamp(newGraphicOption.culling.clusterYIndex, (uint)0, Constants::litClusterYVariation - 1);
				newGraphicOption.culling.clusterZIndex = std::clamp(newGraphicOption.culling.clusterZIndex, (uint)0, Constants::litClusterZVariation - 1);
				newGraphicOption.culling.lightPerClusterIndex = std::clamp(newGraphicOption.culling.lightPerClusterIndex, (uint)0, Constants::lightPerClusterVariation - 1);
				newGraphicOption.culling.clusterNear = std::clamp(newGraphicOption.culling.clusterNear, Constants::litClusterNearMin, Constants::litClusterNearMax);
				newGraphicOption.culling.clusterPointLightRangeOffset = std::clamp(newGraphicOption.culling.clusterPointLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);
				newGraphicOption.culling.clusterSpotLightRangeOffset = std::clamp(newGraphicOption.culling.clusterSpotLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);
				newGraphicOption.culling.clusterRectLightRangeOffset = std::clamp(newGraphicOption.culling.clusterRectLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);

				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.allowMultiThread != newGraphicOption.rendering.allowMultiThread);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.allowDeferred != newGraphicOption.rendering.allowDeferred);
				if (newGraphicOption.rendering.allowRaytracing)
					newGraphicOption.rendering.allowRaytracing &= device->IsRaytracingSupported();
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.allowRaytracing != newGraphicOption.rendering.allowRaytracing);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useMSAA != newGraphicOption.rendering.useMSAA);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.renderTargetFormat != newGraphicOption.rendering.renderTargetFormat);

				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useGGXMicrofacet != newGraphicOption.rendering.useGGXMicrofacet);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useBeckmannMicrofacet != newGraphicOption.rendering.useBeckmannMicrofacet);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useBlinnPhongMicrofacet != newGraphicOption.rendering.useBlinnPhongMicrofacet);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useIsotropy != newGraphicOption.rendering.useIsotropy);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useDisneyDiffuse != newGraphicOption.rendering.useDisneyDiffuse);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useFrostBiteDiffuse != newGraphicOption.rendering.useFrostBiteDiffuse);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useHammonDiffuse != newGraphicOption.rendering.useHammonDiffuse);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useOrenNayarDiffuse != newGraphicOption.rendering.useOrenNayarDiffuse);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useShirleyDiffuse != newGraphicOption.rendering.useShirleyDiffuse);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.useLambertianDiffuse != newGraphicOption.rendering.useLambertianDiffuse);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.restir != newGraphicOption.rendering.restir);

				changedMask[(uint)JGraphicOption::TYPE::SHAODW] |= (option.shadow.useHighQualityShadow != newGraphicOption.shadow.useHighQualityShadow);
				changedMask[(uint)JGraphicOption::TYPE::SHAODW] |= (option.shadow.useMiddleQualityShadow != newGraphicOption.shadow.useMiddleQualityShadow);
				changedMask[(uint)JGraphicOption::TYPE::SHAODW] |= (option.shadow.useLowQualityShadow != newGraphicOption.shadow.useLowQualityShadow);

				//cluster
				bool isClusterXIndexChanged = false;
				bool isClusterYIndexChanged = false;
				isClusterXIndexChanged = (option.culling.clusterXIndex != newGraphicOption.culling.clusterXIndex);
				isClusterYIndexChanged = (option.culling.clusterYIndex != newGraphicOption.culling.clusterYIndex);

				//x, y는 항상 2:1 resolution을 유지하기위해 각자 다른 배열에 같은 index위치에
				//2:1 비율이 되는 값을 할당한다.
				if (isClusterXIndexChanged)
					newGraphicOption.culling.clusterYIndex = newGraphicOption.culling.clusterXIndex;
				if (isClusterYIndexChanged)
					newGraphicOption.culling.clusterXIndex = newGraphicOption.culling.clusterYIndex;

				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.isLightCullingActivated != newGraphicOption.culling.isLightCullingActivated);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.allowLightCluster != newGraphicOption.culling.allowLightCluster);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (isClusterXIndexChanged || isClusterYIndexChanged);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.clusterZIndex != newGraphicOption.culling.clusterZIndex);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.lightPerClusterIndex != newGraphicOption.culling.lightPerClusterIndex);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.clusterNear != newGraphicOption.culling.clusterNear);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.clusterPointLightRangeOffset != newGraphicOption.culling.clusterPointLightRangeOffset);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.clusterSpotLightRangeOffset != newGraphicOption.culling.clusterSpotLightRangeOffset);
				changedMask[(uint)JGraphicOption::TYPE::CULLING] |= (option.culling.clusterRectLightRangeOffset != newGraphicOption.culling.clusterRectLightRangeOffset);

				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.usePostprocess != newGraphicOption.postProcess.usePostprocess);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useSsao != newGraphicOption.postProcess.useSsao);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useSsaoInterleave != newGraphicOption.postProcess.useSsaoInterleave);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useHdr != newGraphicOption.postProcess.useHdr);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useFxaa != newGraphicOption.postProcess.useFxaa);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useToneMapping != newGraphicOption.postProcess.useToneMapping);

#ifdef DEVELOP
				//debugging`
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileGraphicShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileLightClusterShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileSsaoShader; ; ;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileToneMappingShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileRtGiShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileRtDenoiseShader;
				//dependencyOption[RECOMPILE_SSAO] |= (option.drawSsaoByComputeShader != newGraphicOption.drawSsaoByComputeShader);
#endif 
				static constexpr uint shadowSwitchCount = 3;
				bool* newShadowSwitch[shadowSwitchCount]
				{
					&newGraphicOption.shadow.useHighQualityShadow,
					&newGraphicOption.shadow.useMiddleQualityShadow,
					&newGraphicOption.shadow.useLowQualityShadow
				};
				bool* oldShadowSwitch[shadowSwitchCount]
				{
					&option.shadow.useHighQualityShadow,
					&option.shadow.useMiddleQualityShadow,
					&option.shadow.useLowQualityShadow
				};
				Private::SwitchBoolValue<shadowSwitchCount>(newShadowSwitch, oldShadowSwitch);

				static constexpr uint bxdfMicrofacetCount = 3;
				bool* newBxdfMicrofacetSwitch[bxdfMicrofacetCount]
				{
					&newGraphicOption.rendering.useGGXMicrofacet,
					&newGraphicOption.rendering.useBeckmannMicrofacet,
					&newGraphicOption.rendering.useBlinnPhongMicrofacet
				};
				bool* oldBxdfMicrofacetSwitch[bxdfMicrofacetCount]
				{
					&option.rendering.useGGXMicrofacet,
					&option.rendering.useBeckmannMicrofacet,
					&option.rendering.useBlinnPhongMicrofacet
				};
				Private::SwitchBoolValue<bxdfMicrofacetCount>(newBxdfMicrofacetSwitch, oldBxdfMicrofacetSwitch);

				static constexpr uint bxdfDiffuseCount = 6;
				bool* newBxdfDiffuseSwitch[bxdfDiffuseCount]
				{
					&newGraphicOption.rendering.useDisneyDiffuse,
					&newGraphicOption.rendering.useFrostBiteDiffuse,
					&newGraphicOption.rendering.useHammonDiffuse,
					&newGraphicOption.rendering.useOrenNayarDiffuse,
					&newGraphicOption.rendering.useShirleyDiffuse,
					&newGraphicOption.rendering.useLambertianDiffuse
				};
				bool* oldBxdfDiffuseSwitch[bxdfDiffuseCount]
				{
					&option.rendering.useDisneyDiffuse,
					&option.rendering.useFrostBiteDiffuse,
					&option.rendering.useHammonDiffuse,
					&option.rendering.useOrenNayarDiffuse,
					&option.rendering.useShirleyDiffuse,
					&option.rendering.useLambertianDiffuse
				};
				Private::SwitchBoolValue<bxdfDiffuseCount>(newBxdfDiffuseSwitch, oldBxdfDiffuseSwitch);

				JGraphicOption preOption = option;
				option = newGraphicOption;

				const bool anyMasked = JCUtil::IsAnySame(true, changedMask, std::make_index_sequence<(uint)JGraphicOption::TYPE::COUNT>());
				if (device != nullptr && anyMasked)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();

					auto notifySet = adapter->CreateOptionChangedSet(option.deviceType, preOption, *drawRefSet);
					for (uint i = 0; i < SIZE_OF_ARRAY(changedMask); ++i)
					{
						if (!changedMask[i])
							continue;

						notifySet->changedPart = (JGraphicOption::TYPE)i;
						for (const auto& data : optionChangedListener[i])
							data->NotifyGraphicOptionChanged(*notifySet);
					}

					if (preOption.rendering.renderTargetFormat != newGraphicOption.rendering.renderTargetFormat)
					{
						auto hdrChangeLam = [](JGraphicImpl* impl)
						{
							impl->device->FlushCommandQueue();
							impl->device->StartPublicCommand();
							impl->device->NotifyChangedBackBufferFormat(JGraphicDeviceInitSet(impl->resourceManage.graphic.get()));
							impl->guiBackendInterface->ReBuildGraphicBackend(impl->GetGuiInitData());
							impl->device->EndPublicCommand();
							impl->device->FlushCommandQueue();
						};
						AddInnerEvent(Core::UniqueBind(std::make_unique<InnerEventF::Functor>(hdrChangeLam), this));
					}
					NotifyEvent<J_GRAPHIC_EVENT_TYPE::OPTION_CHANGED>(guid, preOption, option);
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}
#ifdef DEVELOP
				option.debugging.requestRecompileGraphicShader =
					option.debugging.requestRecompileLightClusterShader =
					option.debugging.requestRecompileSsaoShader =
					option.debugging.requestRecompileToneMappingShader =
					option.debugging.requestRecompileRtGiShader =
					option.debugging.requestRecompileRtDenoiseShader = false;
#endif
			}
			bool SetTextureDetail(const JUserPtr<JGraphicResourceInfo>& srcInfo, const JConvertColorDesc& convertDesc)
			{
				if (srcInfo == nullptr)
					return false;

				JUserPtr<JGraphicResourceInfo> intermediate00 = nullptr;
				auto rType = srcInfo->GetGraphicResourceType();
				if (rType != J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D)
					return false;

				auto resourceSize = srcInfo->GetResourceSize();
				//추가로 custom mipmap이 필요한 J_GRAPHIC_RESOURCE_TYPE이 있을경우 수정필요.				 
				JGraphicResourceCreationDesc cDesc;
				cDesc.width = resourceSize.x / 4;
				cDesc.height = resourceSize.y / 4;
				cDesc.textureDesc = std::make_unique<JTextureCreationDesc>();
				cDesc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT;
				cDesc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				cDesc.bindDesc.useEngineDefinedBindType = false;
				cDesc.formatHint = std::make_unique<JGraphicFormatHint>();
				cDesc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32G32B32A32_FLOAT;
				intermediate00 = resourceManage.graphic->CreateResource(device.get(), cDesc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

				JGraphicConvetColorSettingSet convertSet(srcInfo.Get(), intermediate00.Get(), convertDesc);
				if (!adapter->BeginConvertColorTask(option.deviceType, *drawRefSet, convertSet))
				{
					adapter->EndConvertColorTask(option.deviceType, *drawRefSet);
					return false;
				}

				imageProcessing.convertColor->ApplyConvertColor(convertSet.dataSet.get());
				adapter->EndConvertColorTask(option.deviceType, *drawRefSet);

				device->FlushCommandQueue();
				device->StartPublicCommand();
				resourceManage.graphic->CopyResource(device.get(), intermediate00, srcInfo);
				device->EndPublicCommand();
				device->FlushCommandQueue();

				resourceManage.graphic->DestroyGraphicTextureResource(device.get(), intermediate00.Release());
				return true;
			}
		public:
			bool IsRaytracingSupported()const noexcept
			{
				return device->IsRaytracingSupported();
			}
			bool CanBuildGpuAccelerator()const noexcept
			{
				return device->CanBuildGpuAccelerator();
			}
		private:
			bool IsEntryUpdateLoop()
			{
				return JApplicationEngine::GetApplicationSubState() == J_APPLICATION_SUB_STATE::UPDATE_LOOP;
			}
		private:
			void AddInnerEvent(std::unique_ptr<Core::JBindHandleBase>&& b)
			{
				innerEvent.push_back(std::move(b));
			}
		public:
			JUserPtr<JGraphicResourceInfo> CreateResource(const JGraphicResourceCreationDesc& createDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				auto userPtr = resourceManage.graphic->CreateResource(device.get(), createDesc, rType);
				if (userPtr == nullptr)
					return userPtr;

				if (rType == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D || rType == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
				{
					if (!createDesc.textureDesc->UseMipmap())
						userPtr->SetMipmapType(J_GRAPHIC_MIP_MAP_TYPE::NONE);
					else if (userPtr != nullptr && createDesc.textureDesc->mipMapDesc.type != J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT)
					{
						if (!CreateCustomMipmap(userPtr, *createDesc.textureDesc))
							userPtr->SetMipmapType(J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT);
					}
				}
				resourceManage.shareData->NotifyGraphicResourceCreation(device.get(), resourceManage.graphic.get(), userPtr.Get());
				return userPtr;
			}
			bool CreateOption(JUserPtr<JGraphicResourceInfo>& gInfo, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)
			{
				return resourceManage.graphic->CreateOption(device.get(), gInfo, opType);
			}
			bool CreateCustomMipmap(const JUserPtr<JGraphicResourceInfo>& srcInfo, JTextureCreationDesc& createDesc)
			{
				if (srcInfo == nullptr || createDesc.mipMapDesc.type == J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT || createDesc.mipMapDesc.type == J_GRAPHIC_MIP_MAP_TYPE::NONE)
					return false;

				JUserPtr<JGraphicResourceInfo> intermediate00 = nullptr;
				JUserPtr<JGraphicResourceInfo> intermediate01 = nullptr;
				auto rType = srcInfo->GetGraphicResourceType();
				if (rType != J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D && rType != J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
					return false;

				auto resourceSize = srcInfo->GetResourceSize();
				//추가로 custom mipmap이 필요한 J_GRAPHIC_RESOURCE_TYPE이 있을경우 수정필요.				 
				JGraphicResourceCreationDesc cDesc(std::make_unique<JTextureCreationDesc>(createDesc));
				cDesc.width = resourceSize.x;
				cDesc.height = resourceSize.y;
				cDesc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				cDesc.bindDesc.useEngineDefinedBindType = false;
				cDesc.formatHint = std::make_unique<JGraphicFormatHint>();
				cDesc.formatHint->format = srcInfo->GetFormat();
				if (cDesc.formatHint->format == J_GRAPHIC_RESOURCE_FORMAT::API_SPECIALIZED)
					return false;

				intermediate00 = resourceManage.graphic->CreateResource(device.get(), cDesc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);
				intermediate01 = resourceManage.graphic->CreateResource(device.get(), cDesc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);
				resourceManage.graphic->CopyResource(device.get(), srcInfo, intermediate00);
				resourceManage.graphic->CopyResource(device.get(), srcInfo, intermediate01);

				std::vector<Core::JDataHandle> mipHandle00;
				std::vector<Core::JDataHandle> mipHandle01;

				if (intermediate00 == nullptr ||
					intermediate01 == nullptr ||
					!resourceManage.graphic->SettingMipmapBind(device.get(), intermediate00, false, mipHandle00) ||
					!resourceManage.graphic->SettingMipmapBind(device.get(), intermediate01, false, mipHandle01))
				{
					resourceManage.graphic->DestroyGraphicTextureResource(device.get(), intermediate00.Release());
					resourceManage.graphic->DestroyGraphicTextureResource(device.get(), intermediate01.Release());
					return false;
				}

				const JVector2F imageSize = JVector2F(srcInfo->GetWidth(), srcInfo->GetHeight());
				const uint mipCount = srcInfo->GetMipmapCount();
				JGraphicMipmapGenerationSettingSet mipmapSetting(mipHandle00, std::make_unique<JDownSampleDesc>(imageSize, mipCount));

				device->FlushCommandQueue();
				device->StartPublicCommand();
				if (adapter->BeginMipmapGenerationTask(option.deviceType, *drawRefSet, mipmapSetting))
				{
					const JMipmapGenerationDesc& mipmapDesc = createDesc.mipMapDesc;
					const JDrawHelper helper(info, option, alignedObject);

					imageProcessing.downSampling->ApplyMipmapGeneration(mipmapSetting.dataSet.get(), helper);
					adapter->EndMipmapGenerationTask(option.deviceType, *drawRefSet);

					std::unique_ptr<JBlurDesc> blurDesc;
					switch (mipmapDesc.type)
					{
					case JinEngine::Graphic::J_GRAPHIC_MIP_MAP_TYPE::BOX:
					{
						blurDesc = std::make_unique<JBoxBlurDesc>(imageSize * 0.5f, mipmapDesc.kernelSize);
						break;
					}
					case JinEngine::Graphic::J_GRAPHIC_MIP_MAP_TYPE::GAUSSIAN:
					{
						blurDesc = std::make_unique<JGaussianBlurDesc>(imageSize * 0.5f, mipmapDesc.kernelSize, mipmapDesc.sharpnessFactor);
						break;
					}
					case JinEngine::Graphic::J_GRAPHIC_MIP_MAP_TYPE::KAISER:
					{
						blurDesc = std::make_unique<JKaiserBlurDesc>(imageSize * 0.5f, mipmapDesc.kernelSize, mipmapDesc.sharpnessFactor);
						break;
					}
					default:
						break;
					}
					blurDesc->mipLevel = 1;
					blurDesc->blurCount = mipCount - 1;
					blurDesc->tryBlurSubResourcr = true;

					JGraphicBlurTaskSettingSet blurSetting(&mipHandle00[1], &mipHandle01[1], std::move(blurDesc));
					adapter->BeginBlurTask(option.deviceType, *drawRefSet, blurSetting);
					imageProcessing.blur->ApplyBlur(blurSetting.dataSet.get(), helper);
					adapter->EndBlurTask(option.deviceType, *drawRefSet);

					device->EndPublicCommand();
					device->FlushCommandQueue();

					device->FlushCommandQueue();
					device->StartPublicCommand();
					resourceManage.graphic->CopyResource(device.get(), intermediate01, srcInfo);
					srcInfo->SetMipmapType(mipmapDesc.type);
				}
				device->EndPublicCommand();
				device->FlushCommandQueue();

				resourceManage.graphic->DestroyGraphicTextureResource(device.get(), intermediate00.Release());
				resourceManage.graphic->DestroyGraphicTextureResource(device.get(), intermediate01.Release());
				ClearMipmapBind(mipHandle00);
				ClearMipmapBind(mipHandle01);
				return true;
			}
		public:
			bool DestroyGraphicTextureResource(JGraphicResourceInfo* gInfo)
			{
				if (gInfo == nullptr)
					return false;

				if (resourceManage.shareData != nullptr)
					resourceManage.shareData->NotifyGraphicResourceDestruction(device.get(), resourceManage.graphic.get(), gInfo);
				return resourceManage.graphic->DestroyGraphicTextureResource(device.get(), gInfo);
			}
			bool DestroyGraphicOption(JUserPtr<JGraphicResourceInfo>& gInfo, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)
			{
				if (gInfo == nullptr)
					return false;

				return resourceManage.graphic->DestroyGraphicOption(device.get(), gInfo, optype);
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

				if (!resourceManage.graphic->SettingMipmapBind(device.get(), gRInfo, true, dataHandle))
					return false;

				const uint count = dataHandle.size();
				gpuHandle.resize(count);
				for (uint i = 0; i < count; ++i)
					gpuHandle[i] = resourceManage.graphic->GetMPBResourceGpuHandle(dataHandle[i], J_GRAPHIC_BIND_TYPE::SRV);
				return true;
			}
			void ClearMipmapBind(_In_ std::vector<Core::JDataHandle>& dataHandle)
			{
				//mipmap은 순차적으로 할당되며 해제는 마지막에 할당된 handle부터 수행한다.(DataStructure valind index가 가장 앞에 index를 가리키도록...)
				const int count = (int)dataHandle.size();
				for (int i = count - 1; i >= 0; --i)
					resourceManage.graphic->DestroyMPB(device.get(), dataHandle[i]);
			}
		public:
			JUserPtr<JCullingInfo> CreateFrsutumCullingResultBuffer(const J_CULLING_TARGET target, const bool useGpu)
			{
				JCullingCreationDesc desc;
				if (target == J_CULLING_TARGET::RENDERITEM)
					desc.capacity = info.frame.upBoundingObjCapacity;
				else
					desc.capacity = info.frame.GetLocalLightCapacity();
				desc.target = target;
				//if graphic update전 초기화 단계일경우 update wait에 진입하자마자 info.frame.currIndex + 1이 되므로
				//미리  info.frame.currIndex + 1값을 할당한다.
				desc.currFrameIndex = IsEntryUpdateLoop() ? info.frame.currIndex : info.frame.currIndex + 1;
				desc.useGpu = useGpu;
				auto res = resourceManage.culling->CreateFrsutumData(device.get(), desc);
				if (res != nullptr)
					alignedObject.aligned.push_back(JGameObjectBuffer::OpaqueVec());
				return res;
			}
			JUserPtr<JCullingInfo> CreateHzbCullingResultBuffer()
			{
				JCullingCreationDesc desc;
				desc.capacity = info.frame.upHzbObjCapacity;
				desc.target = J_CULLING_TARGET::RENDERITEM;
				desc.currFrameIndex = IsEntryUpdateLoop() ? info.frame.currIndex : info.frame.currIndex + 1;
				desc.useGpu = true;

				auto user = resourceManage.culling->CreateHzbOcclusionData(device.get(), desc);
				if (user.IsValid())
				{
					if (device->CanStartPublicCommand())
					{
						device->FlushCommandQueue();
						device->StartPublicCommand();
						culling.hzb->NotifyBuildNewHzbOccBuffer(device.get(), info.frame.upHzbObjCapacity, user);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						culling.hzb->NotifyBuildNewHzbOccBuffer(device.get(), info.frame.upHzbObjCapacity, user);
				}
				return user;
			}
			JUserPtr<JCullingInfo> CreateHdCullingResultBuffer()
			{
				JCullingCreationDesc desc;
				desc.capacity = info.frame.upBoundingObjCapacity;
				desc.target = J_CULLING_TARGET::RENDERITEM;
				desc.currFrameIndex = IsEntryUpdateLoop() ? info.frame.currIndex : info.frame.currIndex + 1;
				desc.useGpu = true;

				auto user = resourceManage.culling->CreateHdOcclusionData(device.get(), desc);
				if (user.IsValid())
				{
					if (device->CanStartPublicCommand())
					{
						device->FlushCommandQueue();
						device->StartPublicCommand();
						culling.hd->NotifyBuildNewHdOccBuffer(device.get(), info.frame.upBoundingObjCapacity, user);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						culling.hd->NotifyBuildNewHdOccBuffer(device.get(), info.frame.upBoundingObjCapacity, user);
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
						culling.hzb->NotifyDestroyHzbOccBuffer(cullingInfo);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						culling.hzb->NotifyDestroyHzbOccBuffer(cullingInfo);
				}
				else if (cType == J_CULLING_TYPE::HD_OCCLUSION)
				{
					if (device->CanStartPublicCommand())
					{
						device->FlushCommandQueue();
						device->StartPublicCommand();
						culling.hd->NotifyDestroyHdOccBuffer(cullingInfo);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						culling.hd->NotifyDestroyHdOccBuffer(cullingInfo);
				}
				return resourceManage.culling->DestroyCullingData(cullingInfo, device.get());
			}
		public:
			JUserPtr<JGpuAcceleratorInfo> CreateGpuAccelerator(const JGpuAcceleratorBuildDesc& desc)
			{
				if (!device->CanBuildGpuAccelerator())
					return nullptr;

				return resourceManage.accelerator->Create(device.get(), resourceManage.graphic.get(), desc);
			}
			bool DestroyGpuAccelerator(JGpuAcceleratorInfo* info)
			{
				return resourceManage.accelerator->Destroy(device.get(), resourceManage.graphic.get(), info);
			}
			void UpdateTransform(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
			{
				resourceManage.accelerator->UpdateTransform(device.get(), resourceManage.graphic.get(), info, comp);
			}
			void AddComponent(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
			{
				resourceManage.accelerator->Add(device.get(), resourceManage.graphic.get(), info, comp);
			}
			void RemoveComponent(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
			{
				resourceManage.accelerator->Remove(device.get(), resourceManage.graphic.get(), info, comp);
			}
		public:
			bool RegisterHandler(JCsmHandlerInterface* handler)
			{
				return resourceManage.csm->RegisterHandler(handler);
			}
			bool DeRegisterHandler(JCsmHandlerInterface* handler)
			{
				return resourceManage.csm->DeRegisterHandler(handler);
			}
			bool RegisterTarget(JCsmTargetInterface* target)
			{
				return resourceManage.csm->RegisterTarget(target);
			}
			bool DeRegisterTarget(JCsmTargetInterface* target)
			{
				return resourceManage.csm->DeRegisterTarget(target);
			}
		public:
			JOwnerPtr<JShaderDataHolder> StuffGraphicShaderPso(const JGraphicShaderInitData& shaderData)
			{
				device->FlushCommandQueue();
				device->StartPublicCommand();
				auto result = drawing.scene->CreateShader(JGraphicShaderCompileSet(device.get()), shaderData);
				device->EndPublicCommand();
				device->FlushCommandQueue();
				return std::move(result);
			}
			JOwnerPtr<JShaderDataHolder> StuffComputeShaderPso(const JComputeShaderInitData& shaderData)
			{
				JOwnerPtr<JShaderDataHolder> result = nullptr;
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
					result = culling.hzb->CreateComputeShader(device.get(), resourceManage.graphic.get(), shaderData);
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
					resourceManage.frame[i]->ReBuild(device.get(), type, CalculateCapacity(uData));
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

				device->ResizeWindow({ resourceManage.graphic.get() });
				resourceManage.graphic->ResizeWindow(device.get());
				imageProcessing.outline->UpdatePassBuf(info.width, info.height, Constants::commonStencilRef);

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
					resourceManage.graphic.get(),
					resourceManage.culling.get(),
					resourceManage.accelerator.get(),
					resourceManage.currFrame,
					imageProcessing.debug.get(),
					drawing.depthTest.get(),
					imageProcessing.blur.get(),
					imageProcessing.downSampling.get(),
					imageProcessing.ssao.get(),
					imageProcessing.ppEffectSet.get(),
					resourceManage.shareData.get(),
					info.frame.currIndex,
					CalNextFrameResourceIndex(info.frame.currIndex));
			}
		public:
			void UpdateWait()
			{
				info.frame.currIndex = CalNextFrameResourceIndex(info.frame.currIndex);
				resourceManage.currFrame = resourceManage.frame[info.frame.currIndex].get();
				device->UpdateWait(resourceManage.currFrame->GetFenceValue());

				AllocateRefSet();
				if (innerEvent.size() > 0)
				{
					for (const auto& data : innerEvent)
						data->InvokeCompletelyBind();
					innerEvent.clear();
				}

				adapter->BeginUpdateStart(option.deviceType, *drawRefSet);
			}
			void UpdateFrameCapacity()
			{
				updateHelper.Clear();
				for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				{
					if (!updateHelper.uData[i].useGetMultiCount)
						updateHelper.uData[i].count = (*updateHelper.uData[i].getElement)(nullptr);
					updateHelper.uData[i].capacity = resourceManage.currFrame->GetElementCount((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
					UpdateReAllocCondition(updateHelper.uData[i]);
					updateHelper.hasUploadDataDirty |= (bool)updateHelper.uData[i].reAllocCondition;
				}
				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					if (updateHelper.bData[i].HasCallable())
					{
						updateHelper.bData[i].count = (*updateHelper.bData[i].getTextureCount)(nullptr);
						updateHelper.bData[i].capacity = (*updateHelper.bData[i].getTextureCapacity)(nullptr);
						UpdateReAllocCondition(updateHelper.bData[i]);

						updateHelper.hasBindingDataDirty |= (bool)updateHelper.bData[i].reAllocCondition;
						if (updateHelper.bData[i].reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
							updateHelper.bData[i].capacity = CalculateCapacity(updateHelper.bData[i]);
					}
				}

				JGraphicInfo newInfo = info;
				updateHelper.WriteGraphicInfo(newInfo);
				if (updateHelper.hasUploadDataDirty)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();
					for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
					{
						if (updateHelper.uData[i].reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
						{
							ReBuildFrameResource((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
							updateHelper.uData[i].setDirty = Constants::gNumFrameResources;
							updateHelper.uData[i].capacity = resourceManage.currFrame->GetElementCount((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
						}
					}
					//Has sequency dependency
					updateHelper.WriteGraphicInfo(newInfo);
					SetGraphicInfo(newInfo, true, updateHelper.hasBindingDataDirty);

					//drawing.scene->RecompileShader(JGraphicShaderCompileSet(device.get()));		
					//use graphic info
					//updateHelper.NotifyUpdateFrameCapacity(*thisGraphic);	//use graphic info		
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}
				else if (updateHelper.hasBindingDataDirty)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();
					SetGraphicInfo(newInfo, false, true);
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}

			}
			void UpdateFrameBuffer()
			{
				const uint drawListCount = JGraphicDrawList::GetListCount();
				//update frame resource and decide something drawing
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
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
					drawTarget->EndUpdate();

					UpdateSceneCB(drawTarget);
					updateHelper.EndUpdatingDrawTarget();
				}
				UpdateMaterialCB();

#ifdef GRAPIC_DEBUG
				//Debug
				//if(culling.hzb->CanReadBackDebugInfo())
				//	culling.hzb->StreamOutDebugInfo(JApplicationProject::LogPath() + L"\\Hzb.txt");
				//JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(0); 
				//resourceManage.culling->TryStreamOutCullingBuffer(resourceManage.culling->GetCullingInfo(J_CULLING_TYPE::HD_OCCLUSION, 
				//	drawTarget->scene->FindFirstSelectedCamera(false)->CullingUserInterface().GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)).Get(),"LightCullignResult");
				//culling.lit->StreamOutDebugInfo(JApplicationProject::LogPath());
				if (option.rendering.allowRaytracing)
					raytracing.ao->StreamOutDebugInfo();
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
				else if (uBase.count < (uBase.capacity / uBase.downCapacityFactor) && uBase.capacity > info.minCapacity)
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
				const bool isUpdateBoundingObj = scene->IsMainScene() && option.culling.isOcclusionQueryActivated;
				const std::vector<JUserPtr<JComponent>>& jRvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint renderItemCount = (uint)jRvec.size();

				auto currObjectCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT);
				auto currBoundingObjectCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT);
				auto currOccObjectBuffer = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT);
				auto currRefInfoBuffer = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT_REF_INFO);

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].setDirty;

				auto& objSet = contCache.objSet;

				using FrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
				auto updateInfo = target->updateInfo.get();
				for (uint i = 0; i < renderItemCount; ++i)
				{
					JRenderItem* renderItem = static_cast<JRenderItem*>(jRvec[i].Get());
					objSet.Begin();
					objSet.updateStart = FrameUpdateInterface::UpdateStart(renderItem, forcedSetFrameDirty);

					FrameUpdateInterface::UpdateFrame(renderItem, objSet);
					if (objSet.isUpdated[ObjectFrameLayer::object])
						currObjectCB->CopyData(objSet.frameIndex[ObjectFrameLayer::object], objSet.subMeshCount, objSet.object.data());
					if (objSet.isUpdated[ObjectFrameLayer::bounding])
						currBoundingObjectCB->CopyData(objSet.frameIndex[ObjectFrameLayer::bounding], &objSet.bounding);
					if (objSet.isUpdated[ObjectFrameLayer::hzb])
						currOccObjectBuffer->CopyData(objSet.frameIndex[ObjectFrameLayer::hzb], &objSet.hzb);
					if (objSet.isUpdated[ObjectFrameLayer::refInfo])
						currRefInfoBuffer->CopyData(objSet.frameIndex[ObjectFrameLayer::refInfo], objSet.subMeshCount, objSet.refInfo.data());
					if (objSet.updateStart)
					{
						FrameUpdateInterface::UpdateEnd(renderItem);
						if (FrameUpdateInterface::IsLastFrameHotUpdated(renderItem))
							++updateInfo->hotObjUpdateCount;
						++updateInfo->objUpdateCount;
					}
					updateInfo->hasObjRecopy = objSet.hasCopy;
				}

				target->updateInfo->thisFrameObjCount = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].uploadCountPerTarget = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].uploadCountPerTarget = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].uploadCountPerTarget = renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].uploadCountPerTarget = scene->GetMeshCount();
			}
			void UpdateMaterialCB()
			{
				auto currMaterialBuffer = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL);
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
			void UpdateSceneCB(JGraphicDrawTarget* drawTarget)
			{
				//if (!drawTarget->updateInfo->sceneUpdated)
				//	return;

				const JUserPtr<JScene>& scene = drawTarget->scene;
				using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;
				contCache.scenePass.appTotalTime = JEngineTimer::Data().TotalTime();
				contCache.scenePass.appDeltaTime = JEngineTimer::Data().DeltaTime();
				if (scene->IsActivatedSceneTime())
				{
					contCache.scenePass.sceneTotalTime = scene->GetTotalTime();
					contCache.scenePass.sceneDeltaTime = scene->GetDeltaTime();
				}
				else
				{
					contCache.scenePass.sceneTotalTime = 0;
					contCache.scenePass.sceneDeltaTime = 0;
				}

				if (contCache.missing == nullptr)
					contCache.missing = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::MISSING);
				if (contCache.bluseNoise == nullptr)
					contCache.bluseNoise = _JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::BLUE_NOISE);

				auto missingInterface = contCache.missing->GraphicResourceUserInterface();
				auto blueNoiseInterface = contCache.bluseNoise->GraphicResourceUserInterface();

				contCache.scenePass.missingTextureIndex = missingInterface.GetFirstResourceArrayIndex();
				contCache.scenePass.bluseNoiseTextureIndex = blueNoiseInterface.GetFirstResourceArrayIndex();
				contCache.scenePass.bluseNoiseTextureSize = blueNoiseInterface.GetFirstResourceSize();
				contCache.scenePass.invBluseNoiseTextureSize = blueNoiseInterface.GetFirstResourceInvSize();
				contCache.scenePass.clusterMinDepth = std::log2(option.culling.clusterNear);

				auto currSceneCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS);
				currSceneCB->CopyData(SceneFrameIndexInterface::GetFrameIndex(scene.Get()), &contCache.scenePass);
			}
			void UpdateSceneAnimationCB(_In_ const JUserPtr<JScene>& scene, _Inout_ JGraphicDrawTarget* target)
			{
				const std::vector<JUserPtr<JComponent>>& jAvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
				const uint animatorCount = (uint)jAvec.size();

				auto currSkinnedCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION);

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
							FrameUpdateInterface::UpdateFrame(animator, contCache.ani);
							currSkinnedCB->CopyData(FrameUpdateInterface::GetFrameIndex(animator), &contCache.ani);
							FrameUpdateInterface::UpdateEnd(animator);
							++updateInfo->aniUpdateCount;
						}
						else if (FrameUpdateInterface::HasRecopyRequest(animator))
						{
							FrameUpdateInterface::UpdateFrame(animator, contCache.ani);
							currSkinnedCB->CopyData(FrameUpdateInterface::GetFrameIndex(animator), &contCache.ani);
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

				auto currCameraCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA);
				auto currDepthCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS);
				auto currHzbOccReqCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS);
				auto currLitCullCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_CULLING_PASS);
				auto currSsaoCB = resourceManage.currFrame->GetGraphicBufferBase(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS);

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS].setDirty;
				using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;

				auto& camSet = contCache.camSet;
				camSet.hzbQueryCount = hzbOccQueryCount;
				camSet.hzbQueryOffset = hzbOccQueryOffset;

				auto updateInfo = target->updateInfo.get();
				for (uint i = 0; i < cameraCount; ++i)
				{
					JCamera* camera = static_cast<JCamera*>(jCvec[i].Get());
					camSet.Begin();
					camSet.updateStart = FrameUpdateInterface::UpdateStart(camera, forcedSetFrameDirty);

					FrameUpdateInterface::UpdateFrame(camera, camSet);
					if (camSet.isUpdated[CameraFrameLayer::drawScene])
						currCameraCB->CopyData(camSet.frameIndex[CameraFrameLayer::drawScene], &camSet.drawScene);
					if (camSet.isUpdated[CameraFrameLayer::depthTest])
					{
						if (camera->AllowHdOcclusionCulling())
							++updateInfo->hdOccUpdateCount;
						else if (camera->AllowHzbOcclusionCulling())
							++updateInfo->hzbOccUpdateCount;
						currDepthCB->CopyData(camSet.frameIndex[CameraFrameLayer::depthTest], &camSet.depthTest);
					}
					if (camSet.isUpdated[CameraFrameLayer::hzb])
						currHzbOccReqCB->CopyData(camSet.frameIndex[CameraFrameLayer::hzb], &camSet.hzb);
					if (camSet.isUpdated[CameraFrameLayer::lightCulling])
						currLitCullCB->CopyData(camSet.frameIndex[CameraFrameLayer::lightCulling], &camSet.lightCulling);
					if (camSet.isUpdated[CameraFrameLayer::ssao])
						currSsaoCB->CopyData(camSet.frameIndex[CameraFrameLayer::ssao], &camSet.ssao);

					if (camSet.updateStart)
					{
						FrameUpdateInterface::UpdateEnd(camera);
						if (FrameUpdateInterface::IsLastFrameHotUpdated(camera))
							++updateInfo->hotCamUpdateCount;
						++updateInfo->camUpdateCount;
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
				static constexpr int minFrameInit = INT_MAX;
				int minFrameIndex[(uint)J_LIGHT_TYPE::COUNT] = { minFrameInit, minFrameInit, minFrameInit, minFrameInit };

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
				public:
					LitUpdateHelper(JFrameResource* currFrame)
						:currFrame(*currFrame)
					{}
				public:
					void UpdateLit(JLight* light, JLightPrivate* lp, JGraphicDrawTarget::UpdateInfo* info, JLightConstantsSet& set)
					{
						if (!set.isUpdated[LightFrameLayer::light])
							return;

						auto& frameUpdateInterface = lp->GetFrameUpdateInterface();
						const int litFrameIndex = frameUpdateInterface.GetFrameIndex(light, LightFrameLayer::light);
						const J_LIGHT_TYPE litType = light->GetLightType();

						if (litType == J_LIGHT_TYPE::DIRECTIONAL)
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT, litFrameIndex, &set.directionalLight);
						else if (litType == J_LIGHT_TYPE::POINT)
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, litFrameIndex, &set.pointLight);
						else if (litType == J_LIGHT_TYPE::SPOT)
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, litFrameIndex, &set.spotLight);
						else if (litType == J_LIGHT_TYPE::RECT)
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, litFrameIndex, &set.rectLight);
						info->lightUpdateCount += set.updateStart;
					}
					void UpdateShadow(JLight* light, JLightPrivate* lp, JGraphicDrawTarget::UpdateInfo* info, JLightConstantsSet& set)
					{
						if (!set.isUpdated[LightFrameLayer::shadowMap] &&
							!set.isUpdated[LightFrameLayer::shadowMapArray] &&
							!set.isUpdated[LightFrameLayer::shadowMapCube])
							return;

						auto& frameUpdateInterface = lp->GetFrameUpdateInterface();
						const uint shadowLayer = frameUpdateInterface.GetShadowFrameLayerIndex(light);
						const int shadowMapFrameIndex = frameUpdateInterface.GetFrameIndex(light, shadowLayer);
						const J_LIGHT_TYPE litType = light->GetLightType();

						if (litType == J_LIGHT_TYPE::DIRECTIONAL)
						{
							if (shadowLayer == LightFrameLayer::shadowMapArray)
							{
								const uint csmTargetCount = frameUpdateInterface.GetFrameIndexSize(light, shadowLayer);
								currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO, shadowMapFrameIndex, csmTargetCount, set.csm.data());
								currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW, shadowMapFrameIndex, csmTargetCount, set.shadowMapArray.data());
							}
							else
								currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, shadowMapFrameIndex, &set.shadowMap);
						}
						else if (litType == J_LIGHT_TYPE::POINT)
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW, shadowMapFrameIndex, &set.shadowMapCube);
						else if (litType == J_LIGHT_TYPE::SPOT)
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, shadowMapFrameIndex, &set.shadowMap);
						else if (litType == J_LIGHT_TYPE::RECT)
							currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, shadowMapFrameIndex, &set.shadowMap);
						info->shadowMapUpdateCount += set.updateStart;
					}
					void UpdateDepthTest(JLight* light, JLightPrivate* lp, JGraphicDrawTarget::UpdateInfo* info, JLightConstantsSet& set)
					{
						if (!set.isUpdated[LightFrameLayer::depthTest])
							return;

						int frameIndex = lp->GetFrameUpdateInterface().GetFrameIndex(light, LightFrameLayer::depthTest);
						currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, frameIndex, &set.depthTest);
					}
					void UpdateOcc(JLight* light, JLightPrivate* lp, JGraphicDrawTarget::UpdateInfo* info, JLightConstantsSet& set)
					{
						if (!set.isUpdated[LightFrameLayer::hzb])
							return;

						int frameIndex = lp->GetFrameUpdateInterface().GetFrameIndex(light, LightFrameLayer::hzb);
						currFrame.CopyData(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, frameIndex, &set.hzb);
						info->hzbOccUpdateCount += set.updateStart;
					}
				};

				auto& litSet = contCache.litSet;
				litSet.hzbQueryCount = hzbOccQueryCount;
				litSet.hzbQueryOffset = hzbOccQueryOffset;

				LitUpdateHelper litUpdateHelper(resourceManage.currFrame);
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

					litSet.Begin();
					litSet.updateStart = frameUpdateInterface.UpdateStart(light, forcedSetFrameDirtyValue);
					frameUpdateInterface.UpdateFrame(light, litSet);
					litUpdateHelper.UpdateLit(light, lp, updateInfo, litSet);
					litUpdateHelper.UpdateShadow(light, lp, updateInfo, litSet);
					litUpdateHelper.UpdateDepthTest(light, lp, updateInfo, litSet);
					litUpdateHelper.UpdateOcc(light, lp, updateInfo, litSet);

					if (litSet.updateStart)
					{
						frameUpdateInterface.UpdateEnd(light);
						if (frameUpdateInterface.IsLastFrameHotUpdated(light))
							++updateInfo->hotLitghtUpdateCount;
					}
					++litCount[(uint)litType];
					int frameIndex = frameUpdateInterface.GetFrameIndex(light, LightFrameLayer::light);
					if (frameIndex < minFrameIndex[(uint)litType])
						minFrameIndex[(uint)litType] = frameIndex;
				}

				contCache.scenePass.directionalLitSt = minFrameInit != minFrameIndex[(int)J_LIGHT_TYPE::DIRECTIONAL] ? minFrameIndex[(int)J_LIGHT_TYPE::DIRECTIONAL] : 0;
				contCache.scenePass.directionalLitEd = contCache.scenePass.directionalLitSt + litCount[(uint)J_LIGHT_TYPE::DIRECTIONAL];
				contCache.scenePass.pointLitSt = minFrameInit != minFrameIndex[(int)J_LIGHT_TYPE::POINT] ? minFrameIndex[(int)J_LIGHT_TYPE::POINT] : 0;
				contCache.scenePass.pointLitEd = contCache.scenePass.pointLitSt + litCount[(uint)J_LIGHT_TYPE::POINT];
				contCache.scenePass.spotLitSt = minFrameInit != minFrameIndex[(int)J_LIGHT_TYPE::SPOT] ? minFrameIndex[(int)J_LIGHT_TYPE::SPOT] : 0;
				contCache.scenePass.spotLitEd = contCache.scenePass.spotLitSt + litCount[(uint)J_LIGHT_TYPE::SPOT];
				contCache.scenePass.rectLitSt = minFrameInit != minFrameIndex[(int)J_LIGHT_TYPE::RECT] ? minFrameIndex[(int)J_LIGHT_TYPE::RECT] : 0;
				contCache.scenePass.rectLitEd = contCache.scenePass.rectLitSt + litCount[(uint)J_LIGHT_TYPE::RECT];

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
					option.culling.isLightCullingActivated &&
					option.culling.allowLightCluster &&
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
						const int frameIndex = lp->GetFrameUpdateInterface().GetFrameIndex(lit, LightFrameLayer::light);
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
				if (!canDraw)
					return;
				/*
				* 사용예정인 Graphic resource들은 Draw이전에 모두 할당되야하며(Gpu timeline ... resource할당에 대한 command가 사용하는 command이전에 위치하면 ok)
				* Draw중에 Resource에 대해서 할당, 해제하는 것은 오류를 일으킬수있으니 AddInnerEvent를 통해
				* UpdateWait()함수 호출에 수행하도록한다.
				*/
				resourceManage.context->Begin();
				alignedObject.ClearAlignedVecElement();
				AllocateRefSet();
				if (allowDrawScene)
				{
					if (option.rendering.allowMultiThread)
						DrawUseMultiThread();
					else
						DrawUseSingleThread();
				}
				else
					EndFrame(false);
				resourceManage.context->End();
			}
		private:
			void DrawUseSingleThread()
			{
				JGraphicDrawSceneSTSet dataSet;
				adapter->BeginDrawSceneSingleThread(option.deviceType, *drawRefSet, dataSet);

				const J_GRAPHIC_RENDERING_PROCESS objRenderingType = option.rendering.allowDeferred ? J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY : J_GRAPHIC_RENDERING_PROCESS::FORWARD;
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

						culling.frustum->FrustumCulling(drawTarget->scene, JDrawHelper::CreateFrustumCullingHelper(helper, data->comp));
					}
					if (helper.scene->AllowLightCulling())
					{
						culling.lit->BindDrawResource(dataSet.bind.get());
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.lit->ExecuteLightClusterTask(dataSet.litCulling.get(), JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
						}
					}
					if (option.IsOcclusionActivated())
					{
						//1.draw depth map
						//2.create mipmap and compute hzb
						//3.draw imageProcessing.debug map
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hzb->DrawOcclusionDepthMap(dataSet.occDraw.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hzb->ComputeOcclusionCulling(dataSet.hzbCompute.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}

					drawing.shadowMap->BindResource(dataSet.bind.get());
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						drawing.shadowMap->DrawSceneShadowMap(dataSet.shadowMapDraw.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						imageProcessing.debug->ComputeLitDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}

					drawing.scene->BindResource(objRenderingType, dataSet.bind.get());
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						drawing.scene->DrawSceneRenderTarget(dataSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						drawing.scene->ComputeSceneDependencyTemporalResource(dataSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					if (option.rendering.allowDeferred)
					{
						drawing.scene->BindResource(J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING, dataSet.bind.get());
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							drawing.scene->DrawSceneShade(dataSet.sceneDraw.get(),
								JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
						}
					}
					if (option.CanUseRtGi())
					{
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->jCamera->AllowRaytracingGI())
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingDrawScene(data->jCamera);
							raytracing.gi->ComputeGI(dataSet.rtgi.get(), copiedHelper);
							raytracing.denoiser->ApplyGIDenoise(dataSet.rtDenoiser.get(), copiedHelper);
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

							imageProcessing.ssao->ApplySsao(dataSet.ssao.get(), copiedHelper);
						}
					}
					if (option.IsOcclusionActivated())
					{
						//1.query test
						//2.extract query result
						//3.draw imageProcessing.debug map
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hd->DrawOcclusionDepthMap(dataSet.occDraw.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hd->ExtractHDOcclusionCullingData(dataSet.hdExtract.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}

					drawing.scene->BindResource(J_GRAPHIC_RENDERING_PROCESS::FORWARD, dataSet.bind.get());
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						drawing.scene->DrawSceneDebugUI(dataSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						imageProcessing.outline->DrawCamOutline(dataSet.outline.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->jCamera->AllowPostProcess())
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						imageProcessing.ppPipeline->ApplyPostProcess(dataSet.postPrcess.get(), copiedHelper, data->canDrawThisFrame);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						imageProcessing.debug->ComputeCamDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}

					if (option.debugging.allowDisplayLightCullingResult && helper.scene->AllowLightCulling())
					{
						culling.lit->BindDebugResource(dataSet.bind.get());
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.lit->ExecuteLightClusterDebug(dataSet.litCullingDebug.get(),
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

				for (uint i = 0; i < info.frame.threadCount; ++i)
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

						culling.frustum->FrustumCulling(drawTarget->scene, JDrawHelper::CreateFrustumCullingHelper(helper, data->comp));
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

				const J_GRAPHIC_RENDERING_PROCESS objRenderingType = option.rendering.allowDeferred ? J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY : J_GRAPHIC_RENDERING_PROCESS::FORWARD;
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);
				helper.SetAllowMultithreadDraw(true);
				helper.SetTheadInfo(info.frame.threadCount, threadIndex);

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

							culling.hzb->DrawOcclusionDepthMapMultiThread(occTaskSet.occDraw.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
				}
				adapter->NotifyCompleteDrawOccTask(option.deviceType, *drawRefSet, threadIndex);
				adapter->SettingDrawShadowMapTask(option.deviceType, *drawRefSet, threadIndex, shadowMapTaskSet);
				drawing.shadowMap->BindResource(shadowMapTaskSet.bind.get());
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						drawing.shadowMap->DrawSceneShadowMapMultiThread(shadowMapTaskSet.shadowMapDraw.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
				}

				adapter->NotifyCompleteDrawShadowMapTask(option.deviceType, *drawRefSet, threadIndex);
				adapter->SettingDrawSceneTask(option.deviceType, *drawRefSet, threadIndex, sceneTaskSet);
				drawing.scene->BindResource(objRenderingType, sceneTaskSet.bind.get());

				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						drawing.scene->DrawSceneRenderTargetMultiThread(sceneTaskSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}
				if (option.IsOcclusionActivated())
				{
					for (uint i = 0; i < drawListCount; ++i)
					{
						JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
						helper.SetDrawTarget(drawTarget);
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hd->DrawOcclusionDepthMapMultiThread(sceneTaskSet.occDraw.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
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

				culling.lit->BindDrawResource(dataSet.bind.get());
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

							culling.lit->ExecuteLightClusterTask(dataSet.litCulling.get(),
								JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
						}
					}
				}
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.IsOcclusionActivated() && culling.hzb->HasPreprocessing())
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hzb->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsOcclusionActivated() && culling.hd->HasPreprocessing())
					{
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hd->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (drawing.shadowMap->HasPreprocessing())
					{
						for (const auto& data : drawTarget->shadowRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							drawing.shadowMap->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
						}
					}
					if (drawing.scene->HasPreprocessing())
					{ 
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							drawing.scene->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
						}
					}
				}
				adapter->ExecuteBeginFrame(option.deviceType, *drawRefSet);
			}
			void MidFrame()
			{
				JGraphicMidFrameSet dataSet;
				adapter->SettingMidFrame(option.deviceType, *drawRefSet, dataSet);

				//mostly handle compute shader task or drawing imageProcessing.debug
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);

				if (option.rendering.allowDeferred)
				{
					drawing.scene->BindResource(J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING, dataSet.bind.get());
					for (uint i = 0; i < drawListCount; ++i)
					{
						JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
						helper.SetDrawTarget(drawTarget);
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							drawing.scene->DrawSceneShadeMultiThread(dataSet.sceneDraw.get(),
								JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
						}
					}
				}
				drawing.scene->BindResource(J_GRAPHIC_RENDERING_PROCESS::FORWARD, dataSet.bind.get());
				//EndDraw Process
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.IsOcclusionActivated() && culling.hzb->HasPostprocessing())
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hzb->EndDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsOcclusionActivated() && culling.hd->HasPostprocessing())
					{
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hd->EndDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (drawing.shadowMap->HasPostprocessing())
					{
						for (const auto& data : drawTarget->shadowRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							drawing.shadowMap->EndDraw(dataSet.bind.get(), JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
						}
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						if (copiedHelper.allowDrawDebugObject)
							drawing.scene->DrawSceneDebugUIMultiThread(dataSet.sceneDraw.get(), copiedHelper);
						drawing.scene->EndDraw(dataSet.bind.get(), copiedHelper);
					}
				}

				//after Scene EndDraw
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

							culling.hzb->ComputeOcclusionCulling(dataSet.hzbCompute.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (option.IsOcclusionActivated())
					{
						for (const auto& data : drawTarget->hdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							culling.hd->ExtractHDOcclusionCullingData(dataSet.hdExtract.get(),
								JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						imageProcessing.debug->ComputeLitDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
					if (option.CanUseRtGi())
					{ 
						for (const auto& data : drawTarget->sceneRequestor)
						{
							if (!data->jCamera->AllowRaytracingGI())
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingDrawScene(data->jCamera);
							drawing.scene->ComputeSceneDependencyTemporalResource(dataSet.sceneDraw.get(), copiedHelper);
							raytracing.gi->ComputeGI(dataSet.rtgi.get(), copiedHelper);
							raytracing.denoiser->ApplyGIDenoise(dataSet.rtDenoiser.get(), copiedHelper);
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
							imageProcessing.ssao->ApplySsao(dataSet.ssao.get(), copiedHelper);
						}
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->jCamera->AllowPostProcess())
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						imageProcessing.ppPipeline->ApplyPostProcess(dataSet.postPrcess.get(), copiedHelper, data->canDrawThisFrame);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						imageProcessing.debug->ComputeCamDebug(dataSet.debugCompute.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						imageProcessing.outline->DrawCamOutline(dataSet.outline.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}
				if (option.debugging.allowDisplayLightCullingResult)
				{
					culling.lit->BindDebugResource(dataSet.bind.get());
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

								culling.lit->ExecuteLightClusterDebug(dataSet.litCullingDebug.get(),
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

				if (option.rendering.allowDrawGui)
				{
					guiBackendInterface->SettingGuiDrawing();
					guiBackendInterface->Draw(guiAdapter->CreateDrawData(device.get(),
						resourceManage.graphic.get(),
						resourceManage.currFrame,
						option,
						guiBackendInterface->GetGuiIdentification()));
				}
				adapter->ExecuteEndFrame(option.deviceType, *drawRefSet, condSet);
			}
		public:
			void Initialize(std::unique_ptr<JGraphicAdapter>&& newAdpter,
				std::unique_ptr<JGuiBackendDataAdapter> newGuiAdapter,
				JGuiBackendInterface* newGuiBackendInterface)
			{
				const JVector2F clientSize = JWindow::GetClientSize();
				info.width = clientSize.x;
				info.height = clientSize.y;

				adapter = std::move(newAdpter);
				guiAdapter = std::move(newGuiAdapter);
				guiBackendInterface = newGuiBackendInterface;

				auto pushEvLam = [](std::unique_ptr<Core::JBindHandleBase>&& b) {_JGraphic::Instance().impl->AddInnerEvent(std::move(b)); };
				JGraphicSubClassShareData shareData(&frameAccess, pushEvLam);
				device = adapter->CreateDevice(option.deviceType, shareData);
				adapter->CreateResourceManageSubclass(option.deviceType, shareData, resourceManage);
				adapter->CreateDrawSubclass(option.deviceType, shareData, drawing);
				adapter->CreateCullingSubclass(option.deviceType, shareData, culling);
				adapter->CreateImageProcessingSubclass(option.deviceType, shareData, imageProcessing);
				adapter->CreateRaytracingSubclass(option.deviceType, shareData, raytracing);

				std::vector<JGraphicSubClassInterface*> managedSubclass;
				resourceManage.GetManageSubclass(managedSubclass);
				drawing.GetManageSubclass(managedSubclass);
				culling.GetManageSubclass(managedSubclass);
				imageProcessing.GetManageSubclass(managedSubclass);
				raytracing.GetManageSubclass(managedSubclass);

				for (uint i = 0; i < SIZE_OF_ARRAY(infoChangedListener); ++i)
				{
					for (const auto& data : managedSubclass)
					{
						if (data->HasDependency((JGraphicInfo::TYPE)i))
							infoChangedListener[i].push_back(data);
					}
				}
				for (uint i = 0; i < SIZE_OF_ARRAY(optionChangedListener); ++i)
				{
					for (const auto& data : managedSubclass)
					{
						if (data->HasDependency((JGraphicOption::TYPE)i))
							optionChangedListener[i].push_back(data);
					}
				}

				if (InitializeResource())
				{
					AllocateRefSet();
					OnResize();
				}
				else
					Clear();
				LoadData();

				canDraw = Window::JWindow::IsActivated();
				std::vector<Window::J_WINDOW_EVENT> evList
				{
					Window::J_WINDOW_EVENT::WINDOW_RESIZE,
					Window::J_WINDOW_EVENT::WINDOW_ACTIVATE,
					Window::J_WINDOW_EVENT::WINDOW_DEACTIVATE
				};
				WindowEventListener::AddEventListener(*JWindow::EvInterface(), guid, evList);
			}
			void Clear()
			{
				WindowEventListener::RemoveListener(*JWindow::EvInterface(), guid);
				if (JApplicationEngine::GetApplicationState() != J_APPLICATION_STATE::PROJECT_SELECT)
					StoreData();

				drawRefSet = nullptr;
				updateHelper.Clear();
				device->FlushCommandQueue();
				device->StartPublicCommand();

				for (uint i = 0; i < SIZE_OF_ARRAY(infoChangedListener); ++i)
					infoChangedListener[i].clear();
				for (uint i = 0; i < SIZE_OF_ARRAY(optionChangedListener); ++i)
					optionChangedListener[i].clear();

				raytracing.Clear();
				imageProcessing.Clear();
				culling.Clear();
				drawing.Clear();
				resourceManage.Clear();
				info.frame.currIndex = 0;

				device->EndPublicCommand();
				device->FlushCommandQueue();
				device = nullptr;

				guiBackendInterface = nullptr;
				guiAdapter = nullptr;
				adapter = nullptr;

				info.width = 0;
				info.height = 0;
			}
		private:
			bool InitializeResource()
			{
				adapter->Initialize(resourceManage.context.get(), option.deviceType);

				bool res = true;
				res &= device->CreateDeviceObject();
				device->FlushCommandQueue();
				device->StartPublicCommand();

				resourceManage.Initialize(device.get(), info);
				drawing.Initialize(device.get(), &resourceManage);
				culling.Initialize(device.get(), &resourceManage);
				imageProcessing.Initialize(device.get(), &resourceManage);
				raytracing.Initialize(device.get(), &resourceManage);

				device->EndPublicCommand();
				device->FlushCommandQueue();
				return res;
			}
		public:
			void LoadData()
			{
				JGraphicInfo newinfo = info;
				JGraphicOption newOption;
				newinfo.Load();
				newOption.Load();

				SetGraphicInfo(newinfo, true, true);
				SetOption(newOption);
			}
			void StoreData()
			{
				info.Store();
				option.Store();
			}
			void WriteLastRsTexture()
			{
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				{
					// 0 is main camera
					JApplicationProjectInfo* opendInfo = JApplicationProject::GetOpenProjectInfo();
					if (opendInfo == nullptr)
						J_LOG_PRINT_OUT("Get open proejct info erro", "");

					//main scene
					auto firstDrawTarget = JGraphicDrawList::GetDrawScene(0);
					auto firstCam = firstDrawTarget->scene->FindFirstSelectedCamera(false);

					if (firstCam != nullptr)
					{
						auto gUser = firstCam->GraphicResourceUserInterface();
						const int index = gUser.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
						resourceManage.graphic->StoreTexture(device.get(), J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, index, opendInfo->lastRsPath());
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

				//window size가 0일시 graphic update을 제한한다.
				if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
				{
					OnResize();
					canDraw = info.width > 0 && info.height > 0;
				}
				else if (eventType == Window::J_WINDOW_EVENT::WINDOW_ACTIVATE)
					canDraw = info.width > 0 && info.height > 0;
				else if (eventType == Window::J_WINDOW_EVENT::WINDOW_DEACTIVATE)
				{
					FlushCommandQueue();
					canDraw = false;
				}
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
		const JGraphicInfo& JGraphic::GetGraphicInfoRef()const noexcept
		{
			return impl->info;
		}
		JGraphicOption JGraphic::GetGraphicOption()const noexcept
		{
			return impl->GetGraphicOption();
		}
		const JGraphicOption& JGraphic::GetGraphicOptionRef()const noexcept
		{
			return impl->option;
		}
		void JGraphic::GetLastDeviceErrorInfo(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)const noexcept
		{
			impl->GetLastDeviceErrorInfo(errorCode, errorMsg);
		}
		std::vector<JCommandContextLog> JGraphic::GetCpuDrawingLog()const noexcept
		{
			return impl->GetCpuDrawingLog();
		}
		void JGraphic::SetGraphicOption(JGraphicOption newGraphicOption)noexcept
		{
			impl->SetOption(newGraphicOption);
		}
		bool JGraphic::IsRaytracingSupported()const noexcept
		{
			return impl->IsRaytracingSupported();
		}
		bool JGraphic::CanBuildGpuAccelerator()const noexcept
		{
			return impl->CanBuildGpuAccelerator();
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
		using AcceleratorInterface = JGraphicPrivate::AcceleratorInterface;
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
			return JinEngine::JGraphic::Instance().impl->CreateCustomMipmap(info, createDesc);
		}
		bool ResourceInterface::SetTextureDetail(const JUserPtr<JGraphicResourceInfo>& info, const JConvertColorDesc& convertDesc)
		{
			return JinEngine::JGraphic::Instance().impl->SetTextureDetail(info, convertDesc);
		}
		JOwnerPtr<JShaderDataHolder> ResourceInterface::StuffGraphicShaderPso(const JGraphicShaderInitData& shaderData)
		{
			return JinEngine::JGraphic::Instance().impl->StuffGraphicShaderPso(shaderData);
		}
		JOwnerPtr<JShaderDataHolder> ResourceInterface::StuffComputeShaderPso(const JComputeShaderInitData& shaderData)
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

		JUserPtr<JGpuAcceleratorInfo> AcceleratorInterface::CreateGpuAccelerator(const JGpuAcceleratorBuildDesc& desc)
		{
			return JinEngine::JGraphic::Instance().impl->CreateGpuAccelerator(desc);
		}
		bool AcceleratorInterface::DestroyGpuAccelerator(JGpuAcceleratorInfo* info)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyGpuAccelerator(info);
		}
		void AcceleratorInterface::UpdateTransform(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
		{
			JinEngine::JGraphic::Instance().impl->UpdateTransform(info, comp);
		}
		void AcceleratorInterface::AddComponent(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
		{
			JinEngine::JGraphic::Instance().impl->AddComponent(info, comp);
		}
		void AcceleratorInterface::RemoveComponent(JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp)
		{
			JinEngine::JGraphic::Instance().impl->RemoveComponent(info, comp);
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
			JinEngine::JGraphic::Instance().impl->UpdateFrameCapacity();
			JinEngine::JGraphic::Instance().impl->UpdateFrameBuffer();
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