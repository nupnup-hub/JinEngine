#pragma once
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include"JGraphic.h" 
#include"JGraphicPrivate.h" 
#include"JGraphicConstants.h"
#include"JGraphicDrawList.h" 
#include"JGraphicUpdateHelper.h"
#include"GraphicResource/JGraphicResourceManager.h"
#include"GraphicResource/JGraphicResourceInfo.h"
#include"GraphicResource/JGraphicResourceInterface.h"
#include"ShadowMap/JShadowMap.h"
#include"Utility/JDepthMapDebug.h"
#include"Culling/JCullingInfo.h"
#include"Culling/JCullingManager.h"
#include"Culling/Frustum/JFrustumCulling.h"
#include"Culling/Occlusion/JHardwareOccCulling.h"
#include"Culling/Occlusion/JHZBOccCulling.h"
#include"Outline/JOutline.h"

#include"../Window/JWindow.h"
#include"../Window/JWindowPrivate.h"
#include"Upload/FrameResource/JFrameResource.h" 
#include"Upload/FrameResource/JObjectConstants.h" 
#include"Upload/FrameResource/JAnimationConstants.h" 
#include"Upload/FrameResource/JMaterialConstants.h" 
#include"Upload/FrameResource/JPassConstants.h" 
#include"Upload/FrameResource/JCameraConstants.h" 
#include"Upload/FrameResource/JLightConstants.h" 
#include"Upload/FrameResource/JShadowMapConstants.h" 
#include"Upload/FrameResource/JBoundingObjectConstants.h"
#include"Upload/FrameResource/JOcclusionConstants.h"

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
#include"../Object/Resource/Shader/JShaderData.h"
#include"../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../Object/Resource/Scene/JSceneManager.h"
#include"../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"
#include"../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../Object/Resource/Texture/JTexture.h" 

#include"../Application/JApplicationEngine.h"
#include"../Application/JApplicationEnginePrivate.h"
#include"../Application/JApplicationProject.h"
#include"../Utility/JCommonUtility.h" 
#include"../Editor/GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include"../../ThirdParty/imgui/imgui_impl_dx12.h"
#include"../../ThirdParty/imgui/imgui_impl_win32.h"
#include"../../ThirdParty/DirectX/TK/Inc/ScreenGrab.h"

#include<DirectXColors.h>
#include<functional>
#include <dxgidebug.h>
#include<WindowsX.h>
#include<wrl/client.h>
#include<memory>
#include<dxgi1_4.h>
#include<d3d12.h>
#include<vector>
#include<unordered_map>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#define DEBUG 
// 
//#include"../Core/File/JFileIOHelper.h"

namespace JinEngine
{
	using namespace DirectX;
	namespace Graphic
	{
		namespace
		{
			using CamEditorSettingInterface = JCameraPrivate::EditorSettingInterface;
			using GraphicThreadInteface = Core::JThreadManagerPrivate::GraphicInterface;
		} 
		namespace
		{ 
		}
		class JDrawHandle
		{
		public:
			//HANDLE begin;
			HANDLE endHzbDrawDepthMap[Constants::gMaxFrameThread];
			HANDLE endDrawShadwMap[Constants::gMaxFrameThread];
			HANDLE endDrawScene[Constants::gMaxFrameThread];
		private:
			uint handleCount;
		public:
			void Initialize(const uint threadCount)
			{
				//begin = CreateEvent(NULL, FALSE, FALSE, NULL);
				for (uint i = 0; i < threadCount; ++i)
				{
					endHzbDrawDepthMap[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
					endDrawShadwMap[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
					endDrawScene[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
				}
				handleCount = threadCount;
			}
			void Clear()
			{
				for (uint i = 0; i < handleCount; ++i)
				{
					CloseHandle(endHzbDrawDepthMap[i]);
					CloseHandle(endDrawShadwMap[i]);
					CloseHandle(endDrawScene[i]);
				}
			}
		};

		//나중에 할일
		//멀티스레드 함수 이름 변경
		//멀티스레드 테스트
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
			// Set true to use 4X MSAA (?.1.8).  The default is false.
			uint      m4xMsaaQuality = 0;      // quality level of 4X MSAA
			bool      m4xMsaaState = false;    // 4X MSAA enabled
		public:
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> publicCmdListAlloc;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> publicCmdList;
		public:
			Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
			Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
			Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
			Microsoft::WRL::ComPtr<ID3D12Fence> fence;
			UINT64 currentFence = 0;
			D3D12_VIEWPORT screenViewport;
			D3D12_RECT scissorRect;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			int currBackBuffer = 0;
		public:
			bool stCommand = false;
		public:
			JGraphicInfo info;
			JGraphicOption option;
			JUpdateHelper updateHelper;
			JDrawHandle handle;
		public:
			std::unique_ptr<JGraphicResourceManager> graphicResourceM;
			std::unique_ptr<JShadowMap> shadowMap;
			std::unique_ptr<JCullingManager> cullingM;
			std::unique_ptr<JFrustumCulling> frustumHelper;
			std::unique_ptr<JHardwareOccCulling> hdOccHelper;
			std::unique_ptr<JHZBOccCulling> hzbOccHelper;
			std::unique_ptr<JDepthMapDebug> depthMapDebug;
			std::unique_ptr<JOutline> outlineHelper;
		public:
			JUserPtr<JGraphicResourceInfo> defaultSceneDsInfo;
		public:
			std::unique_ptr<WorkerThreadF::Functor> workerFunctor;
		public:
			JGraphicImpl(const size_t guid, JGraphic* thisGraphic)
				:guid(guid), thisGraphic(thisGraphic)
			{ 
				const uint occMipMapViewCapa = JGraphicResourceManager::GetOcclusionMipMapViewCapacity();
				info.occlusionWidth = std::pow(2, occMipMapViewCapa - 1);
				info.occlusionHeight = std::pow(2, occMipMapViewCapa - 1);
				info.occlusionMinSize = JGraphicResourceManager::GetOcclusionMinSize();
				info.occlusionMapCapacity = occMipMapViewCapa;
				info.occlusionMapCount = JMathHelper::Log2Int(info.occlusionWidth) - JMathHelper::Log2Int(JGraphicResourceManager::GetOcclusionMinSize()) + 1;
				info.frameThreadCount = _JThreadManager::Instance().GetReservedSpaceCount(Core::J_THREAD_USE_CASE_TYPE::GRAPHIC_DRAW);

				updateHelper.uData.resize((int)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT);
				updateHelper.bData.resize((int)J_GRAPHIC_RESOURCE_TYPE::COUNT);

				auto objGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetMeshCount();
					return sum;
				};
				auto passGetElemenLam = []()->uint {return 1; };
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
				auto litGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
					return sum;
				};
				auto litIndexGetElementLam = []() {return JGraphicDrawList::GetListCount(); };
				auto shadowLitGetElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->shadowRequestor.size();
					return sum;
				};
				auto shadowMapElementLam = []()
				{
					uint sum = 0;
					const uint drawListCount = JGraphicDrawList::GetListCount();
					for (uint i = 0; i < drawListCount; ++i)
						sum += (uint)JGraphicDrawList::GetDrawScene(i)->shadowRequestor.size();
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

				using GetElementCount = JUpdateHelper::GetElementCountT::Ptr;
				std::unordered_map<J_UPLOAD_FRAME_RESOURCE_TYPE, GetElementCount> uGetCountFunc
				{
					{J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, objGetElementLam}, {J_UPLOAD_FRAME_RESOURCE_TYPE::PASS, passGetElemenLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, aniGetElementLam},{J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, camGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT, litGetElementLam},	{J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_INDEX, litIndexGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT, shadowLitGetElementLam},{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP, shadowMapElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, materialGetElementLam},{J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, boundObjGetElementLam},
					{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_REQUESTOR, hzbRequestorGetElementLam}, 	{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, hzbObjectGetElementLam}
				};

				using NotifyUpdateCapacity = JUpdateHelper::NotifyUpdateCapacityT::Callable;
				auto updateOccObjCapaLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					const uint newCapa = impl->currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT);

					impl->hzbOccHelper->ReBuildObjectConstants(impl->d3dDevice.Get(), newCapa);
					impl->hzbOccHelper->ReBuildOccBuffer(impl->d3dDevice.Get(), newCapa, impl->cullingM->GetCullingInfoVec(J_CULLING_TYPE::OCCLUSION));
				};
				auto updateFrustumCullingResultVecLam = []()
				{
					JGraphic::JGraphicImpl* impl = _JGraphic::Instance().impl.get();
					const uint newCapa = impl->currFrameResource->GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT);

					impl->frustumHelper->ReBuildResultBuffer(newCapa, impl->cullingM->GetCullingInfoVec(J_CULLING_TYPE::FRUSTUM));
				};

