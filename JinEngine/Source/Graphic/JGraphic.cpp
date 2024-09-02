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
#include"DataSet/JGraphicObjectDataSetManager.h"
#include"Scene/JSceneDraw.h"
#include"Scene/JOutline.h"
#include"Scene/JSceneVelocity.h"
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
#include"Raytracing/Light/Global/JRaytracingGI.h"
#include"Raytracing/Occlusion/JRaytracingAmbientOcclusion.h" 
#include"Raytracing/Denoiser/JRaytracingDenoiser.h" 

#include"FrameResource/JFrameResource.h"  
#include"FrameResource/JFrameResourceManager.h"   

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
#include"../Core/Math/JMathHelper.h"

#include"../Object/GameObject/JGameObject.h"
#include"../Object/Component/RenderItem/JRenderItem.h"  
#include"../Object/Component/Camera/JCamera.h"  
#include"../Object/Component/Light/JLight.h"  
 
#include"../Object/Resource/Scene/JScene.h" 
#include"../Object/Resource/Scene/JSceneManager.h" 
#include"../Object/Resource/Shader/JShader.h" 

#include"../Object/GraphicRule/JGraphicModuleInterface.h"
#include"../Object/GraphicRule/JGraphicModuleInterfaceHolder.h"

#include"../Window/JWindow.h"
#include"../Window/JWindowPrivate.h"

#include"../Application/Engine/JApplicationEngine.h"
#include"../Application/Engine/JApplicationEnginePrivate.h"
#include"../Application/Project/JApplicationProject.h"  

#include"../Core/Time/JStopWatch.h"
//#ifdef DEVELOP
//#include"../Develop/Debug/JDevelopDebug.h"  
//#endif

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
		
			culling->Initialize(device);
			csm->Initialize(device);
			accelerator->Initialize(device);
			frame->Initialize(device);  
			objectData->Initialize(device);
		}
		void JResourceManageSubclassSet::Clear()
		{ 
			shareData = nullptr;
			objectData = nullptr;
			frame = nullptr;
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
			outV.push_back(frame.get());
			outV.push_back(objectData.get());
			outV.push_back(shareData.get()); 
		}
		void JSceneDrawingSubclassSet::Initialize(JGraphicDevice* device, JResourceManageSubclassSet* resourceManage)
		{
			scene->Initialize(device, resourceManage->graphic.get());
			shadowMap->Initialize(device, resourceManage->graphic.get());
			depthTest->Initialize(device, resourceManage->graphic.get());

			outline->Initialize(device, resourceManage->graphic.get());
			debug->Initialize(device, resourceManage->graphic.get());
			velocity->Initialize(device, resourceManage->graphic.get());
		}
		void JSceneDrawingSubclassSet::Clear()
		{
			velocity = nullptr;
			debug = nullptr;
			outline = nullptr;

			depthTest = nullptr;
			shadowMap = nullptr;
			scene = nullptr;
		}
		void JSceneDrawingSubclassSet::GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV)
		{
			outV.push_back(scene.get());
			outV.push_back(shadowMap.get());
			outV.push_back(depthTest.get());

			outV.push_back(outline.get());
			outV.push_back(debug.get());
			outV.push_back(velocity.get());
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
		}
		void JImageProcessingSubclassSet::GetManageSubclass(std::vector<JGraphicSubClassInterface*>& outV)
		{
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
		 
		namespace
		{ 
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
		class JGraphic::JGraphicImpl : public WindowEventListener
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
		private:
			std::unique_ptr<JGraphicAdapter> adapter;
		public:
			std::unique_ptr<JGraphicDevice> device;
		public:
			JResourceManageSubclassSet resourceManage;
			JSceneDrawingSubclassSet drawing;
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
				:guid(guid), thisGraphic(thisGraphic)
			{
				IntializeGraphicInfo();
				InitializeGameObjectBuffer();
				workerFunctor = std::make_unique<WorkerThreadF::Functor>(&JGraphicImpl::WorkerThread, this);
			}
			~JGraphicImpl()
			{ }
		private:
			//CallOnce
			void IntializeGraphicInfo()
			{
				const uint occMipmapViewCapa = Constants::occlusionMipmapViewCapacity;
				info.resource.occlusionWidth = std::pow(2, occMipmapViewCapa - 1);
				info.resource.occlusionHeight = std::pow(2, occMipmapViewCapa - 1);
				info.resource.occlusionMinSize = Constants::minOcclusionSize;
				info.resource.occlusionMapCapacity = occMipmapViewCapa;
				info.resource.occlusionMapCount = JMathHelper::Log2Int(info.resource.occlusionWidth) - JMathHelper::Log2Int(Constants::minOcclusionSize) + 1;
				info.frame.threadCount = _JThreadManager::Instance().GetReservedSpaceCount(Core::J_THREAD_USE_CASE_TYPE::ENGINE_TASK_SYNC);
			}
			//CallOnce
			void InitializeGameObjectBuffer()
			{
				alignedObject.common.resize(info.minCapacity);
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
			void SetGraphicInfo(const JGraphicInfo& newInfo, bool isFrameDirty, bool isResourceDirty, bool useCommand)
			{
				if (useCommand)
				{
					device->FlushCommandQueue();
					device->StartPublicCommand();
				}

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

				if (alignedObject.common.size() != info.frame.capacity[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT])
					alignedObject.common.resize(info.frame.capacity[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT]);

				if (useCommand)
				{
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}
			}
			void SetOption(JGraphicOption newGraphicOption)noexcept
			{
#pragma region PreProcess
				bool changedMask[(uint)JGraphicOption::TYPE::COUNT];
				memset(&changedMask, false, sizeof(bool) * SIZE_OF_ARRAY(changedMask));
				 
#pragma endregion
#pragma region  Restrict Value
				newGraphicOption.culling.clusterXIndex = std::clamp(newGraphicOption.culling.clusterXIndex, (uint)0, Constants::litClusterXVariation - 1);
				newGraphicOption.culling.clusterYIndex = std::clamp(newGraphicOption.culling.clusterYIndex, (uint)0, Constants::litClusterYVariation - 1);
				newGraphicOption.culling.clusterZIndex = std::clamp(newGraphicOption.culling.clusterZIndex, (uint)0, Constants::litClusterZVariation - 1);
				newGraphicOption.culling.lightPerClusterIndex = std::clamp(newGraphicOption.culling.lightPerClusterIndex, (uint)0, Constants::lightPerClusterVariation - 1);
				newGraphicOption.culling.clusterNear = std::clamp(newGraphicOption.culling.clusterNear, Constants::litClusterNearMin, Constants::litClusterNearMax);
				newGraphicOption.culling.clusterPointLightRangeOffset = std::clamp(newGraphicOption.culling.clusterPointLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);
				newGraphicOption.culling.clusterSpotLightRangeOffset = std::clamp(newGraphicOption.culling.clusterSpotLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);
				newGraphicOption.culling.clusterRectLightRangeOffset = std::clamp(newGraphicOption.culling.clusterRectLightRangeOffset, Constants::litClusterRangeOffsetMin, Constants::litClusterRangeOffsetMax);

				for (uint i = 0; i < (uint)J_GRAPHIC_OPTIONAL_FEATURE::COUNT; ++i)
				{
					const J_GRAPHIC_OPTIONAL_FEATURE type = (J_GRAPHIC_OPTIONAL_FEATURE)i;
					bool* ptr = newGraphicOption.GetOptionalFeatureValuePtr(type);
					if (ptr == nullptr)
						continue;

					if(*ptr) 
						*ptr &= device->IsSupported(type);
				}

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

#pragma endregion
#pragma region Mask
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.allowMultiThread != newGraphicOption.rendering.allowMultiThread);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.allowDeferred != newGraphicOption.rendering.allowDeferred);
				changedMask[(uint)JGraphicOption::TYPE::RENDERING] |= (option.rendering.allowRaytracing != newGraphicOption.rendering.allowRaytracing);
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
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useTaa != newGraphicOption.postProcess.useTaa);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useToneMapping != newGraphicOption.postProcess.useToneMapping);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useBloom != newGraphicOption.postProcess.useBloom);
				changedMask[(uint)JGraphicOption::TYPE::POST_PROCESS] |= (option.postProcess.useBlur != newGraphicOption.postProcess.useBlur);