				for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				{
					J_UPLOAD_FRAME_RESOURCE_TYPE type = (J_UPLOAD_FRAME_RESOURCE_TYPE)i;
					updateHelper.RegisterCallable(type, &uGetCountFunc.find(type)->second);
				}
				updateHelper.RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, std::make_unique<NotifyUpdateCapacity>(updateFrustumCullingResultVecLam));
				updateHelper.RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, std::make_unique<NotifyUpdateCapacity>(updateOccObjCapaLam));

				auto texture2DGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D); };
				auto cubeMapGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE); };
				auto shadowMapGetCountLam = []() {return _JGraphic::Instance().impl->graphicResourceM->GetResourceCount(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP); };

				auto texture2DGetCapacityLam = []() {return _JGraphic::Instance().impl->info.binding2DTextureCapacity; };
				auto cubeMapGetCapacityLam = []() {return _JGraphic::Instance().impl->info.bindingCubeMapCapacity; };
				auto shadowMapGetCapacityLam = []() {return _JGraphic::Instance().impl->info.bindingShadowTextureCapacity; };

				auto texture2DSetCapaLam = []()
				{
					JGraphic& graphic = _JGraphic::Instance();
					graphic.impl->SetInfoCapacity(graphic.impl->info.binding2DTextureCapacity,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].count,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].recompileCondition);
				};
				auto cubeMapeSetCapaLam = []()
				{
					JGraphic& graphic = _JGraphic::Instance();
					graphic.impl->SetInfoCapacity(graphic.impl->info.bindingCubeMapCapacity,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].count,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].recompileCondition);
				};
				auto shadowMapSetCapaLam = []()
				{
					JGraphic& graphic = _JGraphic::Instance();
					graphic.impl->SetInfoCapacity(graphic.impl->info.bindingShadowTextureCapacity,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].count,
						graphic.impl->updateHelper.bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].recompileCondition);
				};

				using BindTextureGetCount = JUpdateHelper::GetElementCountT::Ptr;
				using BindTextureGetCapacity = JUpdateHelper::GetElementCapacityT::Ptr;
				using BindTextureSetCapacity = JUpdateHelper::SetCapacityT::Ptr;

				//recompile shader for change texture array capacity	 
				std::unordered_map <J_GRAPHIC_RESOURCE_TYPE, bool> hasCallable
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, true},
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, true},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, true}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureGetCount> bindTextureGetCountFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DGetCountLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapGetCountLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapGetCountLam}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureGetCapacity> bindTextureGetCapacityFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DGetCapacityLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapGetCapacityLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapGetCapacityLam}
				};
				std::unordered_map<J_GRAPHIC_RESOURCE_TYPE, BindTextureSetCapacity> bindTextureSetCapaFunc
				{
					{J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D, texture2DSetCapaLam}, {J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE, cubeMapeSetCapaLam},
					{J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, shadowMapSetCapaLam}
				};

				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					J_GRAPHIC_RESOURCE_TYPE type = (J_GRAPHIC_RESOURCE_TYPE)i;
					auto data = hasCallable.find(type);
					if (data != hasCallable.end())
					{
						updateHelper.RegisterCallable(type,
							&bindTextureGetCountFunc.find(type)->second,
							&bindTextureGetCapacityFunc.find(type)->second,
							&bindTextureSetCapaFunc.find(type)->second);
					}
				}

				workerFunctor = std::make_unique<WorkerThreadF::Functor>(&JGraphicImpl::WorkerThread, this);
			}
			~JGraphicImpl()
			{ 
			}
		public:
			ID3D12Device* GetDevice()const noexcept
			{
				return d3dDevice.Get();
			}
			ID3D12CommandQueue* GetCommandQueue()const noexcept
			{
				return commandQueue.Get();
			}
			ID3D12CommandAllocator* GetCommandAllocator()const noexcept
			{
				return publicCmdListAlloc.Get();
			}
			ID3D12GraphicsCommandList* GetCommandList()const noexcept
			{
				return publicCmdList.Get();
			}
			JGraphicInfo GetGraphicInfo()const noexcept
			{
				return info;
			}
			JGraphicOption GetGraphicOption()const noexcept
			{
				return option;
			}
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return graphicResourceM->GetCpuRtvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return graphicResourceM->GetCpuDsvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return graphicResourceM->GetCpuSrvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return graphicResourceM->GetCpuSrvDescriptorHandle(index);
				default:
					return CD3DX12_CPU_DESCRIPTOR_HANDLE();
				}
			}
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept
			{
				switch (bType)
				{
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::RTV:
					return graphicResourceM->GetGpuRtvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::DSV:
					return graphicResourceM->GetGpuDsvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::SRV:
					return graphicResourceM->GetGpuSrvDescriptorHandle(index);
				case JinEngine::Graphic::J_GRAPHIC_BIND_TYPE::UAV:
					return graphicResourceM->GetGpuSrvDescriptorHandle(index);
				default:
					return CD3DX12_GPU_DESCRIPTOR_HANDLE();
				}
			}
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex)
			{
				return GetCpuDescriptorHandle(bType, graphicResourceM->GetInfo(rType, rIndex)->GetHeapIndexStart(bType) + bIndex);
			}
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex)
			{
				return GetGpuDescriptorHandle(bType, graphicResourceM->GetInfo(rType, rIndex)->GetHeapIndexStart(bType) + bIndex);
			}
		public:
			void SetGraphicOption(JGraphicOption newGraphicOption)noexcept
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
			void SetImGuiBackEnd()
			{
				ID3D12DescriptorHeap* srvHeap = graphicResourceM->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV);
				ImGui_ImplDX12_Init(d3dDevice.Get(), Constants::gNumFrameResources,
					graphicResourceM->GetBackBufferFormat(),
					srvHeap,
					srvHeap->GetCPUDescriptorHandleForHeapStart(),
					srvHeap->GetGPUDescriptorHandleForHeapStart());
			}
			void SetInfoCapacity(uint& capacity, const uint count, const J_UPLOAD_CAPACITY_CONDITION cond)
			{
				capacity = CalculateCapacity(cond, capacity, count);
			}
		public:
			J_UPLOAD_CAPACITY_CONDITION IsPassRedefineCapacity(const uint capacity, const uint nowCount)const noexcept
			{
				if (capacity <= nowCount)
					return J_UPLOAD_CAPACITY_CONDITION::UP_CAPACITY;
				else if (nowCount < capacity / 2)
				{
					if (nowCount <= info.minCapacity)
						return J_UPLOAD_CAPACITY_CONDITION::KEEP;
					else
						return J_UPLOAD_CAPACITY_CONDITION::DOWN_CAPACITY;
				}
				else
					return J_UPLOAD_CAPACITY_CONDITION::KEEP;
			}
		public:
			void StartCommand()
			{
				if (!stCommand)
				{
					ThrowIfFailedHr(publicCmdList->Reset(publicCmdListAlloc.Get(), nullptr));
					stCommand = true;
				}
			}
			void EndCommand()
			{
				if (stCommand)
				{
					ThrowIfFailedG(publicCmdList->Close());
					ID3D12CommandList* cmdsLists[] = { publicCmdList.Get() };
					commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
					stCommand = false;
				}
			}
			void FlushCommandQueue()
			{
				// Advance the fence value to mark commands up to this fence point.
				currentFence++;
				// Add an instruction to the command queue to set a new fence point.  Because we 
				// are on the GPU timeline, the new fence point won't be set until the GPU finishes
				// libcessing all the commands prior to this Signal().
				ThrowIfFailedHr(commandQueue->Signal(fence.Get(), currentFence));

				// Wait until the GPU has completed commands up to this fence point.
				if (fence->GetCompletedValue() < currentFence)
				{
					HANDLE eventHandle = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
					// Fire event when GPU hits current fence.  
					ThrowIfFailedHr(fence->SetEventOnCompletion(currentFence, eventHandle));
					// Wait until the GPU hits current fence event is fired.
					WaitForSingleObject(eventHandle, INFINITE);
					CloseHandle(eventHandle);
				}
			}
		public:
			JUserPtr<JGraphicResourceInfo> CreateSceneDepthStencilResource()
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->CreateSceneDepthStencilResource(d3dDevice.Get(), publicCmdList.Get(), info.width, info.height, m4xMsaaState, m4xMsaaQuality);
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->CreateSceneDepthStencilResource(d3dDevice.Get(), publicCmdList.Get(), info.width, info.height, m4xMsaaState, m4xMsaaQuality);
			}
			JUserPtr<JGraphicResourceInfo> CreateSceneDepthStencilDebugResource()
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->CreateSceneDepthStencilDebugResource(d3dDevice.Get(), publicCmdList.Get(), info.width, info.height, m4xMsaaState, m4xMsaaQuality);
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->CreateSceneDepthStencilDebugResource(d3dDevice.Get(), publicCmdList.Get(), info.width, info.height, m4xMsaaState, m4xMsaaQuality);
			}
			JUserPtr<JGraphicResourceInfo> CreateDebugDepthStencilResource()
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->CreateDebugDepthStencilResource(d3dDevice.Get(), publicCmdList.Get(), info.width, info.height);
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->CreateDebugDepthStencilResource(d3dDevice.Get(), publicCmdList.Get(), info.width, info.height);
			}
			void CreateOcclusionHZBResource(_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo, _Out_ JUserPtr<JGraphicResourceInfo>& outOccMipMapInfo)
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					graphicResourceM->CreateOcclusionHZBResource(d3dDevice.Get(), publicCmdList.Get(), info.occlusionWidth, info.occlusionHeight, outOccDsInfo, outOccMipMapInfo);
					EndCommand();
					FlushCommandQueue();
				}
				else
					graphicResourceM->CreateOcclusionHZBResource(d3dDevice.Get(), publicCmdList.Get(), info.occlusionWidth, info.occlusionHeight, outOccDsInfo, outOccMipMapInfo);
			}
			JUserPtr<JGraphicResourceInfo> CreateOcclusionHZBResourceDebug()
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->CreateOcclusionHZBResourceDebug(d3dDevice.Get(), publicCmdList.Get(), info.occlusionWidth, info.occlusionWidth);
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->CreateOcclusionHZBResourceDebug(d3dDevice.Get(), publicCmdList.Get(), info.occlusionWidth, info.occlusionWidth);
			}
			JUserPtr<JGraphicResourceInfo> Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->Create2DTexture(uploadBuffer, path, oriFormat, d3dDevice.Get(), publicCmdList.Get());
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->Create2DTexture(uploadBuffer, path, oriFormat, d3dDevice.Get(), publicCmdList.Get());
			}
			JUserPtr<JGraphicResourceInfo> CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
			{
				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->CreateCubeMap(uploadBuffer, path, oriFormat, d3dDevice.Get(), publicCmdList.Get());
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->CreateCubeMap(uploadBuffer, path, oriFormat, d3dDevice.Get(), publicCmdList.Get());
			}
			JUserPtr<JGraphicResourceInfo> CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0 || textureHeight == 0)
				{
					textureWidth = info.width;
					textureHeight = info.height;
				}

				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->CreateRenderTargetTexture(d3dDevice.Get(), textureWidth, textureHeight);
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->CreateRenderTargetTexture(d3dDevice.Get(), textureWidth, textureHeight);
			}
			JUserPtr<JGraphicResourceInfo> CreateShadowMapTexture(uint textureWidth, uint textureHeight)
			{
				if (textureWidth == 0 || textureHeight == 0)
				{
					textureWidth = info.defaultShadowWidth;
					textureHeight = info.defaultShadowHeight;
				}

				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					JUserPtr<JGraphicResourceInfo> result = graphicResourceM->CreateShadowMapTexture(d3dDevice.Get(), textureWidth, textureHeight);
					EndCommand();
					FlushCommandQueue();
					return result;
				}
				else
					return graphicResourceM->CreateShadowMapTexture(d3dDevice.Get(), textureWidth, textureHeight);
			}
			bool DestroyGraphicTextureResource(JGraphicResourceInfo* info)
			{
				if (info == nullptr)
					return false;

				if (!stCommand)
				{
					FlushCommandQueue();
					StartCommand();
					bool res = graphicResourceM->DestroyGraphicTextureResource(d3dDevice.Get(), info);
					EndCommand();
					FlushCommandQueue();
					return res;
				}
				else
				{
					bool res = graphicResourceM->DestroyGraphicTextureResource(d3dDevice.Get(), info);
					return res;
				}
			}
		public:
			JUserPtr<JCullingInfo> CreateFrsutumCullingResultBuffer()
			{
				auto user = cullingM->CreateFrsutumData();
				frustumHelper->BuildResultBuffer(currFrameResource->bundingObjectCB->ElementCount(), user);
				//result arr
				return user;
			}
			JUserPtr<JCullingInfo> CreateOccCullingResultBuffer()
			{
				auto user = cullingM->CreateOcclusionData();
				if (user.IsValid())
				{
					if (!stCommand)
					{
						FlushCommandQueue();
						StartCommand();
						hzbOccHelper->BuildOccBuffer(d3dDevice.Get(), currFrameResource->bundingObjectCB->ElementCount(), user);
						EndCommand();
						FlushCommandQueue();
					}
					else
						hzbOccHelper->BuildOccBuffer(d3dDevice.Get(), currFrameResource->bundingObjectCB->ElementCount(), user);
				}
				return user;
			}
			bool DestroyCullignData(JCullingInfo* cullignInfo)
			{
				if (cullignInfo == nullptr)
					return false;

				const J_CULLING_TYPE cType = cullignInfo->GetCullingType();
				if (cType == J_CULLING_TYPE::OCCLUSION)
				{
					if (!stCommand)
					{
						FlushCommandQueue();
						StartCommand();
						hzbOccHelper->DestroyOccBuffer(cullignInfo);
						EndCommand();
						FlushCommandQueue();
					}
					else
						hzbOccHelper->DestroyOccBuffer(cullignInfo);
				}
				else if (cType == J_CULLING_TYPE::FRUSTUM)
					frustumHelper->DestroyResultBuffer(cullignInfo);
				return cullingM->DestroyCullingData(cullignInfo);
			}
		public:
			void StuffGraphicShaderPso(JGraphicShaderData* shaderData,
				const J_SHADER_VERTEX_LAYOUT vertexLayout,
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
				const JShaderGraphicPsoCondition& psoCondition,
				const J_GRAPHIC_EXTRA_PSO_TYPE extraType)
			{
				FlushCommandQueue();
				StartCommand();

				D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
				newShaderPso.InputLayout = { shaderData->inputLayout.data(), (uint)shaderData->inputLayout.size() };
				newShaderPso.pRootSignature = mRootSignature.Get();
				newShaderPso.VS =
				{
					reinterpret_cast<BYTE*>(shaderData->vs->GetBufferPointer()),
					shaderData->vs->GetBufferSize()
				};
				if ((gFunctionFlag & SHADER_FUNCTION_WRITE_SHADOWMAP) == 0 && (gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) == 0)
				{
					newShaderPso.PS =
					{
						reinterpret_cast<BYTE*>(shaderData->ps->GetBufferPointer()),
						shaderData->ps->GetBufferSize()
					};
				}
				if (shaderData->hs != nullptr)
				{
					newShaderPso.HS =
					{
						reinterpret_cast<BYTE*>(shaderData->hs->GetBufferPointer()),
						shaderData->hs->GetBufferSize()
					};
				}
				if (shaderData->ds != nullptr)
				{
					newShaderPso.DS =
					{
						reinterpret_cast<BYTE*>(shaderData->ds->GetBufferPointer()),
						shaderData->ds->GetBufferSize()
					};
				}
				if (shaderData->gs != nullptr)
				{
					newShaderPso.GS =
					{
						reinterpret_cast<BYTE*>(shaderData->gs->GetBufferPointer()),
						shaderData->gs->GetBufferSize()
					};
				}

				newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				if (extraType == J_GRAPHIC_EXTRA_PSO_TYPE::STENCIL_WRITE_ALWAYS)
				{
					newShaderPso.DepthStencilState.StencilEnable = true;
					newShaderPso.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
					newShaderPso.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
				}

				if ((gFunctionFlag & SHADER_FUNCTION_SKY) > 0)
				{
					newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
					//newShaderPso.DepthStencilState.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; 
					newShaderPso.DepthStencilState.StencilEnable = false;
				}

				newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				newShaderPso.SampleMask = UINT_MAX;
				newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				newShaderPso.NumRenderTargets = 1;
				newShaderPso.RTVFormats[0] = graphicResourceM->GetBackBufferFormat();
				newShaderPso.SampleDesc.Count = m4xMsaaState ? 4 : 1;
				newShaderPso.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;

				if ((gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) > 0)
					newShaderPso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				else
					newShaderPso.DSVFormat = graphicResourceM->GetDepthStencilFormat();

				if ((gFunctionFlag & SHADER_FUNCTION_WRITE_SHADOWMAP) > 0)
				{
					newShaderPso.DepthStencilState.StencilEnable = false;
					newShaderPso.RasterizerState.DepthBias = 100000;
					newShaderPso.RasterizerState.DepthBiasClamp = 0.0f;
					newShaderPso.RasterizerState.SlopeScaledDepthBias = 1.0f;
					newShaderPso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
					newShaderPso.NumRenderTargets = 0;
					newShaderPso.SampleDesc.Count = 1;
					newShaderPso.SampleDesc.Quality = 0;
				}
				if ((gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) > 0)
				{
					newShaderPso.DepthStencilState.StencilEnable = false;
					newShaderPso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
					newShaderPso.NumRenderTargets = 0;
					newShaderPso.SampleDesc.Count = 1;
					newShaderPso.SampleDesc.Quality = 0;
				}
				if ((gFunctionFlag & SHADER_FUNCTION_DEBUG) > 0)
				{
					//newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
					newShaderPso.DepthStencilState.StencilEnable = false;
				}

				if (psoCondition.primitiveCondition == J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO)
					newShaderPso.PrimitiveTopologyType = psoCondition.ConvertD3d12PrimitiveType();
				if (psoCondition.depthCompareCondition == J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO)
					newShaderPso.DepthStencilState.DepthFunc = psoCondition.ConvertD3d12Comparesion();
				if (psoCondition.cullModeCondition == J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO)
					newShaderPso.RasterizerState.CullMode = psoCondition.ConvertD3d12CullMode();

				if (extraType == J_GRAPHIC_EXTRA_PSO_TYPE::NONE)
				{
					ThrowIfFailedG(d3dDevice->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->pso.GetAddressOf())));
					shaderData->pso->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Pso") - 1, "Pso");
				}
				else if (extraType == J_GRAPHIC_EXTRA_PSO_TYPE::STENCIL_WRITE_ALWAYS)
				{
					ThrowIfFailedG(d3dDevice->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->extraPso[(int)extraType].GetAddressOf())));
					shaderData->extraPso[(int)extraType]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Extra Pso") - 1, "Extra Pso");
				}

				EndCommand();
				FlushCommandQueue();
			}
			void StuffComputeShaderPso(JComputeShaderData* shaderData, J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
			{
				FlushCommandQueue();
				StartCommand();

				D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
				ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

				if (cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_COPY ||
					cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING ||
					cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION)
					newShaderPso.pRootSignature = hzbOccHelper->GetRootSignature();

				shaderData->RootSignature = newShaderPso.pRootSignature;
				newShaderPso.CS =
				{
					reinterpret_cast<BYTE*>(shaderData->cs->GetBufferPointer()),
					shaderData->cs->GetBufferSize()
				};
				newShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
				ThrowIfFailedG(d3dDevice->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->pso.GetAddressOf())));

				EndCommand();
				FlushCommandQueue();
			}
		public:
			void ResourceTransition(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* pResource,
				D3D12_RESOURCE_STATES stateBefore,
				D3D12_RESOURCE_STATES stateAfter)
			{
				CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, stateBefore, stateAfter);
				cmdList->ResourceBarrier(1, &rsBarrier);
			}
		public:
			void ReBuildFrameResource(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowObjCount)
			{
				//const uint newCapacity = CalculateCapacity(condition, currFrameResource->GetElementCount(type), nowObjCount);
				//currFrameResource->ReBuildFrameResource(d3dDevice.Get(), type, newCapacity);
				for (int i = 0; i < Constants::gNumFrameResources; ++i)
				{
					const uint newCapacity = CalculateCapacity(condition, frameResources[i]->GetElementCount(type), nowObjCount);
					frameResources[i]->ReBuildFrameResource(d3dDevice.Get(), type, newCapacity);
				}
			}
			void ReCompileGraphicShader()
			{
				mRootSignature.Reset();
				BuildRootSignature();

				auto shaderVec = JShader::StaticTypeInfo().GetInstanceRawPtrVec();
				for (auto& data : shaderVec)
				{
					JShader* shader = static_cast<JShader*>(data);
					if (!shader->IsComputeShader())
						JShaderPrivate::CompileInterface::RecompileGraphicShader(shader);
				}
			}
		public:
			uint CalculateCapacity(const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowCapacity, const uint nowCount)const noexcept
			{
				uint nextCapacity = nowCapacity;
				if (condition == J_UPLOAD_CAPACITY_CONDITION::UP_CAPACITY)
				{
					while (nextCapacity <= nowCount)
						nextCapacity *= 2;
				}
				else if (condition == J_UPLOAD_CAPACITY_CONDITION::DOWN_CAPACITY)
				{
					while ((nextCapacity / 2) > nowCount && (nextCapacity / 2) > info.minCapacity)
						nextCapacity /= 2;
				}
				return nextCapacity;
			}
			void OnResize()
			{
				info.width = JWindow::GetClientWidth();
				info.height = JWindow::GetClientHeight();

				assert(d3dDevice);
				assert(swapChain);
				assert(publicCmdListAlloc);

				// Flush before changing any resources.
				FlushCommandQueue();
				StartCommand();

				currBackBuffer = 0;
				graphicResourceM->CreateSwapChainBuffer(d3dDevice.Get(), swapChain.Get(), info.width, info.height);
				outlineHelper->UpdatePassBuf(info.width, info.height, Constants::commonStencilRef);

				if (defaultSceneDsInfo != nullptr)
					DestroyGraphicTextureResource(defaultSceneDsInfo.Release());
				defaultSceneDsInfo = CreateSceneDepthStencilResource();

				// Wait until resize is complete.
				EndCommand();
				FlushCommandQueue();

				// Update the viewport transform to cover the client area.
				screenViewport.TopLeftX = 0;
				screenViewport.TopLeftY = 0;
				screenViewport.Width = static_cast<float>(info.width);
				screenViewport.Height = static_cast<float>(info.height);
				screenViewport.MinDepth = 0.0f;
				screenViewport.MaxDepth = 1.0f;

				scissorRect = { 0, 0,info.width, info.height };
			}
		public:
			ID3D12Resource* CurrentBackBuffer()const
			{
				return graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, currBackBuffer);
			}
			D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const
			{
				return graphicResourceM->GetCpuRtvDescriptorHandle(graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, currBackBuffer)->
					GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
			}
			const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()const noexcept
			{
				return std::vector<CD3DX12_STATIC_SAMPLER_DESC>
				{
					//PointWrap
					CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

						//pointClamp
						CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

							//linearWrap
						CD3DX12_STATIC_SAMPLER_DESC(2, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

							//linearClamp
						CD3DX12_STATIC_SAMPLER_DESC(3, // shaderRegister
							D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

							//anisotropicWrap
						CD3DX12_STATIC_SAMPLER_DESC(4, // shaderRegister
							D3D12_FILTER_ANISOTROPIC, // filter
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
							0.0f,                             // mipLODBias
							8),				                  // maxAnisotropy

							//anisotropicClamp
						CD3DX12_STATIC_SAMPLER_DESC(5, // shaderRegister
							D3D12_FILTER_ANISOTROPIC, // filter
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
							0.0f,                              // mipLODBias
							8),                                // maxAnisotropy

							//shadow
						CD3DX12_STATIC_SAMPLER_DESC(6, // shaderRegister
							D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
							D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
							D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
							D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
							0.0f,                               // mipLODBias
							16,                                 // maxAnisotropy
							D3D12_COMPARISON_FUNC_LESS_EQUAL,
							D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
				};
			}
		public:
			void UpdateWait()
			{
				currFrameResourceIndex = (currFrameResourceIndex + 1) % Constants::gNumFrameResources;
				currFrameResource = frameResources[currFrameResourceIndex].get();
 
				if (currFrameResource->fence != 0 && fence->GetCompletedValue() < currFrameResource->fence)
				{
					//nullptr, FALSE, FALSE, EVENT_ALL_ACCESS 
					HANDLE eventHandle = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
					ThrowIfFailedHr(fence->SetEventOnCompletion(currFrameResource->fence, eventHandle));
					WaitForSingleObject(eventHandle, INFINITE);
					CloseHandle(eventHandle);
				}
			}
			void UpdateGuiBackend()
			{
				ImGui_ImplDX12_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
			}
			void UpdateFrame()
			{
				updateHelper.Clear();
				for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				{
					updateHelper.uData[i].count = (*updateHelper.uData[i].getElement)(nullptr);
					updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
					updateHelper.uData[i].rebuildCondition = IsPassRedefineCapacity(updateHelper.uData[i].capacity, updateHelper.uData[i].count);
					updateHelper.hasRebuildCondition |= (bool)updateHelper.uData[i].rebuildCondition;
				}
				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					if (updateHelper.bData[i].HasCallable())
					{
						updateHelper.bData[i].count = (*updateHelper.bData[i].getTextureCount)(nullptr);
						updateHelper.bData[i].capacity = (*updateHelper.bData[i].getTextureCapacity)(nullptr);
						updateHelper.bData[i].recompileCondition = IsPassRedefineCapacity(updateHelper.bData[i].capacity, updateHelper.bData[i].count);
						updateHelper.hasRecompileShader |= (bool)updateHelper.bData[i].recompileCondition;

						if (updateHelper.bData[i].recompileCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
						{
							updateHelper.bData[i].capacity = CalculateCapacity(updateHelper.bData[i].recompileCondition,
								updateHelper.bData[i].capacity,
								updateHelper.bData[i].count);
						}
					}
				}

				updateHelper.WriteGraphicInfo(info);
				if (updateHelper.hasRebuildCondition)
				{
					FlushCommandQueue();
					StartCommand();
					for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
					{
						if (updateHelper.uData[i].rebuildCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
						{
							ReBuildFrameResource((J_UPLOAD_FRAME_RESOURCE_TYPE)i, updateHelper.uData[i].rebuildCondition, updateHelper.uData[i].count);
							updateHelper.uData[i].setDirty = Constants::gNumFrameResources;
							updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_FRAME_RESOURCE_TYPE)i);
						}
					}
					//Has sequency dependency
					updateHelper.WriteGraphicInfo(info);
					if (updateHelper.hasRecompileShader)
						ReCompileGraphicShader();		//use graphic info
					updateHelper.NotifyUpdateFrameCapacity(*thisGraphic);	//use graphic info				 
					EndCommand();
					FlushCommandQueue();
				}
				else if (updateHelper.hasRecompileShader)
				{
					FlushCommandQueue();
					StartCommand();
					ReCompileGraphicShader();
					EndCommand();
					FlushCommandQueue();
				}

				const uint drawListCount = JGraphicDrawList::GetListCount();
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					const bool isAllowOcclusion = drawTarget->scene->IsMainScene() && option.isOcclusionQueryActivated;
					const uint sceneDrawReqCount = (uint)drawTarget->sceneRequestor.size();
					drawTarget->UpdateStart();
					UpdateSceneAnimationCB(drawTarget->scene, drawTarget->updateInfo->aniUpdateCount, drawTarget->updateInfo->hotAniUpdateCount);
					UpdateSceneCameraCB(drawTarget->scene, drawTarget->updateInfo->camUpdateCount, drawTarget->updateInfo->hzbOccUpdateCount, drawTarget->updateInfo->hotCamUpdateCount);
					UpdateSceneLightCB(drawTarget->scene, drawTarget->updateInfo->lightUpdateCount, drawTarget->updateInfo->shadowMapUpdateCount, drawTarget->updateInfo->hzbOccUpdateCount, drawTarget->updateInfo->hotLitghtUpdateCount);	//always update
					UpdateSceneObjectCB(drawTarget->scene, drawTarget->updateInfo->objUpdateCount, drawTarget->updateInfo->hotObjUpdateCount);
					UpdateSceneRequestor(drawTarget);
					UpdateShadowRequestor(drawTarget);
					UpdateFrustumCullingRequestor(drawTarget);
					UpdateOccCullingRequestor(drawTarget);
					drawTarget->UpdateEnd();
				}
				UpdateScenePassCB();	//always update
				UpdateMaterialCB();
			}
		private:
			void UpdateSceneObjectCB(_In_ const JUserPtr<JScene>& scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
			{
				const bool isUpdateBoundingObj = scene->IsMainScene() && option.isOcclusionQueryActivated;
				const std::vector<JUserPtr<JComponent>>& jRvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint renderItemCount = (uint)jRvec.size();

				JObjectConstants objectConstants;
				JBoundingObjectConstants boundingConstants;
				JHzbOccObjectConstants occObjectConstants;

				auto currObjectCB = currFrameResource->objectCB.get();
				auto currBoundingObjectCB = currFrameResource->bundingObjectCB.get();
				auto currOccObjectBuffer = currFrameResource->hzbOccObjectBuffer.get();

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].setDirty;

				using FrameUpdateInterface = JRenderItemPrivate::FrameUpdateInterface;
				for (uint i = 0; i < renderItemCount; ++i)
				{
					JRenderItem* renderItem = static_cast<JRenderItem*>(jRvec[i].Get());
					if (FrameUpdateInterface::UpdateStart(renderItem, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(renderItem, boundingConstants);
						FrameUpdateInterface::UpdateFrame(renderItem, occObjectConstants);
						currBoundingObjectCB->CopyData(FrameUpdateInterface::GetBoundingFrameIndex(renderItem), boundingConstants);
						currOccObjectBuffer->CopyData(FrameUpdateInterface::GetOccObjectFrameIndex(renderItem), occObjectConstants);

						const uint objectFrameIndexOffset = FrameUpdateInterface::GetObjectFrameIndex(renderItem);
						const uint submeshCount = renderItem->GetSubmeshCount();
						for (uint j = 0; j < submeshCount; ++j)
						{
							FrameUpdateInterface::UpdateFrame(renderItem, objectConstants, j);
							currObjectCB->CopyData(objectFrameIndexOffset + j, objectConstants);
						}
						FrameUpdateInterface::UpdateEnd(renderItem);
						if (FrameUpdateInterface::IsHotUpdated(renderItem))
							++hotUpdateCount;
						++updateCount;
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
								currObjectCB->CopyData(objectFrameIndexOffset + j, objectConstants);
							}
						}
						if (FrameUpdateInterface::HasBoundingRecopyRequest(renderItem))
						{
							FrameUpdateInterface::UpdateFrame(renderItem, boundingConstants);
							currBoundingObjectCB->CopyData(FrameUpdateInterface::GetBoundingFrameIndex(renderItem), boundingConstants);
						}
						if (FrameUpdateInterface::HasOccObjectRecopyRequest(renderItem))
						{
							FrameUpdateInterface::UpdateFrame(renderItem, occObjectConstants);
							currOccObjectBuffer->CopyData(FrameUpdateInterface::GetOccObjectFrameIndex(renderItem), occObjectConstants);
						}
					}
				}

				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].offset += renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].offset += renderItemCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].offset += scene->GetMeshCount();
			}
			void UpdateMaterialCB()
			{
				auto currMaterialBuffer = currFrameResource->materialBuffer.get();
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
						currMaterialBuffer->CopyData(FrameUpdateInterface::GetMaterialFrameIndex(material), materialConstant);
						FrameUpdateInterface::UpdateEnd(material);
					}
					else if (FrameUpdateInterface::HasRecopyRequest(material))
					{
						FrameUpdateInterface::UpdateFrame(material, materialConstant);
						currMaterialBuffer->CopyData(FrameUpdateInterface::GetMaterialFrameIndex(material), materialConstant);
					}
				};
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].offset += matCount;
			}
			void UpdateScenePassCB()
			{
				JPassConstants passContants;
				passContants.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
				passContants.totalTime = JEngineTimer::Data().TotalTime();
				passContants.deltaTime = JEngineTimer::Data().DeltaTime();

				auto currPassCB = currFrameResource->passCB.get();
				currPassCB->CopyData(updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::PASS].offset, passContants);
				++updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::PASS].offset;
			}
			void UpdateSceneAnimationCB(_In_ const JUserPtr<JScene>& scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
			{
				const std::vector<JUserPtr<JComponent>>& jAvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
				const uint animatorCount = (uint)jAvec.size();

				auto currSkinnedCB = currFrameResource->skinnedCB.get();
				JAnimationConstants animationConstatns;

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].setDirty;
				using FrameUpdateInterface = JAnimatorPrivate::FrameUpdateInterface;

				if (scene->IsActivatedSceneTime())
				{
					for (uint i = 0; i < animatorCount; ++i)
					{
						JAnimator* animator = static_cast<JAnimator*>(jAvec[i].Get());
						if (FrameUpdateInterface::UpdateStart(animator))
						{
							FrameUpdateInterface::UpdateFrame(animator, animationConstatns);
							currSkinnedCB->CopyData(FrameUpdateInterface::GetFrameIndex(animator), animationConstatns);
							FrameUpdateInterface::UpdateEnd(animator);
							++updateCount;
						}
						else if (FrameUpdateInterface::HasRecopyRequest(animator))
						{
							FrameUpdateInterface::UpdateFrame(animator, animationConstatns);
							currSkinnedCB->CopyData(FrameUpdateInterface::GetFrameIndex(animator), animationConstatns);
						}
					}
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].offset += animatorCount;
			}
			void UpdateSceneCameraCB(_In_ const JUserPtr<JScene>& scene,
				_Out_ uint& camUpdateCount,
				_Out_ uint& hzbUpdateCount,
				_Out_ uint& hotUpdateCount)
			{
				//is same as sceneRequestor 
				const std::vector<JUserPtr<JComponent>>& jCvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
				const uint cameraCount = (uint)jCvec.size();
				const uint hzbOccQueryCount = scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint hzbOccQueryOffset = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].offset;

				auto currCameraCB = currFrameResource->cameraCB.get();
				auto currHzbOccReqCB = currFrameResource->hzbOccReqCB.get();

				JCameraConstants camContants;
				JHzbOccRequestorConstants hzbOccReqConstants;

				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].setDirty ||
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_REQUESTOR].setDirty;
				using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;

				for (uint i = 0; i < cameraCount; ++i)
				{
					JCamera* camera = static_cast<JCamera*>(jCvec[i].Get());
					if (FrameUpdateInterface::UpdateStart(camera, forcedSetFrameDirty))
					{
						FrameUpdateInterface::UpdateFrame(camera, camContants);
						currCameraCB->CopyData(FrameUpdateInterface::GetCamFrameIndex(camera), camContants);
						if (camera->AllowHzbOcclusionCulling())
						{
							FrameUpdateInterface::UpdateFrame(camera, hzbOccReqConstants, hzbOccQueryCount, hzbOccQueryOffset);
							currHzbOccReqCB->CopyData(FrameUpdateInterface::GetHzbOccReqFrameIndex(camera), hzbOccReqConstants);
							++hzbUpdateCount;
						}
						FrameUpdateInterface::UpdateEnd(camera);
						if (FrameUpdateInterface::IsHotUpdated(camera))
							++hotUpdateCount;
						++camUpdateCount;
					}
					else
					{
						if (FrameUpdateInterface::HasCamRecopyRequest(camera))
						{
							FrameUpdateInterface::UpdateFrame(camera, camContants);
							currCameraCB->CopyData(FrameUpdateInterface::GetCamFrameIndex(camera), camContants);
						}
						if (FrameUpdateInterface::HasOccPassRecopyRequest(camera))
						{
							FrameUpdateInterface::UpdateFrame(camera, hzbOccReqConstants, hzbOccQueryCount, hzbOccQueryOffset);
							currHzbOccReqCB->CopyData(FrameUpdateInterface::GetHzbOccReqFrameIndex(camera), hzbOccReqConstants);
						}
					}
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].offset += cameraCount;
			}
			void UpdateSceneLightCB(_In_ const JUserPtr<JScene>& scene,
				_Out_ uint& litUpdateCount,
				_Out_ uint& shadowMapUpdateCount,
				_Out_ uint& hzbUpdateCount,
				_Out_ uint& hotUpdateCount)
			{
				const std::vector<JUserPtr<JComponent>>& jLvec = JScenePrivate::CashInterface::GetComponentCashVec(scene, J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
				const uint lightVecCount = (uint)jLvec.size();
				const uint hzbOccQueryCount = scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				const uint hzbOccQueryOffset = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].offset;

				auto currLightCB = currFrameResource->lightBuffer.get();
				auto currLightIndexCB = currFrameResource->lightIndexCB.get();
				auto currSMLightCB = currFrameResource->smLightBuffer.get();
				auto currShadowMapCB = currFrameResource->shadowMapCalCB.get();
				auto currHzbOccReqCB = currFrameResource->hzbOccReqCB.get();

				JLightConstants lightConstants;
				JLightIndexConstants lightIndexConstants;
				JShadowMapLightConstants smLightConstants;
				JShadowMapConstants smConstants;
				JHzbOccRequestorConstants hzbOccReqConstants;

				uint litCount = 0;
				uint smLitCount = 0;

				bool hasLitUpdate = false;
				const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_INDEX].setDirty |
					updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT].setDirty;

				//hzb 미구현
				using LitFrameUpdateInterface = JLightPrivate::FrameUpdateInterface;
				using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;

				for (uint i = 0; i < lightVecCount; ++i)
				{
					JLight* light = static_cast<JLight*>(jLvec[i].Get());
					const bool onShadow = light->IsShadowActivated();

					if (LitFrameUpdateInterface::UpdateStart(light, forcedSetFrameDirty))
					{
						if (onShadow)
						{
							LitFrameUpdateInterface::UpdateFrame(light, smLightConstants);
							LitFrameUpdateInterface::UpdateFrame(light, smConstants);
							currSMLightCB->CopyData(LitFrameUpdateInterface::GetShadowLitFrameIndex(light), smLightConstants);
							currShadowMapCB->CopyData(LitFrameUpdateInterface::GetShadowMapFrameIndex(light), smConstants);
							++shadowMapUpdateCount;
						}
						else
						{
							LitFrameUpdateInterface::UpdateFrame(light, lightConstants);
							currLightCB->CopyData(LitFrameUpdateInterface::GetLitFrameIndex(light), lightConstants);
						}
						if (light->AllowHzbOcclusionCulling())
						{
							LitFrameUpdateInterface::UpdateFrame(light, hzbOccReqConstants, hzbOccQueryCount, hzbOccQueryOffset);
							currHzbOccReqCB->CopyData(LitFrameUpdateInterface::GetHzbOccReqFrameIndex(light), hzbOccReqConstants);
							++hzbUpdateCount;
						}
						LitFrameUpdateInterface::UpdateEnd(light);
						if (LitFrameUpdateInterface::IsHotUpdated(light))
							++hotUpdateCount;
						++litUpdateCount;
					}
					else
					{
						if (LitFrameUpdateInterface::HasLitRecopyRequest(light))
						{
							LitFrameUpdateInterface::UpdateFrame(light, lightConstants);
							currLightCB->CopyData(LitFrameUpdateInterface::GetLitFrameIndex(light), lightConstants);
						}
						if (LitFrameUpdateInterface::HasShadowLitRecopyRequest(light))
						{
							LitFrameUpdateInterface::UpdateFrame(light, smLightConstants);
							currSMLightCB->CopyData(LitFrameUpdateInterface::GetShadowLitFrameIndex(light), smLightConstants);
						}
						if (LitFrameUpdateInterface::HasShadowMapRecopyRequest(light))
						{
							LitFrameUpdateInterface::UpdateFrame(light, smConstants);
							currShadowMapCB->CopyData(LitFrameUpdateInterface::GetShadowMapFrameIndex(light), smConstants);
						}
						if (LitFrameUpdateInterface::HasOccPassRecopyRequest(light))
						{
							LitFrameUpdateInterface::UpdateFrame(light, hzbOccReqConstants, hzbOccQueryCount, hzbOccQueryOffset);
							currHzbOccReqCB->CopyData(LitFrameUpdateInterface::GetHzbOccReqFrameIndex(light), hzbOccReqConstants);
						}
					}
					if (onShadow)
						++smLitCount;
					else
						++litCount;
				}

				lightIndexConstants.litStIndex = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT].offset;
				lightIndexConstants.litEdIndex = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT].offset + litCount;
				lightIndexConstants.shadwMapStIndex = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset;
				lightIndexConstants.shadowMapEdIndex = updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset + smLitCount;

				const uint litIndexFrameIndex = SceneFrameIndexInterface::GetLitIndexFrameIndex(scene.Get());
				currLightIndexCB->CopyData(litIndexFrameIndex, lightIndexConstants);

				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT].offset += litCount;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_INDEX].offset += 1;
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset += smLitCount;
			}
			void UpdateSceneRequestor(JGraphicDrawTarget* target)
			{
				using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
				for (auto& data : target->sceneRequestor)
				{
					JCamera* cam = data->jCamera.Get();
					if (FrameUpdateInterface::IsLastUpdated(cam))
						data->isUpdated = true;
				}
			}
			void UpdateShadowRequestor(JGraphicDrawTarget* target)
			{
				using FrameUpdateInterface = JLightPrivate::FrameUpdateInterface;
				for (auto& data : target->shadowRequestor)
				{
					JLight* lit = data->jLight.Get();
					if (FrameUpdateInterface::IsHotUpdated(lit))
						data->isUpdated = true;
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP].offset += target->shadowRequestor.size();
			}
			void UpdateFrustumCullingRequestor(JGraphicDrawTarget* target)
			{
				using CamFrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
				using LitFrameUpdateInterface = JLightPrivate::FrameUpdateInterface;

				for (auto& data : target->frustumCullingRequestor)
				{
					JComponent* comp = data->comp.Get();
					J_COMPONENT_TYPE cType = comp->GetComponentType();
					if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					{
						if (CamFrameUpdateInterface::IsHotUpdated(static_cast<JCamera*>(comp)))
							data->isUpdated = true;
					}
					else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					{
						if (LitFrameUpdateInterface::IsHotUpdated(static_cast<JLight*>(comp)))
							data->isUpdated = true;
					}
				}
			}
			void UpdateOccCullingRequestor(JGraphicDrawTarget* target)
			{
				using CamFrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
				using LitFrameUpdateInterface = JLightPrivate::FrameUpdateInterface;

				for (auto& data : target->hzbOccCullingRequestor)
				{
					JComponent* comp = data->comp.Get();
					J_COMPONENT_TYPE cType = comp->GetComponentType();
					if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					{
						if (CamFrameUpdateInterface::IsHotUpdated(static_cast<JCamera*>(comp)))
							data->isUpdated = true;
					}
					else if (cType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					{
						if (LitFrameUpdateInterface::IsHotUpdated(static_cast<JLight*>(comp)))
							data->isUpdated = true;
					}
				}
				updateHelper.uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_REQUESTOR].offset += target->hzbOccCullingRequestor.size();
			}
		public:
			void DrawScene()
			{
				if (option.allowMultiThread)
					DrawUseMultiThread();
				else
					DrawUseSingThread();
			}
			void DrawGui()
			{
				currFrameResource->ResetCmd(J_MAIN_THREAD_CMD_ORDER::END); 
				EndFrame();
			}
		private:
			void DrawUseSingThread()
			{
				currFrameResource->ResetCmd(J_MAIN_THREAD_CMD_ORDER::BEGIN);
				currFrameResource->ResetCmd(J_MAIN_THREAD_CMD_ORDER::END);
				ID3D12GraphicsCommandList* cmdList = currFrameResource->GetCmd(J_MAIN_THREAD_CMD_ORDER::BEGIN);
				ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResourceM->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
				cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
				cmdList->OMSetStencilRef(Constants::commonStencilRef);
				SettingGraphicRootSignature(cmdList);

				//수정필요
				//Shadow Map Draw
				//Update가 있는 광원만 추적해서 Draw
				//광원내에 있는 오브젝트 검색을 위한 공간분할 및 검색 필요

				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper;
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.scene = drawTarget->scene;
					helper.drawTarget = drawTarget;
					for (const auto& data : drawTarget->frustumCullingRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JScenePrivate::CullingInterface::ViewCulling(drawTarget->scene, data->comp);
					}
					SettingGraphicRootSignature(cmdList);
					if (option.isHZBOcclusionActivated)
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingOccCulling(data->comp);
							DrawOcclusionDepthMap(cmdList, copiedHelper);
						}
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingOccCulling(data->comp);
							ComputeOcclusionCulling(cmdList, copiedHelper);
						}
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingOccCulling(data->comp);
							DrawOcclusionMipMap(cmdList, copiedHelper);
						}
						SettingGraphicRootSignature(cmdList);
					}
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawShadowMap(data->jLight);
						DrawShadowMap(cmdList, copiedHelper);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						DrawSceneRenderTarget(cmdList, copiedHelper);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						DrawOutline(cmdList, copiedHelper);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						DrawSceneDepthMap(cmdList, copiedHelper);
					} 
				}
				ThrowIfFailedG(cmdList->Close());
				ID3D12CommandList* cmdsLists[] = { cmdList };
				commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

				EndFrame();
			}
			void DrawUseMultiThread()
			{ 
				currFrameResource->ResetCmd(info.frameThreadCount);
				BeginFrame();
				ComputeFrustumCulling();
				 
				for (uint i = 0; i < info.frameThreadCount; ++i)
					GraphicThreadInteface::CreateDrawThread(Core::JThreadInitInfo{}, UniqueBind(*workerFunctor, std::move(i)));

				WaitForMultipleObjects(info.frameThreadCount, handle.endHzbDrawDepthMap, true, INFINITE);	
				commandQueue->ExecuteCommandLists(info.frameThreadCount, currFrameResource->hzbOccDrawCmdBatch);
				WaitForMultipleObjects(info.frameThreadCount, handle.endDrawShadwMap, true, INFINITE);
				commandQueue->ExecuteCommandLists(info.frameThreadCount, currFrameResource->shadowCmdBatch);
				WaitForMultipleObjects(info.frameThreadCount, handle.endDrawScene, true, INFINITE);
				commandQueue->ExecuteCommandLists(info.frameThreadCount, currFrameResource->sceneCmdBatch);
				 
				MidFrame(); 
				EndFrame(); 
			}
			void DrawSceneRenderTarget(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{
				auto gRInterface = helper.cam->GraphicResourceUserInterface();
				const uint rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
				const uint rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV);

				const uint dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL);
				const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV);

				using CamFrameInterface = JCameraPrivate::FrameUpdateInterface;
				using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;
				const uint camFrameIndex = CamFrameInterface::GetCamFrameIndex(helper.cam.Get());
				const uint litIndexFrmaeIndex = SceneFrameIndexInterface::GetLitIndexFrameIndex(helper.scene.Get());

				cmdList->RSSetViewports(1, &screenViewport);
				cmdList->RSSetScissorRects(1, &scissorRect);

				ID3D12Resource* dsResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsvVecIndex);
				JGraphicResourceInfo* dsInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsvVecIndex);

				ID3D12Resource* rtResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);
				JGraphicResourceInfo* rtInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);

				CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResourceM->GetCpuRtvDescriptorHandle(rtvHeapIndex);
				CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(dsvHeapIndex);

				ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

				cmdList->ClearRenderTargetView(rtv, graphicResourceM->GetBackBufferClearColor(), 0, nullptr);
				cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
				cmdList->OMSetRenderTargets(1, &rtv, true, &dsv);

				currFrameResource->passCB->SetGraphicCBBufferView(cmdList, 2, 0);
				currFrameResource->cameraCB->SetGraphicCBBufferView(cmdList, 3, camFrameIndex);
				currFrameResource->lightIndexCB->SetGraphicCBBufferView(cmdList, 4, litIndexFrmaeIndex);

				using GCash = JScenePrivate::CashInterface; 
				const std::vector<JUserPtr<JGameObject>>& objVec00 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec01 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED);
				const std::vector<JUserPtr<JGameObject>>& objVec02 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::DEBUG_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec03 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::SKY, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec04 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::DEBUG_UI, J_MESHGEOMETRY_TYPE::STATIC);

				DrawGameObject(cmdList, objVec00, helper, JDrawCondition(option, helper, false, true, helper.allowDrawDebug));
				DrawGameObject(cmdList, objVec01, helper, JDrawCondition(option, helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebug));
				if (option.IsHDOccActivated())
					cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
				if (helper.allowDrawDebug)
					DrawGameObject(cmdList, objVec02, helper, JDrawCondition());
				DrawGameObject(cmdList, objVec03, helper, JDrawCondition());
				if (helper.allowDrawDebug)
				{
					const uint debugVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL);
					const uint debugHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV);
					ID3D12Resource* debugResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, debugVecIndex);
					JGraphicResourceInfo* debugDepthInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, debugVecIndex);

					CD3DX12_CPU_DESCRIPTOR_HANDLE editorDsv = graphicResourceM->GetCpuDsvDescriptorHandle(debugHeapIndex);
					ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					cmdList->ClearDepthStencilView(editorDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
					cmdList->OMSetRenderTargets(1, &rtv, true, &editorDsv);
					DrawGameObject(cmdList, objVec04, helper, JDrawCondition());
					ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
				}

				ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
				ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
			}
			void DrawSceneRenderTargetMultiThread(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{
				auto gRInterface = helper.cam->GraphicResourceUserInterface();
			 
				const uint rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV);	 
				const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV);

				using CamFrameInterface = JCameraPrivate::FrameUpdateInterface;
				using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;
				const uint camFrameIndex = CamFrameInterface::GetCamFrameIndex(helper.cam.Get());
				const uint litIndexFrmaeIndex = SceneFrameIndexInterface::GetLitIndexFrameIndex(helper.scene.Get());

				cmdList->RSSetViewports(1, &screenViewport);
				cmdList->RSSetScissorRects(1, &scissorRect);

				CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResourceM->GetCpuRtvDescriptorHandle(rtvHeapIndex);
				CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(dsvHeapIndex);
  
				cmdList->OMSetRenderTargets(1, &rtv, true, &dsv);

				currFrameResource->passCB->SetGraphicCBBufferView(cmdList, 2, 0);
				currFrameResource->cameraCB->SetGraphicCBBufferView(cmdList, 3, camFrameIndex);
				currFrameResource->lightIndexCB->SetGraphicCBBufferView(cmdList, 4, litIndexFrmaeIndex);

				using GCash = JScenePrivate::CashInterface;
				const std::vector<JUserPtr<JGameObject>>& objVec00 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec01 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED);
				const std::vector<JUserPtr<JGameObject>>& objVec02 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::DEBUG_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
				const std::vector<JUserPtr<JGameObject>>& objVec03 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::SKY, J_MESHGEOMETRY_TYPE::STATIC);

				DrawGameObject(cmdList, objVec00, helper, JDrawCondition(option, helper, false, true, helper.allowDrawDebug));
				DrawGameObject(cmdList, objVec01, helper, JDrawCondition(option, helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebug));
				if (option.IsHDOccActivated())
					cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
				if (helper.allowDrawDebug)
					DrawGameObject(cmdList, objVec02, helper, JDrawCondition());
				  
				DrawGameObject(cmdList, objVec03, helper, JDrawCondition());
			}
			void DrawSceneDebugUI(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{
				auto gRInterface = helper.cam->GraphicResourceUserInterface();
				const uint rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV);

				using CamFrameInterface = JCameraPrivate::FrameUpdateInterface;
				using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;
				const uint camFrameIndex = CamFrameInterface::GetCamFrameIndex(helper.cam.Get());
				const uint litIndexFrmaeIndex = SceneFrameIndexInterface::GetLitIndexFrameIndex(helper.scene.Get());

				cmdList->RSSetViewports(1, &screenViewport);
				cmdList->RSSetScissorRects(1, &scissorRect);

				CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResourceM->GetCpuRtvDescriptorHandle(rtvHeapIndex);

				currFrameResource->passCB->SetGraphicCBBufferView(cmdList, 2, 0);
				currFrameResource->cameraCB->SetGraphicCBBufferView(cmdList, 3, camFrameIndex);
				currFrameResource->lightIndexCB->SetGraphicCBBufferView(cmdList, 4, litIndexFrmaeIndex);
				 
				using GCash = JScenePrivate::CashInterface;
				const std::vector<JUserPtr<JGameObject>>& objVec = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::DEBUG_UI, J_MESHGEOMETRY_TYPE::STATIC);
				 
				if (helper.allowDrawDebug)
				{
					const uint debugVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL);
					const uint debugHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV);
					ID3D12Resource* debugResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, debugVecIndex);
					JGraphicResourceInfo* debugDepthInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_DEPTH_STENCIL, debugVecIndex);

					CD3DX12_CPU_DESCRIPTOR_HANDLE editorDsv = graphicResourceM->GetCpuDsvDescriptorHandle(debugHeapIndex);
					ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					cmdList->ClearDepthStencilView(editorDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
					cmdList->OMSetRenderTargets(1, &rtv, true, &editorDsv);
					DrawGameObject(cmdList, objVec, helper, JDrawCondition());
					ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
				}
			}
			void DrawShadowMap(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{
				if (option.allowMultiThread)
					shadowMap->DrawSceneShadowMapMultiThread(cmdList,
						currFrameResource,
						hzbOccHelper.get(),
						graphicResourceM.get(),
						option, helper);
				else
					shadowMap->DrawSceneShadowMap(cmdList,
						currFrameResource,
						hzbOccHelper.get(),
						graphicResourceM.get(),
						option, helper);
			}
			void DrawOcclusionDepthMap(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{		 
				if (!CanHzbOccCulling(helper))
					return;

				//cmdList->SetGraphicsRootSignature(mRootSignature.Get());
				auto gRInterface = helper.cam->GraphicResourceUserInterface();
				const uint occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
				const uint occHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, J_GRAPHIC_BIND_TYPE::DSV);
				const uint occMipMapVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);

				D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)info.occlusionWidth, (float)info.occlusionHeight, 0.0f, 1.0f };
				D3D12_RECT mScissorRect = { 0, 0, info.occlusionWidth, info.occlusionHeight };

				cmdList->RSSetViewports(1, &mViewport);
				cmdList->RSSetScissorRects(1, &mScissorRect);

				ID3D12Resource* occDepthMap = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
				JGraphicResourceInfo* occDsInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
				D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(occHeapIndex);

				ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
				cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);

				using CamFrameInterface = JCameraPrivate::FrameUpdateInterface;
				const uint camFrameIndex = CamFrameInterface::GetCamFrameIndex(helper.cam.Get());

				currFrameResource->cameraCB->SetGraphicCBBufferView(cmdList, 3, camFrameIndex);
				DrawSceneBoundingBox(cmdList,
					helper.scene->GetAlignedObject(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, helper.cam->GetBoundingFrustum()),
					helper, JDrawCondition(option, helper, false, true, false));

				ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
			}
			void DrawOcclusionDepthMapMultiThread(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{
				if (!CanHzbOccCulling(helper))
					return;

				//cmdList->SetGraphicsRootSignature(mRootSignature.Get());
				auto gRInterface = helper.cam->GraphicResourceUserInterface(); 
				const uint occHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, J_GRAPHIC_BIND_TYPE::DSV);
 
				D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)info.occlusionWidth, (float)info.occlusionHeight, 0.0f, 1.0f };
				D3D12_RECT mScissorRect = { 0, 0, info.occlusionWidth, info.occlusionHeight };

				cmdList->RSSetViewports(1, &mViewport);
				cmdList->RSSetScissorRects(1, &mScissorRect);
				  
				D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(occHeapIndex);		  
				cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);

				using CamFrameInterface = JCameraPrivate::FrameUpdateInterface;
				const uint camFrameIndex = CamFrameInterface::GetCamFrameIndex(helper.cam.Get());

				currFrameResource->cameraCB->SetGraphicCBBufferView(cmdList, 3, camFrameIndex);
				DrawSceneBoundingBox(cmdList,
					helper.scene->GetAlignedObject(Core::J_SPACE_SPATIAL_LAYER::COMMON_OBJECT, helper.cam->GetBoundingFrustum()),
					helper, JDrawCondition(option, helper, false, true, false)); 
			}
			void DrawOcclusionMipMap(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)	//for debug
			{
				if (!helper.allowDrawOccMipMap)
					return;

				JVector2<uint> occlusionSize = JVector2<uint>(info.occlusionWidth, info.occlusionHeight);
				const float camNear = helper.cam->GetNear();
				const float camFar = helper.cam->GetFar();

				auto gRInterface = helper.cam->GraphicResourceUserInterface();
				const uint occMipMapVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
				JGraphicResourceInfo* occMipMapInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, occMipMapVecIndex);

				//Debug and mipmap viwe count is same
				const uint occDebugVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
				JGraphicResourceInfo* occDebugInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG, occDebugVecIndex);
				const uint viewCount = occDebugInfo->GetViewCount(J_GRAPHIC_BIND_TYPE::SRV);
				for (uint i = 0; i < viewCount; ++i)
				{
					depthMapDebug->DrawLinearDepthDebug(cmdList,
						graphicResourceM->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + i),
						graphicResourceM->GetGpuSrvDescriptorHandle(occDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV) + i),
						occlusionSize,
						camNear,
						camFar);
					occlusionSize /= 2;
				}
			}
			void DrawGameObject(ID3D12GraphicsCommandList* cmdList,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper helper,
				const JDrawCondition& condition)
			{
				uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
				uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

				auto objectCB = currFrameResource->objectCB->Resource();
				auto skinCB = currFrameResource->skinnedCB->Resource();

				using RItemFrameIndexInterface = JRenderItemPrivate::FrameIndexInterface;
				using MeshBufferViewInterface = JMeshGeometryPrivate::BufferViewInterface;

				const uint gameObjCount = (uint)gameObject.size();
				uint st = 0;
				uint ed = gameObjCount;
				if (helper.CanDispatchWorkIndex())
					helper.DispatchWorkIndex(gameObjCount, st, ed);
				 
				for (uint i = st; i < ed; ++i)
				{
					JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
					const uint objFrameIndex = RItemFrameIndexInterface::GetObjectFrameIndex(renderItem);
					const uint boundFrameIndex = RItemFrameIndexInterface::GetBoundingFrameIndex(renderItem);

					if (condition.allowCulling && helper.cullUser.UnsafeIsCulled(boundFrameIndex))
						continue;

					if (condition.allowAllCullingResult && helper.RefelectOtherCamCullig(boundFrameIndex))
						continue;

					JMeshGeometry* mesh = renderItem->GetMesh().Get();
					const D3D12_VERTEX_BUFFER_VIEW vertexPtr = MeshBufferViewInterface::VertexBufferView(mesh);
					const D3D12_INDEX_BUFFER_VIEW indexPtr = MeshBufferViewInterface::IndexBufferView(mesh);

					cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
					cmdList->IASetIndexBuffer(&indexPtr);
					cmdList->IASetPrimitiveTopology(renderItem->GetPrimitiveType());

					JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>().Get();
					const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

					if (condition.allowDebugOutline && gameObject[i]->IsSelected())
						cmdList->OMSetStencilRef(2);

					for (uint j = 0; j < submeshCount; ++j)
					{
						const JShader* shader = renderItem->GetValidMaterial(j)->GetShader().Get();		
						const bool onSkinned = animator != nullptr && condition.allowAnimation;
						const J_MESHGEOMETRY_TYPE meshType = onSkinned ? J_MESHGEOMETRY_TYPE::SKINNED : J_MESHGEOMETRY_TYPE::STATIC;
						const J_SHADER_VERTEX_LAYOUT shaderLayout = JShaderType::ConvertToVertexLayout(meshType);

						if (condition.allowDebugOutline && gameObject[i]->IsSelected())
							cmdList->SetPipelineState(shader->GetGraphicExtraPso(shaderLayout, J_GRAPHIC_EXTRA_PSO_TYPE::STENCIL_WRITE_ALWAYS));
						else
							cmdList->SetPipelineState(shader->GetGraphicPso(shaderLayout));

						const uint fianlObjFrameIndex = objFrameIndex + j;
						D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + fianlObjFrameIndex * objectCBByteSize;
						cmdList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
						if (onSkinned)
						{
							using AniFrameIndexInterface = JAnimatorPrivate::FrameIndexInterface;
							const uint aniFrameIndex = AniFrameIndexInterface::GetFrameIndex(animator);
							D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + aniFrameIndex * skinCBByteSize;
							cmdList->SetGraphicsRootConstantBufferView(1, skinObjCBAddress);
						}
						if (condition.allowHDOcclusionCulling)
							cmdList->SetPredication(graphicResourceM->GetOcclusionQueryResult(), fianlObjFrameIndex * 8, D3D12_PREDICATION_OP_EQUAL_ZERO);

						cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
					}
					if (condition.allowDebugOutline && gameObject[i]->IsSelected())
						cmdList->OMSetStencilRef(Constants::commonStencilRef);
				}
			}
			void DrawSceneBoundingBox(ID3D12GraphicsCommandList* cmdList,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper helper,
				const JDrawCondition& condition)
			{
				//JMeshGeometry* mesh = _JResourceManager::Instance().Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::CUBE);
				JMeshGeometry* mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE).Get();
				JMaterial* mat = _JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST).Get();
				JShader* shader = mat->GetShader().Get();

				using RItemFrameIndexInterface = JRenderItemPrivate::FrameIndexInterface;
				using MeshBufferViewInterface = JMeshGeometryPrivate::BufferViewInterface;

				const D3D12_VERTEX_BUFFER_VIEW vertexPtr = MeshBufferViewInterface::VertexBufferView(mesh);
				const D3D12_INDEX_BUFFER_VIEW indexPtr = MeshBufferViewInterface::IndexBufferView(mesh);

				cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
				cmdList->IASetIndexBuffer(&indexPtr);
				cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				uint boundingObjectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JBoundingObjectConstants));
				auto boundingObjectCB = currFrameResource->bundingObjectCB->Resource();

				uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));
				auto skinCB = currFrameResource->skinnedCB->Resource();

				const uint gameObjCount = (uint)gameObject.size();
				uint st = 0;
				uint ed = gameObjCount;
				if (helper.CanDispatchWorkIndex())
					helper.DispatchWorkIndex(gameObjCount, st, ed);
				for (uint i = st; i < ed; ++i)
				{
					JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
					const uint boundFrameIndex = RItemFrameIndexInterface::GetBoundingFrameIndex(renderItem);

					if (condition.allowCulling && helper.cullUser.UnsafeIsCulled(J_CULLING_TYPE::FRUSTUM, boundFrameIndex))
						continue;

					//Test Code
					//수정필요
					//if (JMathHelper::Vector3Length(gameObject[i]->GetTransform()->GetScale()) < 5)
					//	continue;

					cmdList->SetPipelineState(shader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::STATIC)));

					D3D12_GPU_VIRTUAL_ADDRESS boundingObjectCBAddress = boundingObjectCB->GetGPUVirtualAddress() + boundFrameIndex * boundingObjectCBByteSize;
					cmdList->SetGraphicsRootConstantBufferView(6, boundingObjectCBAddress);

					if (condition.allowHzbOcclusionCulling)
						cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
					else if (condition.allowHDOcclusionCulling)
					{
						cmdList->BeginQuery(graphicResourceM->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, boundFrameIndex);
						cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
						cmdList->EndQuery(graphicResourceM->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, boundFrameIndex);
					}
				}
			}
			void DrawOutline(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)	//has root signature
			{
				if (helper.allowDrawDebug)
				{
					auto gRInterface = helper.cam->GraphicResourceUserInterface();

					const uint rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
					const uint rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV);

					const uint dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL);
					const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV);

					ID3D12Resource* rtResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);
					JGraphicResourceInfo* depthInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsvVecIndex);

					CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResourceM->GetCpuRtvDescriptorHandle(rtvHeapIndex);
					ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
					
					cmdList->RSSetViewports(1, &screenViewport);
					cmdList->RSSetScissorRects(1, &scissorRect);
					cmdList->OMSetRenderTargets(1, &rtv, true, nullptr);

					outlineHelper->DrawOutline(cmdList,
						graphicResourceM->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						graphicResourceM->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + 1));
					ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
				}
			}
			void DrawSceneDepthMap(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{
				if (helper.allowDrawDepthMap)
				{
					auto gRInterface = helper.cam->GraphicResourceUserInterface();
					const uint sceneDsIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL);
					const uint sceneDebugDsIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG);

					JGraphicResourceInfo* mainDepthInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, sceneDsIndex);
					JGraphicResourceInfo* mainDepthDebugInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG, sceneDebugDsIndex);

					depthMapDebug->DrawNonLinearDepthDebug(cmdList,
						graphicResourceM->GetGpuSrvDescriptorHandle(mainDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						graphicResourceM->GetGpuSrvDescriptorHandle(mainDepthDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
						JVector2<uint>(info.width, info.height),
						helper.cam->GetNear(),
						helper.cam->GetFar());
				}
			}
		public:
			void ComputeOcclusionCulling(ID3D12GraphicsCommandList* cmdList, const JDrawHelper helper)
			{
				auto gRInterface = helper.cam->GraphicResourceUserInterface();
				const uint occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
				const uint occMipMapVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);

				if (option.IsHZBOccActivated())
				{
					JGraphicResourceInfo* occDsInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
					JGraphicResourceInfo* occMipMapInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP, occMipMapVecIndex);
					uint occPassFrameIndex = 0;

					if (helper.cam != nullptr)
					{
						using FrameIndexInterface = JCameraPrivate::FrameIndexInterface;
						occPassFrameIndex = FrameIndexInterface::GetHzbOccReqFrameIndex(helper.cam.Get());
					}
					else if (helper.lit != nullptr)
					{
						using FrameIndexInterface = JLightPrivate::FrameIndexInterface;
						occPassFrameIndex = FrameIndexInterface::GetHzbOccReqFrameIndex(helper.lit.Get());
					}

					hzbOccHelper->DepthMapDownSampling(cmdList,
						currFrameResource,
						graphicResourceM->GetGpuSrvDescriptorHandle(occDsInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						graphicResourceM->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						graphicResourceM->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
						info.occlusionMapCount,
						graphicResourceM->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::SRV),
						occPassFrameIndex);
					hzbOccHelper->OcclusionCulling(cmdList,
						currFrameResource,
						graphicResourceM->GetGpuSrvDescriptorHandle(occMipMapInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
						occPassFrameIndex,
						helper.cullUser);
				}
				else if (option.IsHDOccActivated())
				{
					CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResourceM->GetOcclusionQueryResult(), D3D12_RESOURCE_STATE_PREDICATION, D3D12_RESOURCE_STATE_COPY_DEST);
					cmdList->ResourceBarrier(1, &rsBarrier);
					cmdList->ResolveQueryData(graphicResourceM->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, 0, graphicResourceM->GetOcclusionQueryCapacity(), graphicResourceM->GetOcclusionQueryResult(), 0);
					rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResourceM->GetOcclusionQueryResult(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PREDICATION);
					cmdList->ResourceBarrier(1, &rsBarrier);
				}
			}
		private:
			void WorkerThread(uint threadIndex)
			{ 
				auto sceneCmdList = currFrameResource->GetSceneCmd(threadIndex);
				auto shadowCmdList = currFrameResource->GetShadowCmd(threadIndex);
				auto hzbDrawOccCmdList = currFrameResource->GetHzbDrawOccCmd(threadIndex); 

				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper;
				helper.SetAllowMultithreadDraw(option, true);
				helper.SetTheadInfo(info.frameThreadCount, threadIndex);

				if (option.isHZBOcclusionActivated)
				{
					SettingDescriptorHeaps(hzbDrawOccCmdList);
					SettingGraphicRootSignature(hzbDrawOccCmdList);
					for (uint i = 0; i < drawListCount; ++i)
					{
						JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
						helper.SetDrawTarget(drawTarget); 

						if (option.isHZBOcclusionActivated)
						{
							for (const auto& data : drawTarget->hzbOccCullingRequestor)
							{
								if (!data->canDrawThisFrame)
									continue;

								JDrawHelper copiedHelper = helper;
								copiedHelper.SettingOccCulling(data->comp);
								DrawOcclusionDepthMapMultiThread(hzbDrawOccCmdList, copiedHelper);
							}
						}  
					}
				} 
				ThrowIfFailedG(hzbDrawOccCmdList->Close());
				SetEvent(handle.endHzbDrawDepthMap[threadIndex]);

				SettingDescriptorHeaps(shadowCmdList);
				SettingGraphicRootSignature(shadowCmdList); 

				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawShadowMap(data->jLight);
						DrawShadowMap(shadowCmdList, copiedHelper);
					} 
				}
				ThrowIfFailedG(shadowCmdList->Close());
				SetEvent(handle.endDrawShadwMap[threadIndex]);

				SettingDescriptorHeaps(sceneCmdList);
				SettingGraphicRootSignature(sceneCmdList); 

				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						DrawSceneRenderTargetMultiThread(sceneCmdList, copiedHelper);
					} 
				}
				ThrowIfFailedG(sceneCmdList->Close());
				SetEvent(handle.endDrawScene[threadIndex]);
			}		 
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

						JScenePrivate::CullingInterface::ViewCulling(drawTarget->scene, data->comp);
					}
				}
			}
		private:
			void BeginFrame()
			{
				ID3D12GraphicsCommandList* cmdList = currFrameResource->GetCmd(J_MAIN_THREAD_CMD_ORDER::BEGIN);
				SettingDescriptorHeaps(cmdList);

				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper;

				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget); 

					if (option.isHZBOcclusionActivated)
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingOccCulling(data->comp);

							if (!CanHzbOccCulling(copiedHelper))
								continue;

							//cmdList->SetGraphicsRootSignature(mRootSignature.Get());
							auto gRInterface = copiedHelper.cam->GraphicResourceUserInterface();
							const uint occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
							const uint occHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, J_GRAPHIC_BIND_TYPE::DSV);

							ID3D12Resource* occDepthMap = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
							D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(occHeapIndex);

							ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
							cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
						}
					}
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawShadowMap(data->jLight);

						auto gRInterface = copiedHelper.lit->GraphicResourceUserInterface();
						const J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
						const uint rVecIndex = gRInterface.GetResourceArrayIndex(rType);
						const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(rType, J_GRAPHIC_BIND_TYPE::DSV);

						ID3D12Resource* shdowMapResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, rVecIndex);
						ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

						D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(dsvHeapIndex);
						cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);

						auto gRInterface = copiedHelper.cam->GraphicResourceUserInterface();
						const uint rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
						const uint rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV);

						const uint dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL);
						const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV);

						using CamFrameInterface = JCameraPrivate::FrameUpdateInterface;
						const uint camFrameIndex = CamFrameInterface::GetCamFrameIndex(copiedHelper.cam.Get());

						ID3D12Resource* dsResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsvVecIndex);
						JGraphicResourceInfo* dsInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsvVecIndex);

						ID3D12Resource* rtResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);
						JGraphicResourceInfo* rtInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);

						CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResourceM->GetCpuRtvDescriptorHandle(rtvHeapIndex);
						CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(dsvHeapIndex);

						ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
						ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

						cmdList->ClearRenderTargetView(rtv, graphicResourceM->GetBackBufferClearColor(), 0, nullptr);
						cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
					}
				}

				ThrowIfFailedG(cmdList->Close());
				ID3D12CommandList* cmdsLists[] = { cmdList };
				commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
			}
			void MidFrame()
			{
				ID3D12GraphicsCommandList* cmdList = currFrameResource->GetCmd(J_MAIN_THREAD_CMD_ORDER::MID);
				SettingDescriptorHeaps(cmdList);
				SettingGraphicRootSignature(cmdList);

				const uint drawListCount = JGraphicDrawList::GetListCount();
				JDrawHelper helper;

				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.isHZBOcclusionActivated)
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingOccCulling(data->comp);

							if (!CanHzbOccCulling(copiedHelper))
								continue;

							//cmdList->SetGraphicsRootSignature(mRootSignature.Get());
							auto gRInterface = copiedHelper.cam->GraphicResourceUserInterface();
							const uint occVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
							ID3D12Resource* occDepthMap = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP, occVecIndex);
							ResourceTransition(cmdList, occDepthMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
						}
					}
					for (const auto& data : drawTarget->shadowRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawShadowMap(data->jLight);

						auto gRInterface = copiedHelper.lit->GraphicResourceUserInterface();
						const J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
						const uint rVecIndex = gRInterface.GetResourceArrayIndex(rType);

						ID3D12Resource* shdowMapResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, rVecIndex);
						ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);

						if (copiedHelper.allowDrawDebug)
							DrawSceneDebugUI(cmdList, copiedHelper);

						auto gRInterface = copiedHelper.cam->GraphicResourceUserInterface();
						const uint rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON);
						const uint dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL);

						ID3D12Resource* dsResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsvVecIndex);
						ID3D12Resource* rtResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);

						ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
						ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
					} 
				}
				 
				for (uint i = 0; i < drawListCount; ++i)
				{
					JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
					helper.SetDrawTarget(drawTarget);

					if (option.isHZBOcclusionActivated)
					{
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingOccCulling(data->comp);
							ComputeOcclusionCulling(cmdList, copiedHelper);
						}
						for (const auto& data : drawTarget->hzbOccCullingRequestor)
						{
							if (!data->canDrawThisFrame)
								continue;

							JDrawHelper copiedHelper = helper;
							copiedHelper.SettingOccCulling(data->comp);
							DrawOcclusionMipMap(cmdList, copiedHelper);
						}
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						DrawOutline(cmdList, copiedHelper);
					}
					for (const auto& data : drawTarget->sceneRequestor)
					{
						if (!data->canDrawThisFrame)
							continue;

						JDrawHelper copiedHelper = helper;
						copiedHelper.SettingDrawScene(data->jCamera);
						DrawSceneDepthMap(cmdList, copiedHelper);
					} 
				}
				ThrowIfFailedG(cmdList->Close());
				ID3D12CommandList* cmdsLists[] = { cmdList };
				commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
			}
			void EndFrame()
			{
				ImGui::Render();
				ID3D12GraphicsCommandList* cmdList = currFrameResource->GetCmd(J_MAIN_THREAD_CMD_ORDER::END);

				ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResourceM->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
				cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

				const uint dsIndex = defaultSceneDsInfo->GetArrayIndex();
				ID3D12Resource* mainDepthResource = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsIndex);
				JGraphicResourceInfo* mainDepthInfo = graphicResourceM->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL, dsIndex);

				ResourceTransition(cmdList, mainDepthResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				ResourceTransition(cmdList, CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
				D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResourceM->GetCpuDsvDescriptorHandle(mainDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV));

				cmdList->ClearRenderTargetView(rtv, graphicResourceM->GetBackBufferClearColor(), 0, nullptr);
				cmdList->OMSetRenderTargets(1, &rtv, true, nullptr);

				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);

				ResourceTransition(cmdList, CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
				ResourceTransition(cmdList, mainDepthResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
				ThrowIfFailedG(cmdList->Close());

				ID3D12CommandList* cmdsLists[] = { cmdList };
				commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

				//ImGuiIO& io = ImGui::GetIO();
				//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				//{
				//	ImGui::UpdatePlatformWindows();
				//	ImGui::RenderPlatformWindowsDefault(NULL, (void*)cmdsLists);
				//}

				// Swap the back and front buffers
				ThrowIfFailedG(swapChain->Present(1, 0));
				currBackBuffer = (currBackBuffer + 1) % graphicResourceM->GetResourceCapacity(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN);
				// Advance the fence value to mark commands up to this fence point.
				currFrameResource->fence = ++currentFence;

				// Add an instruction to the command queue to set a new fence point. 
				// Because we are on the GPU timeline, the new fence point won't be 
				// set untwil the GPU finishes processing all the commands prior to this Signal().
				commandQueue->Signal(fence.Get(), currentFence);
			}
		private:
			void SettingDescriptorHeaps(ID3D12GraphicsCommandList* cmdList)
			{
				ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResourceM->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
				cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			}
			void SettingGraphicRootSignature(ID3D12GraphicsCommandList* cmdList)
			{ 
				cmdList->SetGraphicsRootSignature(mRootSignature.Get());

				cmdList->SetGraphicsRootShaderResourceView(7, currFrameResource->lightBuffer->Resource()->GetGPUVirtualAddress());
				cmdList->SetGraphicsRootShaderResourceView(8, currFrameResource->smLightBuffer->Resource()->GetGPUVirtualAddress());
				cmdList->SetGraphicsRootShaderResourceView(9, currFrameResource->materialBuffer->Resource()->GetGPUVirtualAddress());

				cmdList->SetGraphicsRootDescriptorTable(10, graphicResourceM->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE));
				cmdList->SetGraphicsRootDescriptorTable(11, graphicResourceM->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D));
				cmdList->SetGraphicsRootDescriptorTable(12, graphicResourceM->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP));
			}
		private:
			bool CanHzbOccCulling(const JDrawHelper& helper)
			{
				const bool isOcclusionActivated = option.IsHDOccActivated() || option.IsHZBOccActivated();
				const bool canCullingStart = (helper.cam != nullptr && helper.cam->AllowHzbOcclusionCulling()) ||
					helper.lit != nullptr && helper.lit->AllowHzbOcclusionCulling();

				return isOcclusionActivated && canCullingStart;
			}
		public:
			void Initialize()
			{
				AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);

				info.width = JWindow::GetClientWidth();
				info.height = JWindow::GetClientHeight();

				graphicResourceM = std::make_unique<JGraphicResourceManager>();
				cullingM = std::make_unique<JCullingManager>();
				shadowMap = std::make_unique<JShadowMap>();
				frustumHelper = std::make_unique<JFrustumCulling>();
				hdOccHelper = std::make_unique<JHardwareOccCulling>();
				hzbOccHelper = std::make_unique<JHZBOccCulling>();
				depthMapDebug = std::make_unique<JDepthMapDebug>();
				outlineHelper = std::make_unique<JOutline>();
				handle.Initialize(info.frameThreadCount);

				LoadData();
				InitializeD3D();
				InitializeResource();
				OnResize();
			}
			void Clear()
			{
				if (JApplicationEngine::GetApplicationState() != J_APPLICATION_STATE::PROJECT_SELECT)
					StoreData();

				updateHelper.Clear();
				FlushCommandQueue();
				StartCommand();

				handle.Clear();

				outlineHelper->Clear();
				outlineHelper.reset();

				depthMapDebug->Clear();
				depthMapDebug.reset();

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

				DestroyGraphicTextureResource(defaultSceneDsInfo.Release());
				graphicResourceM->Clear();
				graphicResourceM.reset();

				cullingM->Clear();
				cullingM.reset();

				EndCommand();
				FlushCommandQueue();

				swapChain.Reset();
				mRootSignature.Reset();

				fence.Reset();
				currentFence = 0;

				commandQueue.Reset();
				publicCmdListAlloc.Reset();
				publicCmdList.Reset();

				m4xMsaaState = false;
				m4xMsaaQuality = 0;

				d3dDevice.Reset();
				dxgiFactory.Reset();

				screenViewport = D3D12_VIEWPORT();
				scissorRect = D3D12_RECT();

				currBackBuffer = 0;
				info.width = 0;
				info.height = 0;

				RemoveListener(*JWindow::EvInterface(), guid);

#ifdef DEBUG
				HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
				decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));
				IDXGIDebug* debug;
				GetDebugInterface(IID_PPV_ARGS(&debug));
				OutputDebugStringW(L"Starting Live Direct3D Object Dump:\r\n");
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
				OutputDebugStringW(L"Completed Live Direct3D Object Dump.\r\n");
				debug->Release();