#ifdef DEVELOP
				//debugging`
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileGraphicShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileLightClusterShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileSsaoShader; ; ;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileToneMappingShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileRtGiShader;		
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileRtDenoiseShader;
				changedMask[(uint)JGraphicOption::TYPE::DEBUGGING] |= newGraphicOption.debugging.requestRecompileTAAShader;
				//dependencyOption[RECOMPILE_SSAO] |= (option.drawSsaoByComputeShader != newGraphicOption.drawSsaoByComputeShader);
#endif 
#pragma endregion
#pragma region Switch
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
#pragma endregion
#pragma region  Reflect option changed
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
						auto formatDependencyLam = [](JGraphicImpl* impl)
						{
							impl->device->FlushCommandQueue();
							impl->device->StartPublicCommand();
							impl->device->NotifyChangedBackBufferFormat(JGraphicDeviceInitSet(impl->resourceManage.graphic.get()));						
							impl->guiBackendInterface->ReBuildGraphicBackend(impl->GetGuiInitData());
							impl->device->EndPublicCommand();
							impl->device->FlushCommandQueue();
						}; 
						AddInnerEvent(Core::UniqueBind(std::make_unique<InnerEventF::Functor>(formatDependencyLam), this));
					} 
					device->EndPublicCommand();
					device->FlushCommandQueue();
				}
#pragma endregion
#pragma region PostProcess
#ifdef DEVELOP
				option.debugging.requestRecompileGraphicShader =
					option.debugging.requestRecompileLightClusterShader =
					option.debugging.requestRecompileSsaoShader =
					option.debugging.requestRecompileToneMappingShader =
					option.debugging.requestRecompileRtGiShader =
					option.debugging.requestRecompileRtDenoiseShader = 
					option.debugging.requestRecompileTAAShader =false;