#endif
			}
		private:
			bool InitializeD3D()
			{
#if defined(DEBUG) || defined(_DEBUG) 
				// Enable the D3D12 DEBUG layer.
				{
					ComPtr<ID3D12Debug> debugController;
					ThrowIfFailedHr(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
					debugController->EnableDebugLayer();
				}
#endif 
				ThrowIfFailedHr(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
				//ThrowIfFailedHr(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,  IID_PPV_ARGS(&dxgiFactory)));

				HRESULT hardwareResult = D3D12CreateDevice(
					nullptr,
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&d3dDevice));

				if (FAILED(hardwareResult))
				{
					ComPtr<IDXGIAdapter> pWrapAdapter;
					ThrowIfFailedHr(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWrapAdapter)));

					ThrowIfFailedHr(D3D12CreateDevice(
						pWrapAdapter.Get(),
						D3D_FEATURE_LEVEL_11_0,
						IID_PPV_ARGS(&d3dDevice)));
				}

				ThrowIfFailedHr(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
				d3dDevice->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Main Device") - 1, "Main Device");
				// Check 4X MSAA quality support for our back buffer format.
				// All Direct3D 11 capable devices support 4X MSAA for all render 
				// target formats, so we only need to check quality support.

				D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
				msQualityLevels.Format = graphicResourceM->GetBackBufferFormat();
				msQualityLevels.SampleCount = 4;
				msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
				msQualityLevels.NumQualityLevels = 0;
				ThrowIfFailedHr(d3dDevice->CheckFeatureSupport(
					D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
					&msQualityLevels,
					sizeof(msQualityLevels)));

				m4xMsaaQuality = msQualityLevels.NumQualityLevels;
				assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
				LogAdapters();
#endif  
				CreateCommandObjects();
				CreateSwapChain();
				return true;
			}
			bool InitializeResource()
			{
				FlushCommandQueue();
				StartCommand();
				BuildRootSignature();
				graphicResourceM->Initialize(d3dDevice.Get());
				graphicResourceM->CreateOcclusionQueryResource(d3dDevice.Get());
				BuildFrameResources();
				shadowMap->Initialize(d3dDevice.Get(), info);
				frustumHelper->Initialize();
				hzbOccHelper->Initialize(d3dDevice.Get(), info);
				depthMapDebug->Initialize(d3dDevice.Get(), graphicResourceM->GetBackBufferFormat(), graphicResourceM->GetDepthStencilFormat());
				outlineHelper->Initialize(d3dDevice.Get(), graphicResourceM->GetBackBufferFormat(), graphicResourceM->GetDepthStencilFormat(), info.width, info.height, Constants::commonStencilRef);
				EndCommand();
				FlushCommandQueue();
				return true;
			}
			void LogAdapters()
			{
				uint i = 0;
				IDXGIAdapter* adapter = nullptr;
				std::vector<IDXGIAdapter*> adapterList;
				while (dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_ADAPTER_DESC desc;
					adapter->GetDesc(&desc);

					std::wstring text = L"***Adapter: ";
					text += desc.Description;
					text += L"\n";

					OutputDebugString(text.c_str());
					adapterList.push_back(adapter);
					++i;
				}
				for (size_t i = 0; i < adapterList.size(); ++i)
				{
					LogAdapterOutputs(adapterList[i]);
					ReleaseCom(adapterList[i]);
				}
			}
			void LogAdapterOutputs(IDXGIAdapter* adapter)
			{
				uint i = 0;
				IDXGIOutput* output = nullptr;
				while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
				{
					DXGI_OUTPUT_DESC desc;
					output->GetDesc(&desc);

					std::wstring text = L"***Output: ";
					text += desc.DeviceName;
					text += L"\n";
					OutputDebugString(text.c_str());

					LogOutputDisplayModes(output, graphicResourceM->GetBackBufferFormat());

					ReleaseCom(output);

					++i;
				}
			}
			void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
			{
				uint count = 0;
				uint flags = 0;

				// Call with nullptr to get list count.
				output->GetDisplayModeList(format, flags, &count, nullptr);

				std::vector<DXGI_MODE_DESC> modeList(count);
				output->GetDisplayModeList(format, flags, &count, &modeList[0]);

				for (auto& x : modeList)
				{
					uint n = x.RefreshRate.Numerator;
					uint d = x.RefreshRate.Denominator;
					std::wstring text =
						L"Width = " + std::to_wstring(x.Width) + L" " +
						L"Height = " + std::to_wstring(x.Height) + L" " +
						L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
						L"\n";
					::OutputDebugString(text.c_str());
				}
			}
			void CreateCommandObjects()
			{
				D3D12_COMMAND_QUEUE_DESC queueDesc = {};
				queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
				queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				ThrowIfFailedHr(d3dDevice->CreateCommandQueue(
					&queueDesc,
					IID_PPV_ARGS(&commandQueue)));

				ThrowIfFailedHr(d3dDevice->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(publicCmdListAlloc.GetAddressOf())));

				ThrowIfFailedHr(d3dDevice->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					publicCmdListAlloc.Get(),
					nullptr,
					IID_PPV_ARGS(publicCmdList.GetAddressOf())));

				publicCmdList->Close();
			}
			void CreateSwapChain()
			{
				swapChain.Reset();
				DXGI_SWAP_CHAIN_DESC sd;
				sd.BufferDesc.Width = info.width;
				sd.BufferDesc.Height = info.height;
				sd.BufferDesc.RefreshRate.Numerator = 60;
				sd.BufferDesc.RefreshRate.Denominator = 1;
				sd.BufferDesc.Format = graphicResourceM->GetBackBufferFormat();
				sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
				sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
				sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				sd.BufferCount = graphicResourceM->GetSwapChainBufferCount();
				sd.OutputWindow = Window::JWindowPrivate::HandleInterface::GetHandle();
				sd.Windowed = true;
				sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				// Note: Swap chain uses queue to perform flush.

				ThrowIfFailedHr(dxgiFactory->CreateSwapChain(
					commandQueue.Get(),
					&sd,
					swapChain.GetAddressOf()));
			}
			void BuildRootSignature()
			{
				// Root parameter can be a table, root descriptor or root constants.
				CD3DX12_DESCRIPTOR_RANGE cubeMapTable;
				cubeMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingCubeMapCapacity, 2, 1);

				CD3DX12_DESCRIPTOR_RANGE tex2DTable;
				tex2DTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.binding2DTextureCapacity, 2, 2);

				CD3DX12_DESCRIPTOR_RANGE shadowMapTable;
				shadowMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCapacity, 2, 3);

				static constexpr int slotCount = 13;
				CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

				// Create root CBV.
				slotRootParameter[0].InitAsConstantBufferView(0);
				slotRootParameter[1].InitAsConstantBufferView(1);
				slotRootParameter[2].InitAsConstantBufferView(2);
				slotRootParameter[3].InitAsConstantBufferView(3);
				slotRootParameter[4].InitAsConstantBufferView(4);
				slotRootParameter[5].InitAsConstantBufferView(5);
				slotRootParameter[6].InitAsConstantBufferView(6);

				//Light Buff
				slotRootParameter[7].InitAsShaderResourceView(0);
				slotRootParameter[8].InitAsShaderResourceView(0, 1);
				//Material Buff
				slotRootParameter[9].InitAsShaderResourceView(1);
				//Texture Buff
				slotRootParameter[10].InitAsDescriptorTable(1, &cubeMapTable, D3D12_SHADER_VISIBILITY_ALL);
				slotRootParameter[11].InitAsDescriptorTable(1, &tex2DTable, D3D12_SHADER_VISIBILITY_ALL);
				slotRootParameter[12].InitAsDescriptorTable(1, &shadowMapTable, D3D12_SHADER_VISIBILITY_ALL);

				const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Sampler();

				// A root signature is an array of root parameters.
				CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter,
					(uint)sam.size(), sam.data(),
					D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

				// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
				ComPtr<ID3DBlob> serializedRootSig = nullptr;
				ComPtr<ID3DBlob> errorBlob = nullptr;
				HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
					serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

				if (errorBlob != nullptr)
				{
					::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				}
				ThrowIfFailedHr(hr);
				ThrowIfFailedHr(d3dDevice->CreateRootSignature(
					0,
					serializedRootSig->GetBufferPointer(),
					serializedRootSig->GetBufferSize(),
					IID_PPV_ARGS(mRootSignature.GetAddressOf())));

				mRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Main RootSignature") - 1, "Main RootSignature");
			}
			void BuildFrameResources()
			{
				for (int i = 0; i < Constants::gNumFrameResources; ++i)
					frameResources[i] = std::make_unique<JFrameResource>(d3dDevice.Get(), info);
				currFrameResource = frameResources[currFrameResourceIndex].get();
			}
		public:
			void StoreData()
			{
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::SettingPath(), L"GraphicOption.txt");
				std::wofstream stream;
				stream.open(path, std::ios::binary | std::ios::out);
				if (!stream.is_open())
					return;

				JFileIOHelper::StoreJString(stream, L"--Info--", L"");
				JFileIOHelper::StoreAtomicData(stream, L"UploadObjCount:", info.upObjCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadAniCount:", info.upAniCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadPassCount:", info.upPassCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadCameraCount:", info.upCameraCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadLightCount:", info.upLightCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadShadowMapLightCount:", info.upSmLightCount);
				JFileIOHelper::StoreAtomicData(stream, L"UploadMaterialCount:", info.upMaterialCount);
				JFileIOHelper::StoreAtomicData(stream, L"Bind2DTextureCount:", info.binding2DTextureCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindCubeMapCount:", info.bindingCubeMapCapacity);
				JFileIOHelper::StoreAtomicData(stream, L"BindShadowTextureCount:", info.bindingShadowTextureCapacity);

				JFileIOHelper::StoreJString(stream, L"--Option--", L"");
				JFileIOHelper::StoreAtomicData(stream, L"AllowOcclusionQuery:", option.isOcclusionQueryActivated);
				JFileIOHelper::StoreAtomicData(stream, L"HardwareOcclusionAcitvated:", option.isHDOcclusionAcitvated);
				JFileIOHelper::StoreAtomicData(stream, L"HZBOcclusionAcitvated:", option.isHZBOcclusionActivated);
				JFileIOHelper::StoreAtomicData(stream, L"AllowHZBCorrectFail:", option.allowHZBCorrectFail);
				JFileIOHelper::StoreAtomicData(stream, L"AllowOutline:", option.allowDebugOutline);
				JFileIOHelper::StoreAtomicData(stream, L"AllowMultiThread:", option.allowMultiThread);
				stream.close();
			}
			void LoadData()
			{
				const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::SettingPath(), L"GraphicOption.txt");
				std::wifstream stream;
				stream.open(path, std::ios::binary | std::ios::in);
				if (!stream.is_open())
					return;

				std::wstring guide;
				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, info.upObjCount);
				JFileIOHelper::LoadAtomicData(stream, info.upAniCount);
				JFileIOHelper::LoadAtomicData(stream, info.upPassCount);
				JFileIOHelper::LoadAtomicData(stream, info.upCameraCount);
				JFileIOHelper::LoadAtomicData(stream, info.upLightCount);
				JFileIOHelper::LoadAtomicData(stream, info.upSmLightCount);
				JFileIOHelper::LoadAtomicData(stream, info.upMaterialCount);
				JFileIOHelper::LoadAtomicData(stream, info.binding2DTextureCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingCubeMapCapacity);
				JFileIOHelper::LoadAtomicData(stream, info.bindingShadowTextureCapacity);

				JGraphicOption newOption;
				JFileIOHelper::LoadJString(stream, guide);
				JFileIOHelper::LoadAtomicData(stream, newOption.isOcclusionQueryActivated);
				JFileIOHelper::LoadAtomicData(stream, newOption.isHDOcclusionAcitvated);
				JFileIOHelper::LoadAtomicData(stream, newOption.isHZBOcclusionActivated);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowHZBCorrectFail);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowDebugOutline);
				JFileIOHelper::LoadAtomicData(stream, newOption.allowMultiThread);
				stream.close();
				SetGraphicOption(newOption);
			}
			void WriteLastRsTexture()
			{
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				{
					// 0 is main camera

					auto mainCamRs = graphicResourceM->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
					JApplicationProjectInfo* opendInfo = JApplicationProject::GetOpenProjectInfo();
					if (opendInfo == nullptr)
						MessageBox(0, L"get open proejct info error", 0, 0);

					SaveDDSTextureToFile(commandQueue.Get(), mainCamRs, opendInfo->lastRsPath().c_str());
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
		void JGraphic::SetGraphicOption(JGraphicOption newGraphicOption)noexcept
		{
			impl->SetGraphicOption(newGraphicOption);
		}
		JGraphic::JGraphic()
			:impl(std::make_unique<JGraphicImpl>(Core::MakeGuid(), this))
		{}
		JGraphic::~JGraphic()
		{
			impl.reset();
		}

		using DeviceInterface = JGraphicPrivate::DeviceInterface;
		using ResourceInterface = JGraphicPrivate::ResourceInterface;
		using CullingInterface = JGraphicPrivate::CullingInterface;
		using EditorInterface = JGraphicPrivate::EditorInterface;
		using DebugInterface = JGraphicPrivate::DebugInterface;
		using CommandInterface = JGraphicPrivate::CommandInterface;
		using AppAccess = JGraphicPrivate::AppAccess;

		ID3D12Device* DeviceInterface::GetDevice() noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetDevice();
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceInterface::GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCpuDescriptorHandle(bType, index);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceInterface::GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetGpuDescriptorHandle(bType, index);
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceInterface::GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
			const J_GRAPHIC_BIND_TYPE bType,
			int rIndex,
			int bIndex)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCpuDescriptorHandle(rType, bType, rIndex, bIndex);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceInterface::GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
			const J_GRAPHIC_BIND_TYPE bType,
			int rIndex,
			int bIndex)noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetGpuDescriptorHandle(rType, bType, rIndex, bIndex);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateSceneDepthStencilResource()
		{
			return JinEngine::JGraphic::Instance().impl->CreateSceneDepthStencilResource();
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateSceneDepthStencilDebugResource()
		{
			return JinEngine::JGraphic::Instance().impl->CreateSceneDepthStencilDebugResource();
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateDebugDepthStencilResource()
		{
			return JinEngine::JGraphic::Instance().impl->CreateDebugDepthStencilResource();
		}
		void ResourceInterface::CreateOcclusionHZBResource(_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo, _Out_ JUserPtr<JGraphicResourceInfo>& outOccMipMapInfo)
		{
			JinEngine::JGraphic::Instance().impl->CreateOcclusionHZBResource(outOccDsInfo, outOccMipMapInfo);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateOcclusionHZBResourceDebug()
		{
			return JinEngine::JGraphic::Instance().impl->CreateOcclusionHZBResourceDebug();
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)
		{
			return JinEngine::JGraphic::Instance().impl->Create2DTexture(uploadHeap, path, oriFormat);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)
		{
			return JinEngine::JGraphic::Instance().impl->CreateCubeMap(uploadHeap, path, oriFormat);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateRenderTargetTexture(textureWidth, textureHeight);
		}
		JUserPtr<JGraphicResourceInfo> ResourceInterface::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			return JinEngine::JGraphic::Instance().impl->CreateShadowMapTexture(textureWidth, textureHeight);
		}
		bool ResourceInterface::DestroyGraphicTextureResource(JGraphicResourceInfo* info)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyGraphicTextureResource(info);
		}
		JUserPtr<JCullingInfo> CullingInterface::CreateFrsutumCullingResultBuffer()
		{
			return JinEngine::JGraphic::Instance().impl->CreateFrsutumCullingResultBuffer();
		}
		JUserPtr<JCullingInfo> CullingInterface::CreateOccCullingResultBuffer()
		{
			return JinEngine::JGraphic::Instance().impl->CreateOccCullingResultBuffer();
		}
		bool CullingInterface::DestroyCullignData(JCullingInfo* cullignInfo)
		{
			return JinEngine::JGraphic::Instance().impl->DestroyCullignData(cullignInfo);
		}
		void ResourceInterface::StuffGraphicShaderPso(JGraphicShaderData* shaderData,
			const J_SHADER_VERTEX_LAYOUT vertexLayout,
			const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JShaderGraphicPsoCondition& psoCondition,
			const J_GRAPHIC_EXTRA_PSO_TYPE extraType)
		{
			JinEngine::JGraphic::Instance().impl->StuffGraphicShaderPso(shaderData, vertexLayout, gFunctionFlag, psoCondition, extraType);
		}
		void ResourceInterface::StuffComputeShaderPso(JComputeShaderData* shaderData, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		{
			JinEngine::JGraphic::Instance().impl->StuffComputeShaderPso(shaderData, cFunctionFlag);
		}

		void EditorInterface::SetImGuiBackEnd()
		{
			JinEngine::JGraphic::Instance().impl->SetImGuiBackEnd();
		}

		JGraphicResourceManager* DebugInterface::GetGraphicResourceManager()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->graphicResourceM.get();
		}

		ID3D12CommandQueue* CommandInterface::GetCommandQueue()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCommandQueue();
		}
		ID3D12CommandAllocator* CommandInterface::GetCommandAllocator()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCommandAllocator();
		}
		ID3D12GraphicsCommandList* CommandInterface::GetCommandList()noexcept
		{
			return JinEngine::JGraphic::Instance().impl->GetCommandList();
		}
		void CommandInterface::StartCommand()
		{
			JinEngine::JGraphic::Instance().impl->StartCommand();
		}
		void CommandInterface::EndCommand()
		{
			JinEngine::JGraphic::Instance().impl->EndCommand();
		}
		void CommandInterface::FlushCommandQueue()
		{
			JinEngine::JGraphic::Instance().impl->FlushCommandQueue();
		}

		void AppAccess::Initialize()
		{
			JinEngine::JGraphic::Instance().impl->Initialize();
		}
		void AppAccess::Clear()
		{
			JinEngine::JGraphic::Instance().impl->Clear();
		}
		void AppAccess::UpdateWait()
		{
			JinEngine::JGraphic::Instance().impl->UpdateWait();
		}
		void AppAccess::UpdateGuiBackend()
		{
			JinEngine::JGraphic::Instance().impl->UpdateGuiBackend();
		}
		void AppAccess::UpdateFrame()
		{
			JinEngine::JGraphic::Instance().impl->UpdateFrame();
		}
		void AppAccess::DrawGui()
		{ 
			JinEngine::JGraphic::Instance().impl->DrawGui();
		}
		void AppAccess::DrawScene()
		{
			JinEngine::JGraphic::Instance().impl->DrawScene(); 
		}
		void AppAccess::WriteLastRsTexture()
		{
			JinEngine::JGraphic::Instance().impl->WriteLastRsTexture();
		}
#pragma endregion
	}
}