#endif
#pragma endregion

			}
			bool SetCustomMipmap(const JUserPtr<JGraphicResourceInfo>& srcInfo, JTextureCreationDesc& creationDesc)
			{
				if (srcInfo == nullptr || creationDesc.mipMapDesc.type == J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT || creationDesc.mipMapDesc.type == J_GRAPHIC_MIP_MAP_TYPE::NONE)
					return false;

				JUserPtr<JGraphicResourceInfo> intermediate00 = nullptr;
				JUserPtr<JGraphicResourceInfo> intermediate01 = nullptr;
				auto rType = srcInfo->GetGraphicResourceType();
				if (rType != J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D && rType != J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
					return false;

				auto resourceSize = srcInfo->GetResourceSize();
				//추가로 custom mipmap이 필요한 J_GRAPHIC_RESOURCE_TYPE이 있을경우 수정필요.	
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON, J_GRAPHIC_TASK_TYPE::UNKNOWN);
				JGraphicResourceCreationDesc cDesc(typeSet, std::make_unique<JTextureCreationDesc>(creationDesc));
				cDesc.width = resourceSize.x;
				cDesc.height = resourceSize.y;
				cDesc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				cDesc.bindDesc.useEngineDefinedBindType = false;
				cDesc.formatHint = std::make_unique<JGraphicFormatHint>();
				cDesc.formatHint->format = srcInfo->GetFormat();
				if (cDesc.formatHint->format == J_GRAPHIC_RESOURCE_FORMAT::API_SPECIALIZED)
					return false;

				intermediate00 = resourceManage.graphic->CreateResource(device.get(), cDesc);
				intermediate01 = resourceManage.graphic->CreateResource(device.get(), cDesc);
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
					const JMipmapGenerationDesc& mipmapDesc = creationDesc.mipMapDesc;
					const JDrawHelper helper(info, option, alignedObject);

					imageProcessing.downSampling->ApplyMipmapGeneration(mipmapSetting.dataSet.get(), helper);
					adapter->EndMipmapGenerationTask(option.deviceType, *drawRefSet);

					std::unique_ptr<JBlurDesc> blurDesc;
					switch (mipmapDesc.type)
					{
					case JinEngine::J_GRAPHIC_MIP_MAP_TYPE::BOX:
					{
						blurDesc = std::make_unique<JBoxBlurDesc>(imageSize * 0.5f, mipmapDesc.kernelSize);
						break;
					}
					case JinEngine::J_GRAPHIC_MIP_MAP_TYPE::GAUSSIAN:
					{
						blurDesc = std::make_unique<JGaussianBlurDesc>(imageSize * 0.5f, mipmapDesc.kernelSize, mipmapDesc.sharpnessFactor);
						break;
					}
					case JinEngine::J_GRAPHIC_MIP_MAP_TYPE::KAISER:
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
				const JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON, J_GRAPHIC_TASK_TYPE::UNKNOWN);
				JGraphicResourceCreationDesc cDesc(typeSet); 
				cDesc.width = resourceSize.x / 4;
				cDesc.height = resourceSize.y / 4;
				cDesc.textureDesc = std::make_unique<JTextureCreationDesc>();
				cDesc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT;
				cDesc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
				cDesc.bindDesc.useEngineDefinedBindType = false;
				cDesc.formatHint = std::make_unique<JGraphicFormatHint>();
				cDesc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32G32B32A32_FLOAT;
				intermediate00 = resourceManage.graphic->CreateResource(device.get(), cDesc);

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
			bool IsSupported(const J_GRAPHIC_OPTIONAL_FEATURE featureType)const noexcept
			{
				return device->IsSupported(featureType);
			}
			bool IsActivated(const J_GRAPHIC_OPTIONAL_FEATURE featureType)const noexcept
			{
				switch (featureType)
				{
				case JinEngine::J_GRAPHIC_OPTIONAL_FEATURE::DEFERRED_RENDERING:
					return option.rendering.allowDeferred;
				case JinEngine::J_GRAPHIC_OPTIONAL_FEATURE::RAYTRACING:
					return option.rendering.allowRaytracing;
				case JinEngine::J_GRAPHIC_OPTIONAL_FEATURE::RAYTRACING_GI:
					return option.CanUseRtGi();
				case JinEngine::J_GRAPHIC_OPTIONAL_FEATURE::POST_PROCESSING:
					return option.postProcess.usePostprocess;
				case JinEngine::J_GRAPHIC_OPTIONAL_FEATURE::GPU_ACCELERATOR:
					return IsSupported(featureType); 
				default:
					break;
				}
				return false; 
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
			JUserPtr<JGraphicResourceInfo> CreateResource(const JGraphicResourceCreationDesc& creationDesc)
			{
				auto userPtr = resourceManage.graphic->CreateResource(device.get(), creationDesc);
				if (userPtr == nullptr)
					return userPtr;

				if (creationDesc.type.resouce == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D || creationDesc.type.resouce == J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
				{
					if (!creationDesc.textureDesc->UseMipmap())
						userPtr->SetMipmapType(J_GRAPHIC_MIP_MAP_TYPE::NONE);
					else if (userPtr != nullptr && creationDesc.textureDesc->mipMapDesc.type != J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT)
					{
						if (!SetCustomMipmap(userPtr, *creationDesc.textureDesc))
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
			bool MipmapBindForDebug(const JUserPtr<JGraphicResourceInfo>& gRInfo, _Inout_ std::vector<ResourceHandle>& gpuHandle, _Inout_ std::vector<Core::JDataHandle>& dataHandle)
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
			JUserPtr<JFrameUpdateInfo> CreateFrameUploadData(const JFrameUploadDataCreationDesc& desc)
			{
				return resourceManage.frame->Register(desc);
			}
			bool DestroyFrameUploadData(JFrameUpdateInfo* fInfo)
			{
				return resourceManage.frame->DeRegister(fInfo);
			}
		public:
			JUserPtr<JCullingInfo> CreateFrsutumCullingResultBuffer(const J_CULLING_TARGET target)
			{
				JCullingCreationDesc desc;
				if (target == J_CULLING_TARGET::RENDERITEM)
					desc.capacity = info.frame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT);
				else
					desc.capacity = info.frame.GetLocalLightCapacity();
				desc.target = target;
				//if graphic update전 초기화 단계일경우 update wait에 진입하자마자 info.frame.currIndex + 1이 되므로
				//미리  info.frame.currIndex + 1값을 할당한다.
				desc.currFrameIndex = IsEntryUpdateLoop() ? info.frame.currIndex : info.frame.currIndex + 1;
				desc.useGpu = target == J_CULLING_TARGET::LIGHT;
				auto res = resourceManage.culling->CreateFrsutumData(device.get(), desc);
				if (res != nullptr)
					alignedObject.aligned.push_back(JGameObjectBuffer::OpaqueVec());
				return res;
			}
			JUserPtr<JCullingInfo> CreateHzbCullingResultBuffer()
			{
				JCullingCreationDesc desc;
				desc.capacity = info.frame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::HZB_OCC_OBJECT);
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
						culling.hzb->NotifyBuildNewHzbOccBuffer(device.get(), desc.capacity, user);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						culling.hzb->NotifyBuildNewHzbOccBuffer(device.get(), desc.capacity, user);
				}
				return user;
			}
			JUserPtr<JCullingInfo> CreateHdCullingResultBuffer()
			{
				JCullingCreationDesc desc;
				desc.capacity = info.frame.GetCapacity(J_FRAME_RESOURCE_UPLOAD_TYPE::BOUNDING_OBJECT);
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
						culling.hd->NotifyBuildNewHdOccBuffer(device.get(), desc.capacity, user);
						device->EndPublicCommand();
						device->FlushCommandQueue();
					}
					else
						culling.hd->NotifyBuildNewHdOccBuffer(device.get(), desc.capacity, user);
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
				if (!IsActivated(J_GRAPHIC_OPTIONAL_FEATURE::GPU_ACCELERATOR))
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
			JUserPtr<JCsmHandlerInfo> CreateCsmHandler(JCsmHandleCreationDesc& desc)
			{
				return resourceManage.csm->CreateHandler(desc);
			}
			bool DestroyCsmHandler(JCsmHandlerInfo* handler)
			{
				return resourceManage.csm->DestroyHandler(handler);
			}
			JUserPtr<JCsmTargetInfo> CreateCsmTarget(JCsmTargetCreationDesc& desc)
			{
				return resourceManage.csm->CreateTarget(desc);
			}
			bool DestroyCsmTarget(JCsmTargetInfo* target)
			{
				return resourceManage.csm->DestroyTarget(target);
			}
		public:
			JOwnerPtr<JShaderDataHolder> CreateGraphicShader(const JGraphicShaderInitData& shaderData)
			{ 
				const bool isDeferred = shaderData.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY || 
					shaderData.processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING;
				if (isDeferred && !option.rendering.allowDeferred)
					return nullptr;

				device->FlushCommandQueue();
				device->StartPublicCommand();
				auto result = drawing.scene->CreateShader(JGraphicShaderCompileSet(device.get()), shaderData);
				device->EndPublicCommand();
				device->FlushCommandQueue();
				return std::move(result);
			}
			JOwnerPtr<JShaderDataHolder> CreateComputeShader(const JComputeShaderInitData& shaderData)
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
			void ReBuildFrameResource(const J_FRAME_RESOURCE_UPLOAD_TYPE type)
			{ 
				auto& uData = updateHelper.uData[(uint)type];
				resourceManage.frame->ReBuild(device.get(), type, CalculateCapacity(uData)); 
			}
		public:
			uint CalculateCapacity(const JUpdateHelper::UpdateDataBase& uBase)const noexcept
			{
				uint nextCapacity = uBase.capacity;
				if (uBase.reAllocCondition == J_GRAPHIC_CAPACITY_CONDITION::UP_CAPACITY)
				{
					while (nextCapacity <= uBase.count)
						nextCapacity *= uBase.upCapacityFactor;
				}
				else if (uBase.reAllocCondition == J_GRAPHIC_CAPACITY_CONDITION::DOWN_CAPACITY)
				{
					float result = nextCapacity / uBase.downCapacityFactor;
					while (result >= uBase.count && result >= info.minCapacity)
					{ 
						nextCapacity = result;
						result /= uBase.downCapacityFactor;
					}
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
				drawing.outline->UpdatePassBuf(info.width, info.height, Constants::commonStencilRef);

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
					resourceManage.frame.get(),
					drawing.debug.get(),
					drawing.depthTest.get(),
					imageProcessing.blur.get(),
					imageProcessing.downSampling.get(),
					imageProcessing.ssao.get(),
					imageProcessing.ppEffectSet.get(),
					resourceManage.shareData.get(),
					resourceManage.frame->GetCurrentFrameIndex(),
					resourceManage.frame->GetNextFrameIndex());
			}
		public:
			void UpdateWait()
			{	 
				resourceManage.frame->SetNextFrameResource();
				info.frame.currIndex = resourceManage.frame->GetCurrentFrameIndex(); 
				device->UpdateWait(resourceManage.frame->GetCurrentFrameResource()->GetFenceValue());

				AllocateRefSet();
				if (innerEvent.size() > 0)
				{
					for (const auto& data : innerEvent)
						data->InvokeCompletelyBind();
					innerEvent.clear();
				}

				adapter->BeginUpdateStart(option.deviceType, *drawRefSet);
			}
			void Update()
			{ 
				UpdateFrameBuffer();
				UpdateRequestor();
			}
			void UpdateReAllocCondition(JUpdateHelper::UpdateDataBase& uBase)const noexcept
			{
				uBase.reAllocCondition = J_GRAPHIC_CAPACITY_CONDITION::KEEP;
				if (uBase.capacity <= uBase.count)
				{
					uBase.reAllocCondition = J_GRAPHIC_CAPACITY_CONDITION::UP_CAPACITY;
					uBase.downCapacityCount = 0;
				}
				else if (uBase.count < (uBase.capacity / uBase.downCapacityFactor) && uBase.capacity > info.minCapacity)
				{
					if (uBase.downCapacityCount >= uBase.downCapacityCountMax)
					{
						uBase.reAllocCondition = J_GRAPHIC_CAPACITY_CONDITION::DOWN_CAPACITY;
						uBase.downCapacityCount = 0;
					}
					else
					{
						uBase.reAllocCondition = J_GRAPHIC_CAPACITY_CONDITION::KEEP;
						++uBase.downCapacityCount;
					}
				}
				else
					uBase.downCapacityCount = 0;
			}
		private:
			void UpdateFrameBuffer()
			{
				updateHelper.Clear();
				for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
				{
					const J_FRAME_RESOURCE_UPLOAD_TYPE type = (J_FRAME_RESOURCE_UPLOAD_TYPE)i;
					updateHelper.uData[i].count = resourceManage.frame->GetTotalFrameCount(type);
					updateHelper.uData[i].capacity = resourceManage.frame->GetFrameResourceCapacity(type);
					UpdateReAllocCondition(updateHelper.uData[i]);
					updateHelper.hasUploadDataDirty |= (bool)updateHelper.uData[i].reAllocCondition;
				}
				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					const J_GRAPHIC_RESOURCE_TYPE type = (J_GRAPHIC_RESOURCE_TYPE)i;
					updateHelper.bData[i].count = resourceManage.graphic->GetResourceCount(type);
					updateHelper.bData[i].capacity = info.resource.border[i];
					UpdateReAllocCondition(updateHelper.bData[i]);

					updateHelper.hasBindingDataDirty |= (bool)updateHelper.bData[i].reAllocCondition;
					if (updateHelper.bData[i].reAllocCondition != J_GRAPHIC_CAPACITY_CONDITION::KEEP)
						updateHelper.bData[i].capacity = CalculateCapacity(updateHelper.bData[i]);
				}

				JGraphicInfo newInfo = info;
				updateHelper.WriteGraphicInfo(newInfo);
				if (updateHelper.hasUploadDataDirty)
				{ 
					device->FlushCommandQueue();
					device->StartPublicCommand();
					for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
					{
						if (updateHelper.uData[i].reAllocCondition != J_GRAPHIC_CAPACITY_CONDITION::KEEP)
						{
							const J_FRAME_RESOURCE_UPLOAD_TYPE type = (J_FRAME_RESOURCE_UPLOAD_TYPE)i;
							ReBuildFrameResource(type);
							updateHelper.uData[i].capacity = resourceManage.frame->GetFrameResourceCapacity(type);
						}
					}
					 
					updateHelper.WriteGraphicInfo(newInfo);
					SetGraphicInfo(newInfo, true, updateHelper.hasBindingDataDirty, false);
					device->EndPublicCommand();
					device->FlushCommandQueue();
					//drawing.scene->RecompileShader(JGraphicShaderCompileSet(device.get()));		
					//use graphic info
					//updateHelper.NotifyUpdateFrameCapacity(*thisGraphic);	//use graphic info		
				}
				else if (updateHelper.hasBindingDataDirty)
					SetGraphicInfo(newInfo, false, true, true);
			}
			void UpdateRequestor()
			{
				updateHelper.Begin(); 
				resourceManage.frame->BeginUpdate();
				const uint drawListCount = JGraphicDrawList::GetListCount();
				//update frame resource and decide something drawing
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					drawTarget->BeginUpdate(); 

					JFrameUpdateOption updateOption; 
					if(option.debugging.testTrigger00)
						updateOption.setUpdateThreadTask = &GraphicThreadInteface::SetUpdateThreadTask;
					updateOption.isActivatedSceneTimer = drawTarget->scene->IsActivatedSceneTime();

					for (uint j = 0; j < totalCompVariation; ++j)
					{
						const JObjectDataSetMetadata meta = resourceManage.objectData->GetMetadata(j);
						if (!meta.isSupportedFrameResourceUpload)
							continue; 

						auto& compVec = drawTarget->scene->GetComponentCacheVec(j);

						JFrameUpdateDataSet updateSet(&compVec, meta, updateOption);
						//Update & Upload
						resourceManage.frame->Update(updateSet); 
						 
						drawTarget->updateInfo.log[j] = updateSet.updateLog;
					}
					UpdateSceneRequestor(drawTarget);
					UpdateShadowRequestor(drawTarget);
					UpdateFrustumCullingRequestor(drawTarget);
					UpdateOccCullingRequestor(drawTarget);
					drawTarget->EndUpdate();
				} 
				for (uint j = 0; j < totalResourceVariation; ++j)
				{
					const uint index = totalCompVariation + j;
					const JObjectDataSetMetadata meta = resourceManage.objectData->GetMetadata(index);
					if (!meta.isSupportedFrameResourceUpload)
						continue;

					const ObjectDataSetVec& dataVec = resourceManage.objectData->GetDataVec(index);
					JFrameUpdateOption option; 
					JFrameUpdateDataSet updateSet(&dataVec, meta, option);
					resourceManage.frame->Update(updateSet);
				}

				//동기화 시도
				resourceManage.frame->EndUpdate();
				updateHelper.End();
#ifdef USE_DEBUG
				//Debug
				//if(culling.hzb->CanReadBackDebugInfo())
				//	culling.hzb->StreamOutDebugInfo(JApplicationProject::LogPath() + L"\\Hzb.txt");
				//JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(0); 
				//resourceManage.culling->TryStreamOutCullingBuffer(resourceManage.culling->GetCullingInfo(J_CULLING_TYPE::HD_OCCLUSION, 
				//	drawTarget->scene->FindFirstSelectedCamera(false)->CullingUserInterface().GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM)).Get(),"LightCullignResult");
				//culling.lit->StreamOutDebugInfo(JApplicationProject::LogPath());
				//if (option.rendering.allowRaytracing)
				//	raytracing.ao->StreamOutDebugInfo();
#endif
			}
		private:   
			void UpdateSceneRequestor(_Inout_ JGraphicDrawTarget* target)
			{ 
				for (auto& data : target->sceneRequestor)
				{ 
					JFrameUpdateInterface* fInterface = static_cast<JFrameUpdateInterface*>(data->jCamera->ModuleManagedData()->GetFrameUpdateUserInterface());
					if (fInterface->IsLastUpdated())
						data->isUpdated = true;
				}
			}
			void UpdateShadowRequestor(_Inout_ JGraphicDrawTarget* target)
			{
				const bool allowLitCluster = target->scene->AllowLightCulling() &&
					option.culling.isLightCullingActivated &&
					option.culling.allowLightCluster &&
					target->shadowRequestor.size() > 0;
				    
				for (auto& data : target->shadowRequestor)
				{
					JLight* lit = data->jLight.Get(); 
					JFrameUpdateInterface* fInterface = static_cast<JFrameUpdateInterface*>(lit->ModuleManagedData()->GetFrameUpdateUserInterface());
					if (fInterface->IsLastUpdated())
						data->isUpdated = true;

					if (allowLitCluster && data->isUpdated)
					{
						const J_LIGHT_TYPE litType = lit->GetLightType(); 
						if (litType != J_LIGHT_TYPE::DIRECTIONAL)
						{
							bool isCull = true;
							auto& camVec = target->scene->GetComponentCacheVec(ConvertCompUniqueIndex<J_COMPONENT_TYPE::ENGINE_CAMERA>());
							const int frameIndex = fInterface->GetFrameIndex(JLightType::LitToFrameR(litType));

							for (const auto& cam : camVec)
							{
								auto cUser = cam->ModuleManagedData()->GetCullingUserInterface();
								isCull &= cUser->IsCulled(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::LIGHT, frameIndex);
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
				for (auto& data : target->frustumCullingRequestor)
				{ 
					auto fInterface = static_cast<JFrameUpdateInterface*>(data->comp->ModuleManagedData()->GetFrameUpdateUserInterface());
					if (fInterface->IsLastUpdated())
						data->isUpdated = true;
				}
			}
			void UpdateOccCullingRequestor(_Inout_ JGraphicDrawTarget* target)
			{ 
				for (auto& data : target->hzbOccCullingRequestor)
				{
					JComponent* comp = data->comp.Get();
					auto fInterface = static_cast<JFrameUpdateInterface*>(comp->ModuleManagedData()->GetFrameUpdateUserInterface());
					auto cInterface = static_cast<JCullingInterface*>(comp->ModuleManagedData()->GetCullingUserInterface());
					if (fInterface->IsLastUpdated())
						data->isUpdated = true;

					if(!cInterface->IsUpdateEnd(J_CULLING_TYPE::HZB_OCCLUSION, J_CULLING_TARGET::RENDERITEM)) 
						data->isUpdated = true;
				}
				for (auto& data : target->hdOccCullingRequestor)
				{
					JComponent* comp = data->comp.Get();
					auto fInterface = static_cast<JFrameUpdateInterface*>(comp->ModuleManagedData()->GetFrameUpdateUserInterface());
					auto cInterface = static_cast<JCullingInterface*>(comp->ModuleManagedData()->GetCullingUserInterface());
					if (fInterface->IsLastUpdated())
						data->isUpdated = true;

					if (!cInterface->IsUpdateEnd(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM))
						data->isUpdated = true;	 
				}
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
						//3.draw drawing.debug map
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

						drawing.debug->ComputeLitDebug(dataSet.debugCompute.get(),
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
					/*Restir이외에 Velocity buffer 사용시 use
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						drawing.scene->ComputeSceneDependencyTemporalResource(dataSet.sceneDraw.get(),
							JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}*/
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
					if (option.IsOcclusionActivated())
					{
						//1.query test
						//2.extract query result
						//3.draw drawing.debug map
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

						drawing.outline->DrawCamOutline(dataSet.outline.get(),
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

						drawing.debug->ComputeCamDebug(dataSet.debugCompute.get(),
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

				//작업분배
				for (uint i = 0; i < info.frame.threadCount; ++i)
					GraphicThreadInteface::SetDrawThreadTask(Core::JThreadInitInfo{}, UniqueBind(*workerFunctor, std::move(i)));

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
					helper.SetDrawTarget(drawTarget);

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

				auto& registeredSceneRequestor = JGraphicDrawList::GetRegisteredSceneDrawRequestor();
				auto& registeredShadowMapRequestor = JGraphicDrawList::GetRegisteredShadowMapDrawRequestor();
				auto& registeredFrustumCullingRequestor = JGraphicDrawList::GetRegisteredFrustumCullingRequestor();
				auto& registeredHzbOccCullingRequestor = JGraphicDrawList::GetRegisteredHzbOccCullingRequestor();
				auto& registeredHdOccCullingRequestor = JGraphicDrawList::GetRegisteredHdOccCullingRequestor();

				culling.lit->BindDrawResource(dataSet.bind.get());
				for (const auto& data : registeredSceneRequestor)
				{
					if (!data->canDrawThisFrame)
						continue;

					helper.SetDrawTarget(data->GetOwnerTarget());
					if (helper.scene->AllowLightCulling())
						culling.lit->ExecuteLightClusterTask(dataSet.litCulling.get(), JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
				}
				if (drawing.scene->HasPreprocessing())
				{
					for (const auto& data : registeredSceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						helper.SetDrawTarget(data->GetOwnerTarget());
						drawing.scene->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}
				if (drawing.shadowMap->HasPreprocessing())
				{
					for (const auto& data : registeredShadowMapRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						helper.SetDrawTarget(data->GetOwnerTarget());
						drawing.shadowMap->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
				}
				if (option.IsOcclusionActivated())
				{
					if (culling.hzb->HasPreprocessing())
					{
						for (const auto& data : registeredHzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							helper.SetDrawTarget(data->GetOwnerTarget());
							culling.hzb->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (culling.hd->HasPreprocessing())
					{
						for (const auto& data : registeredHdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							helper.SetDrawTarget(data->GetOwnerTarget());
							culling.hd->BeginDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
				}
				adapter->ExecuteBeginFrame(option.deviceType, *drawRefSet);
			}
			void MidFrame()
			{
				JGraphicMidFrameSet dataSet;
				adapter->SettingMidFrame(option.deviceType, *drawRefSet, dataSet);

				//mostly handle compute shader task or drawing drawing.debug
				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper(info, option, alignedObject);

				auto& registeredSceneRequestor = JGraphicDrawList::GetRegisteredSceneDrawRequestor();
				auto& registeredShadowMapRequestor = JGraphicDrawList::GetRegisteredShadowMapDrawRequestor();
				auto& registeredFrustumCullingRequestor = JGraphicDrawList::GetRegisteredFrustumCullingRequestor();
				auto& registeredHzbOccCullingRequestor = JGraphicDrawList::GetRegisteredHzbOccCullingRequestor();
				auto& registeredHdOccCullingRequestor = JGraphicDrawList::GetRegisteredHdOccCullingRequestor();

				/*
				if (option.CanUsePostProcess() || option.CanUseRtGi())
				{
					for (const auto& data : registeredSceneRequestor)
					{
						helper.SetDrawTarget(data->GetOwnerTarget());
						if (!data->canDrawThisFrame)
							continue;

						drawing.velocity->Compute(dataSet.velocity.get(), JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}
				*/
				if (option.CanUseSSAO() || option.CanUseRtGi())
				{
					for (const auto& data : registeredSceneRequestor)
					{
						helper.SetDrawTarget(data->GetOwnerTarget());
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						imageProcessing.ssao->ApplySsao(dataSet.ssao.get(), copiedHelper);

						if (!data->jCamera->AllowRaytracingGI())
							continue;

						raytracing.gi->ComputeGI(dataSet.rtgi.get(), copiedHelper);
						raytracing.denoiser->ApplyGIDenoise(dataSet.rtDenoiser.get(), copiedHelper);
					}
				}
				if (option.rendering.allowDeferred)
				{
					drawing.scene->BindResource(J_GRAPHIC_RENDERING_PROCESS::DEFERRED_SHADING, dataSet.bind.get());
					for (const auto& data : registeredSceneRequestor)
					{
						helper.SetDrawTarget(data->GetOwnerTarget());
						if (!data->canDrawThisFrame)
							continue;

						drawing.scene->DrawSceneShadeMultiThread(dataSet.sceneDraw.get(), JDrawHelper::CreateDrawSceneHelper(helper, data->jCamera));
					}
				}

				drawing.scene->BindResource(J_GRAPHIC_RENDERING_PROCESS::FORWARD, dataSet.bind.get());
				if (option.IsOcclusionActivated())
				{
					if (culling.hzb->HasPostprocessing())
					{
						for (const auto& data : registeredHzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							helper.SetDrawTarget(data->GetOwnerTarget());
							culling.hzb->EndDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
					if (culling.hd->HasPostprocessing())
					{
						for (const auto& data : registeredHdOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							helper.SetDrawTarget(data->GetOwnerTarget());
							culling.hd->EndDraw(dataSet.bind.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
						}
					}
				}
				if (drawing.shadowMap->HasPostprocessing())
				{
					for (const auto& data : registeredShadowMapRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						helper.SetDrawTarget(data->GetOwnerTarget());
						drawing.shadowMap->EndDraw(dataSet.bind.get(), JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
					}
				}
				for (const auto& data : registeredSceneRequestor)
				{
					if (!data->canDrawThisFrame)
						continue;

					helper.SetDrawTarget(data->GetOwnerTarget());

					JDrawHelper copiedHelper = helper;
					copiedHelper.SettingDrawScene(data->jCamera);
					if (copiedHelper.allowDrawDebugObject)
						drawing.scene->DrawSceneDebugUIMultiThread(dataSet.sceneDraw.get(), copiedHelper);
					drawing.scene->EndDraw(dataSet.bind.get(), copiedHelper);
				}

				//after Scene EndDraw
				if (option.IsOcclusionActivated())
				{
					for (const auto& data : registeredHzbOccCullingRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						helper.SetDrawTarget(data->GetOwnerTarget());
						culling.hzb->ComputeOcclusionCulling(dataSet.hzbCompute.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
					}
					for (const auto& data : registeredHdOccCullingRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						helper.SetDrawTarget(data->GetOwnerTarget());
						culling.hd->ExtractHDOcclusionCullingData(dataSet.hdExtract.get(), JDrawHelper::CreateOccCullingHelper(helper, data->comp));
					}
				}
				for (const auto& data : registeredShadowMapRequestor)
				{
					if (!data->canDrawThisFrame)
						continue;

					helper.SetDrawTarget(data->GetOwnerTarget());
					drawing.debug->ComputeLitDebug(dataSet.debugCompute.get(), JDrawHelper::CreateDrawShadowMapHelper(helper, data->jLight));
				}
				for (const auto& data : registeredSceneRequestor)
				{
					if (!data->canDrawThisFrame)
						continue;

					helper.SetDrawTarget(data->GetOwnerTarget());

					JDrawHelper copiedHelper = helper;
					copiedHelper.SettingDrawScene(data->jCamera);
					if (data->jCamera->AllowPostProcess())
						imageProcessing.ppPipeline->ApplyPostProcess(dataSet.postPrcess.get(), copiedHelper, data->canDrawThisFrame);

					drawing.debug->ComputeCamDebug(dataSet.debugCompute.get(), copiedHelper);
					drawing.outline->DrawCamOutline(dataSet.outline.get(), copiedHelper);
				}

				if (option.debugging.allowDisplayLightCullingResult)
				{
					culling.lit->BindDebugResource(dataSet.bind.get());
					for (const auto& data : registeredSceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						culling.lit->ExecuteLightClusterDebug(dataSet.litCullingDebug.get(), JDrawHelper::CreateLitCullingHelper(helper, data->jCamera));
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
						resourceManage.frame.get(),
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
				JGraphicSubClassShareData shareData(pushEvLam);
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

				SetGraphicInfo(newinfo, true, true, true);
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
						auto gUser = firstCam->ModuleManagedData()->GetGraphicResourceUserInterface();
						const int index = gUser->GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
						resourceManage.graphic->StoreTexture(device.get(), J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, index, opendInfo->lastRsPath());
					}
				}
			}
		public: 
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
		};
		class JGraphic::JGraphicModuleImpl : public Rule::JGraphicModuleInterface
		{
		private:
			JGraphicImpl* impl = nullptr;
		public:
			JGraphicModuleImpl(JGraphic::JGraphicImpl* impl)
				:impl(impl)
			{

			}
		public:
			JFastPtr<JGraphicModuleManagedDataFrame> Allocate(const JGraphicModuleManagedDataCreationDesc& desc) final
			{
				return impl->resourceManage.objectData->Add(desc);
			}
			void DeAllocate(JFastPtr<JGraphicModuleManagedDataFrame>& data)final
			{
				impl->resourceManage.objectData->Remove(data);
			}
		public:
			bool RegisterScene(JGraphicModuleManagedDataFrame* data, const JGraphicSceneRegisterDesc& desc)final
			{
				if (data == nullptr)
					return false;

				JUserPtr<JScene> scene = Core::ConnectChildUserPtr<JScene>(data->Object());
				if (scene == nullptr)
					return false;

				return JGraphicDrawList::AddDrawList(scene);
			}
			bool DeRegisterScene(JGraphicModuleManagedDataFrame* data) final
			{
				if (data == nullptr)
					return false;

				JUserPtr<JScene> scene = Core::ConnectChildUserPtr<JScene>(data->Object());
				if (scene == nullptr)
					return false;

				return JGraphicDrawList::PopDrawList(scene);
			}
		public:
			//Owner scene had to register before request
			bool RequestExecutableGraphicFeature(JGraphicModuleManagedDataFrame* data, const JGraphicRequestCreationDesc& desc)final
			{
				if (data == nullptr)
					return false;

				JUserPtr<JComponent> comp = Core::ConnectChildUserPtr<JComponent>(data->Object());
				if (comp == nullptr)
					return false;

				JUserPtr<JScene> scene = comp->GetOwner()->GetOwnerScene();
				switch (desc.type)
				{
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::DRAW_SCENE:
					return JGraphicDrawList::AddDrawSceneRequest(scene, Core::ConnectChildUserPtr<JCamera>(comp), desc.frequency);
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::DRAW_SHADOW_MAP:
					return JGraphicDrawList::AddDrawShadowRequest(scene, Core::ConnectChildUserPtr<JLight>(comp));
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING:
					return JGraphicDrawList::AddFrustumCullingRequest(scene, comp, desc.frequency);
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::HZB_OCCLUSION_CULLING:
					return JGraphicDrawList::AddHzbOccCullingRequest(scene, comp, desc.frequency);
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::HARD_WARE_OCCLUSION_CULLING:
					return JGraphicDrawList::AddHdOccCullingRequest(scene, comp, desc.frequency);
				default:
					break;
				}
				return false;
			}
			bool CancelExecutableGraphicFeature(JGraphicModuleManagedDataFrame* data, J_GRAPHIC_REQUEST_TYPE type) final
			{
				if (data == nullptr)
					return false;

				JUserPtr<JComponent> comp = Core::ConnectChildUserPtr<JComponent>(data->Object());
				if (comp == nullptr)
					return false;

				JUserPtr<JScene> scene = comp->GetOwner()->GetOwnerScene();
				switch (type)
				{
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::DRAW_SCENE:
					return JGraphicDrawList::PopDrawSceneRequest(scene, Core::ConnectChildUserPtr<JCamera>(comp));
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::DRAW_SHADOW_MAP:
					return JGraphicDrawList::PopDrawShadowRequest(scene, Core::ConnectChildUserPtr<JLight>(comp));
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::FRUSTUM_CULLING:
					return JGraphicDrawList::PopFrustumCullingRequest(scene, comp);
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::HZB_OCCLUSION_CULLING:
					return JGraphicDrawList::PopHzbOccCullingRequest(scene, comp);
				case JinEngine::J_GRAPHIC_REQUEST_TYPE::HARD_WARE_OCCLUSION_CULLING:
					return JGraphicDrawList::PopHdOccCullingRequest(scene, comp);
				default:
					break;
				}
				return false;
			}
		public:
			bool CreateGraphicResource(JGraphicModuleManagedDataFrame* data, const JGraphicResourceCreationDesc& desc, const uint count = 1)final
			{  
				JGraphicObjectDataSetBase* base = static_cast<JGraphicObjectDataSetBase*>(data);
				if (count == 0 || base == nullptr)
					return false;

				auto gInterface = static_cast<JGraphicResourceInterface*>(base->GetGraphicResourceInterface());
				if (gInterface == nullptr)
					return false;

				uint successCount = 0;
				for (uint i = 0; i < count; ++i)
				{
					int nextIndex = gInterface->NextResourceIndex(desc.type.resouce, desc.type.task);
					if (nextIndex == invalidIndex)
						continue;

					JUserPtr<JGraphicResourceInfo> newInfo = impl->CreateResource(desc);
					if (newInfo == nullptr)
						continue;
					   
					gInterface->AddInfo(newInfo, nextIndex);
					++successCount;

					impl->resourceManage.objectData->NotifyGraphicResourceCreation(base, newInfo, desc.type.task);
				}  
				return successCount > 0;
			}
			bool DestroyGraphicResource(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint localIndex = 0, const uint count = 1)final
			{
				if (count == 0)
					return false;

				auto gInterface = static_cast<JGraphicResourceInterface*>(data->GetGraphicResourceUserInterface());
				if (gInterface == nullptr)
					return false;

				JGraphicResourceInterface::DestoryInfoF destroyF = CreateDestroyGraphicResourceF();
				for(uint i = 0; i < count; ++i)
					gInterface->RemoveInfo(destroyF, typeSet.resouce, typeSet.task, localIndex);
				return true;
			}
			bool DestroyAllGraphicsResourcesOfType(JGraphicModuleManagedDataFrame* data, const J_GRAPHIC_RESOURCE_TYPE type) final
			{
				auto gInterface = static_cast<JGraphicResourceInterface*>(data->GetGraphicResourceUserInterface());
				if (gInterface == nullptr)
					return false;

				JGraphicResourceInterface::DestoryInfoF destroyF = CreateDestroyGraphicResourceF();
				gInterface->RemoveInfoOfType(destroyF, type);
				return true;
			}
			bool DestroyAllGraphicsResources(JGraphicModuleManagedDataFrame* data) final
			{
				auto gInterface = static_cast<JGraphicResourceInterface*>(data->GetGraphicResourceUserInterface());
				if (gInterface == nullptr)
					return false;

				JGraphicResourceInterface::DestoryInfoF destroyF = CreateDestroyGraphicResourceF();
				gInterface->RemoveInfoAll(destroyF);
				return true;
			}
		public:
			bool CreateGraphicResourceOption(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint localIndex = 0) final
			{
				JUserPtr<JGraphicResourceInfo> existInfo = GetGraphicResourceInfo(data, typeSet, localIndex);
				return existInfo != nullptr ? impl->CreateOption(existInfo, typeSet.option) : false;
			}
			bool DestroyGraphicResourceOption(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint localIndex = 0) final
			{
				JUserPtr<JGraphicResourceInfo> existInfo = GetGraphicResourceInfo(data, typeSet, localIndex);
				return existInfo != nullptr ? impl->DestroyGraphicOption(existInfo, typeSet.option) : false;
			}
		public:
			bool CreateFrameUploadData(JGraphicModuleManagedDataFrame* data, const JFrameUploadDataCreationDesc& desc) final
			{
				auto fInterface = static_cast<JFrameUpdateInterface*>(data->GetFrameUpdateUserInterface());
				if (fInterface == nullptr || !fInterface->HasSpace(desc.type))
					return false;

				auto newUser = impl->CreateFrameUploadData(desc);
				if (newUser == nullptr)
					return false;

				return fInterface->Add(newUser);
			}
			bool DestroyFrameUploadData(JGraphicModuleManagedDataFrame* data, const J_FRAME_RESOURCE_UPLOAD_TYPE type) final
			{
				auto fInterface = static_cast<JFrameUpdateInterface*>(data->GetFrameUpdateUserInterface());
				if (fInterface == nullptr)
					return false;

				auto existUser = fInterface->Release(type);
				if (existUser == nullptr)
					return false;

				return impl->DestroyFrameUploadData(existUser);
			}
		public:
			bool CreateCullingData(JGraphicModuleManagedDataFrame* data, const JCullingTypeSet& typeSet) final
			{
				auto cInterface = static_cast<JCullingInterface*>(data->GetCullingUserInterface());
				if (cInterface == nullptr || !cInterface->HasSpace(typeSet.type, typeSet.target))
					return false;

				JUserPtr<JCullingInfo> info;
				switch (typeSet.type)
				{
				case JinEngine::J_CULLING_TYPE::FRUSTUM:
				{
					info = _JGraphic::Instance().impl->CreateFrsutumCullingResultBuffer(typeSet.target);
					break;
				}
				case JinEngine::J_CULLING_TYPE::HZB_OCCLUSION:
				{
					if (typeSet.target != J_CULLING_TARGET::RENDERITEM)
						return false;

					info = _JGraphic::Instance().impl->CreateHzbCullingResultBuffer();
					break;
				}
				case JinEngine::J_CULLING_TYPE::HD_OCCLUSION:
				{
					if (typeSet.target != J_CULLING_TARGET::RENDERITEM)
						return false;

					info = _JGraphic::Instance().impl->CreateHdCullingResultBuffer();
					break;
				}
				default:
					return false;
				}
				cInterface->AddInfo(info);
				return true;
			}
			bool DestroyCullingData(JGraphicModuleManagedDataFrame* data, const JCullingTypeSet& typeSet) final
			{
				auto cInterface = static_cast<JCullingInterface*>(data->GetCullingUserInterface());
				if (cInterface == nullptr)
					return false;
				 
				JCullingInterface::DestoryInfoF destroyF = CreateDestroyCullingF();
				cInterface->RemoveInfo(destroyF, typeSet.type, typeSet.target);
				return true;
			}
			bool DestroyAllCullingDataOfType(JGraphicModuleManagedDataFrame* data, const J_CULLING_TYPE type)final
			{
				auto cInterface = static_cast<JCullingInterface*>(data->GetCullingUserInterface());
				if (cInterface == nullptr)
					return false;

				JCullingInterface::DestoryInfoF destroyF = CreateDestroyCullingF();
				cInterface->RemoveInfoOfType(destroyF, type);
				return true;
			}
			bool DestroyAllCullingData(JGraphicModuleManagedDataFrame* data) final
			{
				auto cInterface = static_cast<JCullingInterface*>(data->GetCullingUserInterface());
				if (cInterface == nullptr)
					return false;

				JCullingInterface::DestoryInfoF destroyF = CreateDestroyCullingF();
				cInterface->RemoveInfoAll(destroyF);
				return true;
			}
		public:
			bool CreateGpuAccelerator(JGraphicModuleManagedDataFrame* data, const JGpuAcceleratorBuildDesc& desc) final
			{
				auto gInterface = static_cast<JGpuAcceleratorInterface*>(data->GetGpuAcceleratorUserInterface());
				if (gInterface == nullptr)
					return false;

				auto info = _JGraphic::Instance().impl->CreateGpuAccelerator(desc);
				if (info == nullptr)
					return false;

				gInterface->AddInfo(info);
				return true;
			}
			bool DestroyGpuAccelerator(JGraphicModuleManagedDataFrame* data) final
			{
				auto gInterface = static_cast<JGpuAcceleratorInterface*>(data->GetGpuAcceleratorUserInterface());
				if (gInterface == nullptr)
					return false;

				auto existInfo = gInterface->ReleaseInfo();
				return _JGraphic::Instance().impl->DestroyGpuAccelerator(existInfo);
			}
		public:
			bool CreateCsmHandler(JGraphicModuleManagedDataFrame* data, JCsmHandleCreationDesc& desc) final
			{
				auto cInterface = static_cast<JCsmHandlerInterface*>(data->GetCsmHandleUserInterface());
				if (cInterface == nullptr)
					return false;

				auto info = _JGraphic::Instance().impl->CreateCsmHandler(desc);
				if (info == nullptr)
					return false;

				cInterface->AddInfo(info);
				return true;
			}
			bool CreateCsmTarget(JGraphicModuleManagedDataFrame* data, JCsmTargetCreationDesc& desc) final
			{
				auto cInterface = static_cast<JCsmTargetInterface*>(data->GetCsmTargetUserInterface());
				if (cInterface == nullptr)
					return false;

				auto info = _JGraphic::Instance().impl->CreateCsmTarget(desc);
				if (info == nullptr)
					return false;

				cInterface->AddInfo(info);
				return true;
			}
			bool DestroyCsmHandler(JGraphicModuleManagedDataFrame* data) final
			{
				auto cInterface = static_cast<JCsmHandlerInterface*>(data->GetCsmHandleUserInterface());
				if (cInterface == nullptr)
					return false;

				return _JGraphic::Instance().impl->DestroyCsmHandler(cInterface->Release());
			}
			bool DestroyCsmTarget(JGraphicModuleManagedDataFrame* data) final
			{
				auto cInterface = static_cast<JCsmTargetInterface*>(data->GetCsmTargetUserInterface());
				if (cInterface == nullptr)
					return false;
				 
				return _JGraphic::Instance().impl->DestroyCsmTarget(cInterface->Release());
			}
		public:
			//Shader isn't use interface 
			//because less features to be provided as an interface 
			JOwnerPtr<JShaderDataHolder> CreateGraphicShader(const JGraphicShaderInitData& initData) final
			{
				return _JGraphic::Instance().impl->CreateGraphicShader(initData);
			}
			JOwnerPtr<JShaderDataHolder> CreateComputeShader(const JComputeShaderInitData& initData) final
			{
				return _JGraphic::Instance().impl->CreateComputeShader(initData);
			} 
		public:
			bool SetMipmap(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint dataIndex, JTextureCreationDesc& creationDesc)final
			{
				auto impl = _JGraphic::Instance().impl.get();
				auto info = GetGraphicResourceInfo(data, typeSet, dataIndex);
				return info != nullptr ? impl->SetCustomMipmap(info, creationDesc) : false;
			}
			bool SetTextureDetail(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint dataIndex, const JConvertColorDesc& convertDesc)final
			{
				auto impl = _JGraphic::Instance().impl.get();
				auto info = GetGraphicResourceInfo(data, typeSet, dataIndex);
				return info != nullptr ? impl->SetTextureDetail(info, convertDesc) : false;
			}
			bool TryFirstGraphicResourceMipmapBind(JGraphicModuleManagedDataFrame* data, _Inout_ std::vector<ResourceHandle>& gpuHandle, _Inout_ std::vector<Core::JDataHandle>& dataHandle)const final
			{ 
				auto gInterface = static_cast<JGraphicResourceInterface*>(data->GetGraphicResourceUserInterface());
				if (gInterface == nullptr)
					return false;
#ifdef DEVELOP 
				return _JGraphic::Instance().impl->MipmapBindForDebug(gInterface->GetFirstGraphicInfo(), gpuHandle, dataHandle);
#else
				return false;
#endif
			}
			void ClearFirstGraphicResourceMipmapBind(JGraphicModuleManagedDataFrame* data, _Inout_ std::vector<Core::JDataHandle>& dataHandle)
			{
				auto gInterface = static_cast<JGraphicResourceInterface*>(data->GetGraphicResourceUserInterface());
				if (gInterface == nullptr)
					return;

				_JGraphic::Instance().impl->ClearMipmapBind(dataHandle);
			}
		public:
			void UpdateTransform(JGpuAcceleratorUserInterface* gUser, const JUserPtr<JComponent>& comp)noexcept
			{
				auto info = static_cast<JGpuAcceleratorInterface*>(gUser)->GetInfo();
				if (info == nullptr)
					return;

				_JGraphic::Instance().impl->UpdateTransform(info.Get(), comp);
			}
			void AddComponent(JGpuAcceleratorUserInterface* gUser, const JUserPtr<JComponent>& newComp)noexcept
			{
				auto info = static_cast<JGpuAcceleratorInterface*>(gUser)->GetInfo();
				if (info == nullptr)
					return;

				_JGraphic::Instance().impl->AddComponent(info.Get(), newComp);
			}
			void RemoveComponent(JGpuAcceleratorUserInterface* gUser, const JUserPtr<JComponent>& comp)noexcept
			{
				auto info = static_cast<JGpuAcceleratorInterface*>(gUser)->GetInfo();
				if (info == nullptr)
					return;

				_JGraphic::Instance().impl->RemoveComponent(info.Get(), comp);
			}
		public:
			//Return whether it can be implemented with the current graphical api device
			bool IsSupported(const J_GRAPHIC_OPTIONAL_FEATURE featureType)const noexcept
			{
				return impl->IsSupported(featureType);
			}
			bool IsActivated(const J_GRAPHIC_OPTIONAL_FEATURE featureType)const noexcept
			{
				return impl->IsActivated(featureType);
			}
		private:
			JUserPtr<JGraphicResourceInfo> GetGraphicResourceInfo(JGraphicModuleManagedDataFrame* data, const JGraphicResourceTypeSet& typeSet, const uint localIndex)
			{
				if (data == nullptr)
					return nullptr;

				auto gInterface = static_cast<JGraphicResourceInterface*>(data->GetGraphicResourceUserInterface());
				if (gInterface == nullptr)
					return nullptr;

				return gInterface->GetGraphicInfo(typeSet.resouce, typeSet.task, localIndex);
			}
			JGraphicResourceInterface::DestoryInfoF CreateDestroyGraphicResourceF()
			{
				auto destroyLam = [](JGraphicResourceInfo* info)
				{
					if (info == nullptr)
						return;

					_JGraphic::Instance().impl->DestroyGraphicTextureResource(info);
				};
				return JGraphicResourceInterface::DestoryInfoF(destroyLam);
			}
			JCullingInterface::DestoryInfoF CreateDestroyCullingF()
			{
				auto destroyLam = [](JCullingInfo* info)
				{
					if (info == nullptr)
						return;

					_JGraphic::Instance().impl->DestroyCullignData(info);
				};
				return JCullingInterface::DestoryInfoF(destroyLam);
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
		JGraphic::JGraphic()
			:impl(std::make_unique<JGraphicImpl>(Core::MakeGuid(), this)),
			moduleImpl(std::make_unique<JGraphicModuleImpl>(impl.get()))
		{
			JGraphicModuleInterfaceHolder::Instance().Set(moduleImpl.get());
		}
		JGraphic::~JGraphic()
		{
			JGraphicModuleInterfaceHolder::Instance().Set(nullptr);
			impl.reset();
			moduleImpl.reset();
		}
		 
		using DebugInterface = JGraphicPrivate::DebugInterface;
		using MainAccess = JGraphicPrivate::MainAccess;
   
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
		void MainAccess::Update()
		{ 
			JinEngine::JGraphic::Instance().impl->Update();
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