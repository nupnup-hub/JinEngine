#include"JGraphic.h" 
#include"JGraphicResourceManager.h"
#include"JGraphicConstants.h"
#include"JGraphicDrawList.h" 
#include"Utility/JDepthMapDebug.h"
#include"OcclusionCulling/JHardwareOccCulling.h"
#include"OcclusionCulling/JHZBOccCulling.h"
#include"Outline/JOutline.h"

#include"../Window/JWindows.h"
#include"FrameResource/JFrameResource.h"
#include"FrameResource/JFrameResource.h" 
#include"FrameResource/JObjectConstants.h" 
#include"FrameResource/JAnimationConstants.h" 
#include"FrameResource/JMaterialConstants.h" 
#include"FrameResource/JPassConstants.h" 
#include"FrameResource/JCameraConstants.h" 
#include"FrameResource/JLightConstants.h" 
#include"FrameResource/JShadowMapConstants.h" 
#include"FrameResource/JBoundingObjectConstants.h"

#include"../Core/Time/JGameTimer.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Object/GameObject/JGameObject.h"
#include"../Object/Component/RenderItem/JRenderItem.h"
#include"../Object/Component/Transform/JTransform.h"
#include"../Object/Component/Camera/JCamera.h"
#include"../Object/Component/Animator/JAnimator.h"
#include"../Object/Component/Light/JLight.h"

#include"../Object/Resource/JResourceData.h"
#include"../Object/Resource/JResourceManager.h"
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Resource/Mesh/JMeshGeometry.h"
#include"../Object/Resource/Material/JMaterial.h"
#include"../Object/Resource/Shader/JShader.h"
#include"../Object/Resource/Shader/JShaderData.h"
#include"../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../Object/Resource/Scene/JSceneManager.h"
#include"../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"
#include"../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../Object/Resource/Texture/JTexture.h"

#include"../Application/JApplicationVariable.h"
#include"../Utility/JCommonUtility.h"
#include"../Core/Exception/JExceptionMacro.h"	
#include"../Editor/GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Lib/DirectX/DDSTextureLoader.h" 
#include"../../Lib/imgui/imgui_impl_dx12.h"
#include"../../Lib/imgui/imgui_impl_win32.h"

#include<DirectXColors.h>
#include<functional>

namespace JinEngine
{
	using namespace DirectX;
	namespace Graphic
	{
		JGraphicInfo JGraphicImpl::GetGraphicInfo()const noexcept
		{
			return info;
		}
		JGraphicOption JGraphicImpl::GetGraphicOption()const noexcept
		{
			return option;
		}
		void JGraphicImpl::SetGraphicOption(JGraphicOption newGraphicOption)noexcept
		{
			if (newGraphicOption.isHDOcclusionAcitvated && newGraphicOption.isHZBOcclusionActivated)
			{
				if (!option.isHDOcclusionAcitvated)
					newGraphicOption.isHZBOcclusionActivated = false;
				else
					newGraphicOption.isHDOcclusionAcitvated = false;
			}
			option = newGraphicOption;
			if (option.isHDOcclusionAcitvated)
				occBase = hdOccHelper.get();
			else if (option.isHZBOcclusionActivated)
				occBase = hzbOccHelper.get();
			else
				occBase = nullptr;

			hdOccHelper->SetUpdateFrequency(option.occUpdateFrequency);
			hzbOccHelper->SetUpdateFrequency(option.occUpdateFrequency);
		}
		JGraphicDeviceInterface* JGraphicImpl::DeviceInterface()noexcept
		{
			return this;
		}
		JGraphicResourceInterface* JGraphicImpl::ResourceInterface()noexcept
		{
			return this;
		}
		JGraphicEditorInterface* JGraphicImpl::EditorInterface()noexcept
		{
			return this;
		}
		JGraphicCommandInterface* JGraphicImpl::CommandInterface()noexcept
		{
			return this;
		}
		JGraphicApplicationIterface* JGraphicImpl::AppInterface()noexcept
		{
			return this;
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetMainDepthSrvHandle()
		{
			return GetGpuSrvDescriptorHandle(graphicResource->GetSrvMainDsStart());
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetMainDepthDebugUavHandle()
		{
			return GetGpuSrvDescriptorHandle(graphicResource->GetUavMainDsDebugStart());
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetDebugSrvHandle(const uint index)
		{
			return GetGpuSrvDescriptorHandle(graphicResource->GetSrvOcclusionDebugStart() + index);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetDebugUavHandle(const uint index)
		{
			return GetGpuSrvDescriptorHandle(graphicResource->GetUavOcclusionDebugStart() + index);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetOcclusionMipMapSrvHandle(const uint index)
		{
			return GetGpuSrvDescriptorHandle(graphicResource->GetSrvOcclusionMipMapStart() + index);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetOcclusionMipMapUavHandle(const uint index)
		{
			return GetGpuSrvDescriptorHandle(graphicResource->GetUavOcclusionMipMapStart() + index);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetOcclusionDepthMapSrvHandle(const uint index)
		{
			return GetGpuSrvDescriptorHandle(graphicResource->GetSrvOcclusionDepthMapStart() + index);
		}
		void JGraphicImpl::OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
		{
			if (senderGuid == guid)
				return;

			if (eventType == Window::J_WINDOW_EVENT::WINDOW_RESIZE)
				OnResize();
		}
		ID3D12Device* JGraphicImpl::GetDevice()const noexcept
		{
			return d3dDevice.Get();
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JGraphicImpl::GetCpuSrvDescriptorHandle(int index)const noexcept
		{
			return graphicResource->GetCpuSrvDescriptorHandle(index);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicImpl::GetGpuSrvDescriptorHandle(int index)const noexcept
		{
			return graphicResource->GetGpuSrvDescriptorHandle(index);
		}
		JGraphicTextureHandle* JGraphicImpl::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)
		{
			if (!stCommand)
			{
				FlushCommandQueue();
				StartCommand();
				JGraphicTextureHandle* handle = graphicResource->Create2DTexture(uploadHeap, path, oriFormat, d3dDevice.Get(), commandList.Get());
				EndCommand();
				FlushCommandQueue();
				return handle;
			}
			else
				return graphicResource->Create2DTexture(uploadHeap, path, oriFormat, d3dDevice.Get(), commandList.Get());
		}
		JGraphicTextureHandle* JGraphicImpl::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat)
		{
			if (!stCommand)
			{
				FlushCommandQueue();
				StartCommand();
				JGraphicTextureHandle* handle = graphicResource->CreateCubeMap(uploadHeap, path, oriFormat, d3dDevice.Get(), commandList.Get());
				EndCommand();
				FlushCommandQueue();
				return handle;
			}
			else
				return graphicResource->CreateCubeMap(uploadHeap, path, oriFormat, d3dDevice.Get(), commandList.Get());
		}
		JGraphicTextureHandle* JGraphicImpl::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
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
				JGraphicTextureHandle* handle = graphicResource->CreateRenderTargetTexture(d3dDevice.Get(), textureWidth, textureHeight);
				EndCommand();
				FlushCommandQueue();
				return handle;
			}
			else
				return graphicResource->CreateRenderTargetTexture(d3dDevice.Get(), textureWidth, textureHeight);
		}
		JGraphicTextureHandle* JGraphicImpl::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
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
				JGraphicTextureHandle* handle = graphicResource->CreateShadowMapTexture(d3dDevice.Get(), textureWidth, textureHeight);
				EndCommand();
				FlushCommandQueue();
				return handle;
			}
			else
				return graphicResource->CreateShadowMapTexture(d3dDevice.Get(), textureWidth, textureHeight);
		}
		bool JGraphicImpl::DestroyGraphicTextureResource(JGraphicTextureHandle** handle)
		{
			if (*handle == nullptr)
				return false;

			if (!stCommand)
			{
				FlushCommandQueue();
				StartCommand();
				bool res = graphicResource->DestroyGraphicTextureResource(d3dDevice.Get(), handle);
				EndCommand();
				FlushCommandQueue();
				return res;
			}
			else
			{
				bool res = graphicResource->DestroyGraphicTextureResource(d3dDevice.Get(), handle);
				return res;
			}
		}
		void JGraphicImpl::StuffGraphicShaderPso(JGraphicShaderData* shaderData, J_SHADER_VERTEX_LAYOUT vertexLayout, J_GRAPHIC_SHADER_FUNCTION gFunctionFlag)
		{
			FlushCommandQueue();
			StartCommand(); 
			D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
			ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			newShaderPso.InputLayout = { shaderData->InputLayout.data(), (uint)shaderData->InputLayout.size() };
			newShaderPso.pRootSignature = mRootSignature.Get();
			newShaderPso.VS =
			{
				reinterpret_cast<BYTE*>(shaderData->Vs->GetBufferPointer()),
				shaderData->Vs->GetBufferSize()
			};
			if ((gFunctionFlag & SHADER_FUNCTION_WRITE_SHADOWMAP) == 0 && (gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) == 0)
			{
				newShaderPso.PS =
				{
					reinterpret_cast<BYTE*>(shaderData->Ps->GetBufferPointer()),
					shaderData->Ps->GetBufferSize()
				};
			}
			if (shaderData->Hs != nullptr)
			{
				newShaderPso.HS =
				{
					reinterpret_cast<BYTE*>(shaderData->Hs->GetBufferPointer()),
					shaderData->Hs->GetBufferSize()
				};
			}
			if (shaderData->Ds != nullptr)
			{
				newShaderPso.DS =
				{
					reinterpret_cast<BYTE*>(shaderData->Ds->GetBufferPointer()),
					shaderData->Ds->GetBufferSize()
				};
			}
			if (shaderData->Gs != nullptr)
			{
				newShaderPso.GS =
				{
					reinterpret_cast<BYTE*>(shaderData->Gs->GetBufferPointer()),
					shaderData->Gs->GetBufferSize()
				};
			}

			newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); 
			newShaderPso.DepthStencilState.StencilEnable = true; 
			newShaderPso.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			
			if ((gFunctionFlag & SHADER_FUNCTION_SKY) > 0)
			{ 
				newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; 
				//newShaderPso.DepthStencilState.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
				newShaderPso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; 
				newShaderPso.DepthStencilState.StencilEnable = false;
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			}
			if ((gFunctionFlag & SHADER_FUNCTION_NONCULLING) > 0)
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

			newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			newShaderPso.SampleMask = UINT_MAX;
			newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			newShaderPso.NumRenderTargets = 1;
			newShaderPso.RTVFormats[0] = graphicResource->backBufferFormat;
			newShaderPso.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			newShaderPso.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			newShaderPso.DSVFormat = graphicResource->depthStencilFormat;
			 
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
				newShaderPso.DepthStencilState.StencilEnable = false; 
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
				newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			}
			ThrowIfFailedG(d3dDevice->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->Pso.GetAddressOf())));

			EndCommand();
			FlushCommandQueue();
		}
		void JGraphicImpl::StuffComputeShaderPso(JComputeShaderData* shaderData, J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
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
				reinterpret_cast<BYTE*>(shaderData->Cs->GetBufferPointer()),
				shaderData->Cs->GetBufferSize()
			};
			newShaderPso.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			ThrowIfFailedG(d3dDevice->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->Pso.GetAddressOf())));

			EndCommand();
			FlushCommandQueue();
		}
		ID3D12CommandQueue* JGraphicImpl::GetCommandQueue()const noexcept
		{
			return commandQueue.Get();
		}
		ID3D12CommandAllocator* JGraphicImpl::GetCommandAllocator()const noexcept
		{
			return directCmdListAlloc.Get();
		}
		ID3D12GraphicsCommandList* JGraphicImpl::GetCommandList()const noexcept
		{
			return commandList.Get();
		}
		void JGraphicImpl::StartCommand()
		{
			if (!stCommand)
			{
				ThrowIfFailedHr(commandList->Reset(directCmdListAlloc.Get(), nullptr));
				stCommand = true;
			}
		}
		void JGraphicImpl::EndCommand()
		{
			if (stCommand)
			{
				ThrowIfFailedG(commandList->Close());
				ID3D12CommandList* cmdsLists[] = { commandList.Get() };
				commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
				stCommand = false;
			}
		}
		void JGraphicImpl::FlushCommandQueue()
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
		void JGraphicImpl::SetImGuiBackEnd()
		{
			ImGui_ImplDX12_Init(d3dDevice.Get(), Constant::gNumFrameResources,
				DXGI_FORMAT_R8G8B8A8_UNORM, graphicResource->srvHeap.Get(),
				graphicResource->srvHeap->GetCPUDescriptorHandleForHeapStart(),
				graphicResource->srvHeap->GetGPUDescriptorHandleForHeapStart());
		}
		void JGraphicImpl::Initialize()
		{
			AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);

			info.width = JWindow::Instance().GetClientWidth();
			info.height = JWindow::Instance().GetClientHeight();

			graphicResource = std::make_unique<JGraphicResourceManager>();
			hdOccHelper = std::make_unique<JHardwareOccCulling>();
			hzbOccHelper = std::make_unique<JHZBOccCulling>();
			depthMapDebug = std::make_unique<JDepthMapDebug>();
			outlineHelper = std::make_unique<JOutline>();

			LoadData();
			InitializeD3D();
			InitializeResource();
			OnResize();
		}
		void JGraphicImpl::Clear()
		{
			if (JApplicationVariable::GetApplicationState() != J_APPLICATION_STATE::PROJECT_SELECT)
				StoreData();

			FlushCommandQueue();

			mRootSignature.Reset();
			graphicResource->Clear();
			graphicResource.reset();

			outlineHelper->Clear();
			outlineHelper.reset();

			depthMapDebug->Clear();
			depthMapDebug.reset();

			hdOccHelper->Clear();
			hdOccHelper.reset();

			hzbOccHelper->Clear();
			hzbOccHelper.reset();
			occBase = nullptr;

			frameResources.clear();
			currFrameResource = nullptr;
			currFrameResourceIndex = 0;

			FlushCommandQueue();

			fence.Reset();
			currentFence = 0;

			commandQueue.Reset();
			directCmdListAlloc.Reset();
			commandList.Reset();

			m4xMsaaState = false;
			m4xMsaaQuality = 0;

			swapChain.Reset();
			dxgiFactory.Reset();
			d3dDevice.Reset();

			screenViewport = D3D12_VIEWPORT();
			scissorRect = D3D12_RECT();

			currBackBuffer = 0;
			info.width = 0;
			info.height = 0;
			RemoveListener(*JWindow::Instance().EvInterface(), guid);
		}
		void JGraphicImpl::StartFrame()
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
		void JGraphicImpl::EndFrame()
		{
			ImGui::Render();
			 
			ResourceTransition(graphicResource->mainDepthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			ResourceTransition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
			D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(0);
			commandList->ClearRenderTargetView(rtv, Colors::DimGray, 0, nullptr);
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			commandList->OMSetRenderTargets(1, &rtv, true, &dsv);

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

			ResourceTransition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);		 
			ResourceTransition(graphicResource->mainDepthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
			ThrowIfFailedG(commandList->Close());
			ID3D12CommandList* cmdsLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

			//ImGuiIO& io = ImGui::GetIO();
			//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			//{
			//	ImGui::UpdatePlatformWindows();
			//	ImGui::RenderPlatformWindowsDefault(NULL, (void*)cmdsLists);
			//}

			// Swap the back and front buffers
			ThrowIfFailedG(swapChain->Present(0, 0));
			currBackBuffer = (currBackBuffer + 1) % graphicResource->swapChainBufferCount;
			// Advance the fence value to mark commands up to this fence point.
			currFrameResource->fence = ++currentFence;

			// Add an instruction to the command queue to set a new fence point. 
			// Because we are on the GPU timeline, the new fence point won't be 
			// set untwil the GPU finishes processing all the commands prior to this Signal().
			commandQueue->Signal(fence.Get(), currentFence);
		}
		void JGraphicImpl::UpdateWait()
		{
			currFrameResourceIndex = (currFrameResourceIndex + 1) % Constant::gNumFrameResources;
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
		void JGraphicImpl::UpdateEngine()
		{
			if (option.IsHZBOccActivated())
				hzbOccHelper->ReadCullingResult();

			updateHelper.Clear();
			for (uint i = 0; i < (uint)J_UPLOAD_RESOURCE_TYPE::COUNT; ++i)
			{
				updateHelper.uData[i].count = (*updateHelper.uData[i].getElementCountCallable)(nullptr);
				updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_RESOURCE_TYPE)i);
				updateHelper.uData[i].rebuildCondition = IsPassRedefineCapacity(updateHelper.uData[i].capacity, updateHelper.uData[i].count);
				updateHelper.hasRebuildCondition |= (bool)updateHelper.uData[i].rebuildCondition;
			}
			for (uint i = 0; i < (uint)J_GRAPHIC_TEXTURE_TYPE::COUNT; ++i)
			{
				if (updateHelper.bData[i].HasCallable())
				{
					updateHelper.bData[i].count = (*updateHelper.bData[i].getTextureCountCallable)(nullptr, *this);
					updateHelper.bData[i].capacity = (*updateHelper.bData[i].getTextureCapacityCallable)(nullptr, *this);
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
				for (uint i = 0; i < (uint)J_UPLOAD_RESOURCE_TYPE::COUNT; ++i)
				{
					if (updateHelper.uData[i].rebuildCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
					{
						ReBuildFrameResource((J_UPLOAD_RESOURCE_TYPE)i, updateHelper.uData[i].rebuildCondition, updateHelper.uData[i].count);
						updateHelper.uData[i].setFrameDirty = true;
						updateHelper.uData[i].capacity = currFrameResource->GetElementCount((J_UPLOAD_RESOURCE_TYPE)i);
					}
				}
				//Has sequency dependency
				updateHelper.WriteGraphicInfo(info);
				updateHelper.NotifyUpdateFrameCapacity(*this);	//use graphic info
				if (updateHelper.hasRecompileShader)
					ReCompileGraphicShader();		//use graphic info
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
				drawTarget->scene->SpaceSpatialInterface()->ViewCulling();
				drawTarget->updateInfo->UpdateStart();
				UpdateSceneObjectCB(drawTarget->scene, drawTarget->updateInfo->objUpdateCount, drawTarget->updateInfo->hotObjUpdateCount);
				UpdateScenePassCB(drawTarget->scene);	//always update
				UpdateSceneAnimationCB(drawTarget->scene, drawTarget->updateInfo->aniUpdateCount, drawTarget->updateInfo->hotAniUpdateCount);
				UpdateSceneCameraCB(drawTarget->scene, drawTarget->updateInfo->camUpdateCount, drawTarget->updateInfo->hotCamUpdateCount);
				UpdateSceneLightCB(drawTarget->scene, drawTarget->updateInfo->lightUpdateCount, drawTarget->updateInfo->hotLitghtUpdateCount);	//always update
				if (isAllowOcclusion)
				{
					if (option.IsHZBOccActivated())
					{
						const uint queryCount = drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
						hzbOccHelper->UpdatePass(drawTarget->scene, info, option, queryCount, updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::PASS].offset - 1);
					}
				}

				drawTarget->updateInfo->UpdateEnd();
			}
			UpdateMaterialCB();
			if (occBase != nullptr)
				occBase->UpdateTimer();
		}
		void JGraphicImpl::UpdateSceneObjectCB(_In_ JScene* scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
		{
			const bool isUpdateBoundingObj = scene->IsMainScene() && option.isOcclusionQueryActivated;
			uint addOffset = 0;
			const std::vector<JComponent*>& jRvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
			const uint renderItemCount = (uint)jRvec.size();

			JObjectConstants objectConstants;
			auto currObjectCB = currFrameResource->objectCB.get();

			JBoundingObjectConstants boundingConstants;
			auto currBoundingObjectCB = currFrameResource->bundingObjectCB.get();

			const uint meshOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].offset;
			const uint rItemOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT].offset;
			const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].setFrameDirty;

			for (uint i = 0; i < renderItemCount; ++i)
			{
				JRenderItem* renderItem = static_cast<JRenderItem*>(jRvec[i]);
				JTransform* transform = renderItem->GetOwner()->GetTransform();

				IFrameDirty* rFrameDirty = renderItem;
				if (forcedSetFrameDirty)
					rFrameDirty->SetFrameDirty();

				const bool isDirted = rFrameDirty->IsFrameDirted();
				if (isDirted)
				{
					JRenderItem::IFrameBase2* rFrameBase2 = renderItem;
					rFrameBase2->UpdateFrame(boundingConstants);
					currBoundingObjectCB->CopyData(rItemOffset + i, boundingConstants);
					hzbOccHelper->UpdateObject(renderItem, rItemOffset + i);
				}
				const uint submeshCount = renderItem->GetSubmeshCount();
				for (uint j = 0; j < submeshCount; ++j)
				{
					if (isDirted)
					{
						JRenderItem::IFrameBase1* rFrameBase1 = renderItem; 
						rFrameBase1->UpdateFrame(objectConstants, j);
						currObjectCB->CopyData(meshOffset + CallGetFirstFrameBuffOffset(*renderItem) + j, objectConstants);
						++updateCount;
					}
				}
				if (isDirted)
				{
					if (rFrameDirty->GetFrameDirty() == Constant::gNumFrameResources)
						++hotUpdateCount;
					renderItem->CallUpdateEnd();
				}
				addOffset += submeshCount;
			}

			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT].offset += renderItemCount;
			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::OBJECT].offset += addOffset;
		}
		void JGraphicImpl::UpdateMaterialCB()
		{
			auto currMaterialBuffer = currFrameResource->materialBuffer.get();
			uint matCount;
			const std::vector<JResourceObject*>::const_iterator matBegin = JResourceManager::Instance().GetResourceVectorHandle<JMaterial>(matCount);
			for (uint i = 0; i < matCount; ++i)
			{
				JMaterialConstants materialConstant;
				JMaterial* material = static_cast<JMaterial*>(*(matBegin + i));
				IFrameDirty* mFrameDirty = material;
				if (mFrameDirty->IsFrameDirted())
				{
					JMaterial::IFrameBase1* mFrameBase1 = material;
					mFrameBase1->UpdateFrame(materialConstant);
					material->CallUpdateEnd();
					currMaterialBuffer->CopyData(CallGetFrameBuffOffset(*material), materialConstant);
				}
			};
		}
		void JGraphicImpl::UpdateScenePassCB(_In_ JScene* scene)
		{
			JPassConstants passContants;
			passContants.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
			passContants.totalTime = JGameTimer::Instance().TotalTime();
			passContants.deltaTime = JGameTimer::Instance().DeltaTime();

			auto currPassCB = currFrameResource->passCB.get();
			currPassCB->CopyData(updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::PASS].offset, passContants);
			++updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::PASS].offset;
		}
		void JGraphicImpl::UpdateSceneAnimationCB(_In_ JScene* scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
		{
			const std::vector<JComponent*>& jAvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
			const uint animatorCount = (uint)jAvec.size();

			auto currSkinnedCB = currFrameResource->skinnedCB.get();
			JAnimationConstants animationConstatns;

			const uint offset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].offset;
			const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].setFrameDirty;

			if (scene->IsAnimatorActivated())
			{
				for (uint i = 0; i < animatorCount; ++i)
				{
					JAnimator* animator = static_cast<JAnimator*>(jAvec[i]);
					IFrameDirty* aFrameDirty = animator;
					if (forcedSetFrameDirty)
						aFrameDirty->SetFrameDirty();
					if (aFrameDirty->IsFrameDirted())
					{
						JAnimator::IFrameBase1* aFrameBase1 = animator;
						aFrameBase1->UpdateFrame(animationConstatns);
						currSkinnedCB->CopyData(offset + i, animationConstatns);
						if (aFrameDirty->GetFrameDirty() == Constant::gNumFrameResources)
							++hotUpdateCount;
						++updateCount;
						animator->CallUpdateEnd();
					}
				}
			}
			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::ANIMATION].offset += animatorCount;
		}
		void JGraphicImpl::UpdateSceneCameraCB(_In_ JScene* scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
		{
			const std::vector<JComponent*>& jCvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			const uint cameraCount = (uint)jCvec.size();

			auto currCameraCB = currFrameResource->cameraCB.get();
			JCameraConstants camContants;

			const uint offset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].offset;
			const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].setFrameDirty;

			for (uint i = 0; i < cameraCount; ++i)
			{
				JCamera* camera = static_cast<JCamera*>(jCvec[i]);
				IFrameDirty* cFrameDirty = camera;
				if (forcedSetFrameDirty)
					cFrameDirty->SetFrameDirty();

				if (cFrameDirty->IsFrameDirted())
				{
					JCamera::IFrameBase1* cFrameBase1 = camera;
					cFrameBase1->UpdateFrame(camContants);
					currCameraCB->CopyData(offset + i, camContants);
					if (cFrameDirty->GetFrameDirty() == Constant::gNumFrameResources)
						++hotUpdateCount;
					++updateCount;
					camera->CallUpdateEnd();
				}
			}
			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::CAMERA].offset += cameraCount;
		}
		void JGraphicImpl::UpdateSceneLightCB(_In_ JScene* scene, _Out_ uint& updateCount, _Out_ uint& hotUpdateCount)
		{
			const std::vector<JComponent*>& jLvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
			const uint lightVecCount = (uint)jLvec.size();

			auto currLightCB = currFrameResource->lightBuffer.get();
			auto currLightIndexCB = currFrameResource->lightIndexCB.get();
			auto currSMLightCB = currFrameResource->smLightBuffer.get();
			auto currShadowCalCB = currFrameResource->shadowCalCB.get();

			JLightConstants lightConstants;
			JLightIndexConstants lightIndexConstants;
			JShadowMapLightConstants smLightConstants;
			JShadowMapConstants shadowCalConstant;

			uint lightOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset;
			uint lightIndexOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX].offset;
			uint shadowLitOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset;
			uint shadowMapOffset = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP].offset;

			uint litCount = 0;
			uint smLitCount = 0;

			bool hasLitUpdate = false;
			const bool forcedSetFrameDirty = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].setFrameDirty |
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX].setFrameDirty |
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].setFrameDirty |
				updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP].setFrameDirty;

			for (uint i = 0; i < lightVecCount; ++i)
			{
				JLight* light = static_cast<JLight*>(jLvec[i]);
				IFrameDirty* lFrameDirty = light;
				const bool onShadow = light->IsShadowActivated();

				if (forcedSetFrameDirty)
					lFrameDirty->SetFrameDirty();

				if (lFrameDirty->IsFrameDirted())
				{ 
					if (onShadow)
					{
						JLight::IFrameBase2* lFrameBase2 = light;
						JLight::IFrameBase3* lFrameBase3 = light;
						lFrameBase2->UpdateFrame(smLightConstants);
						lFrameBase3->UpdateFrame(shadowCalConstant);
						currShadowCalCB->CopyData(shadowMapOffset + smLitCount, shadowCalConstant);
						currSMLightCB->CopyData(shadowMapOffset + smLitCount, smLightConstants);
					}
					else
					{
						JLight::IFrameBase1* lFrameBase1 = light;
						lFrameBase1->UpdateFrame(lightConstants);
						currLightCB->CopyData(lightOffset + litCount, lightConstants);
					}
					if (lFrameDirty->GetFrameDirty() == Constant::gNumFrameResources)
						++hotUpdateCount;
					++updateCount;
					light->CallUpdateEnd();
				}
				if (onShadow)
					++smLitCount;
				else
					++litCount;
			}

			lightIndexConstants.litStIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset;
			lightIndexConstants.litEdIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset + litCount;
			lightIndexConstants.shadwMapStIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset;
			lightIndexConstants.shadowMapEdIndex = updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset + smLitCount;
			currLightIndexCB->CopyData(lightIndexOffset, lightIndexConstants);

			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT].offset += litCount;
			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX].offset += 1;
			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT].offset += smLitCount;
			updateHelper.uData[(int)J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP].offset += smLitCount;
		}
		void JGraphicImpl::DrawScene()
		{
			auto cmdListAlloc = currFrameResource->cmdListAlloc;
			ThrowIfFailedHr(cmdListAlloc->Reset());
			ThrowIfFailedHr(commandList->Reset(cmdListAlloc.Get(), nullptr));

			commandList->SetGraphicsRootSignature(mRootSignature.Get());
			//Test Code
			commandList->OMSetStencilRef(Constant::commonStencilRef);

			ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResource->srvHeap.Get() };
			commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			commandList->RSSetViewports(1, &screenViewport);
			commandList->RSSetScissorRects(1, &scissorRect);

			auto lightBuffer = currFrameResource->lightBuffer->Resource();
			commandList->SetGraphicsRootShaderResourceView(7, lightBuffer->GetGPUVirtualAddress());

			auto shadowLightBuffer = currFrameResource->smLightBuffer->Resource();
			commandList->SetGraphicsRootShaderResourceView(8, shadowLightBuffer->GetGPUVirtualAddress());

			auto matBuffer = currFrameResource->materialBuffer->Resource();
			commandList->SetGraphicsRootShaderResourceView(9, matBuffer->GetGPUVirtualAddress());

			JTexture* skyTxt = JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::DEFAULT_SKY);

			CD3DX12_GPU_DESCRIPTOR_HANDLE cubeMapHandle = graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvUserCubeMapStart());
			commandList->SetGraphicsRootDescriptorTable(10, cubeMapHandle);

			CD3DX12_GPU_DESCRIPTOR_HANDLE texture2DHandle = graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvUser2DTextureStart());
			commandList->SetGraphicsRootDescriptorTable(11, texture2DHandle);

			CD3DX12_GPU_DESCRIPTOR_HANDLE shadowHandle = graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvShadowMapStart());
			commandList->SetGraphicsRootDescriptorTable(12, shadowHandle);

			//수정필요
			//Shadow Map Draw
			//Update가 있는 광원만 추적해서 Draw
			//광원내에 있는 오브젝트 검색을 위한 공간분할 및 검색 필요
			 
			const uint drawListCount = JGraphicDrawList::GetListCount();
			std::vector<JGraphicDrawHelper> occlusionCash;
			JGraphicDrawHelper helper;
			for (uint i = 0; i < drawListCount; ++i)
			{
				JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
				if (drawTarget->updateInfo->hasShadowUpdate)
				{
					const uint shadowReqCount = (uint)drawTarget->shadowRequestor.size();
					for (uint j = 0; j < shadowReqCount; ++j)
					{
						JGraphicDrawHelper copiedHelper = helper;
						copiedHelper.scene = drawTarget->scene;
						copiedHelper.lit = static_cast<JLight*>(drawTarget->shadowRequestor[j]->jLight);
						copiedHelper.shadowOffset += j;
						//copiedHelper.isDrawShadowMap = true;
						DrawSceneShadowMap(copiedHelper);
					}
				}
				if (drawTarget->updateInfo->hasSceneUpdate)
				{
					const uint sceneReqCount = (uint)drawTarget->sceneRequestor.size();
					for (uint j = 0; j < sceneReqCount; ++j)
					{ 
						JGraphicDrawHelper copiedHelper = helper;
						copiedHelper.scene = drawTarget->scene;
						copiedHelper.cam = static_cast<JCamera*>(drawTarget->sceneRequestor[j]->jCamera);
						copiedHelper.camOffset += j;

						const bool isMainTarget = copiedHelper.scene->IsMainScene() && copiedHelper.cam->IsMainCamera();
						const bool canCullingStart = occBase != nullptr && occBase->CanCullingStart();

						DrawSceneRenderTarget(copiedHelper);
						if (isMainTarget && canCullingStart)
							occlusionCash.push_back(copiedHelper);

						if (isMainTarget)
						{
							if (option.allowOutline)
							{
								outlineHelper->DrawOutline(commandList.Get(),
									graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvMainDsStart()),
									graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvMainDsStart() + 1));
							}
							 
							depthMapDebug->DrawNonLinearDepthDebug(commandList.Get(),
								graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvMainDsStart()),
								graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetUavMainDsDebugStart()),
								JVector2<uint>(info.width, info.height),
								copiedHelper.cam->GetNear(),
								copiedHelper.cam->GetFar());
						}
					}
				}
				helper.objectMeshOffset += drawTarget->scene->GetMeshCount();
				helper.objectRitemOffset += drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				helper.passOffset += 1;
				helper.aniOffset += drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
				helper.camOffset += drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
				helper.litIndexOffset += 1;
				helper.shadowOffset += (uint)drawTarget->shadowRequestor.size();
			}
			const uint occlusionCount = (uint)occlusionCash.size();
			for (uint i = 0; i < occlusionCount; ++i)
				DrawOcclusionDepthMap(occlusionCash[i]);
		}
		void JGraphicImpl::DrawProjectSelector()
		{
			auto cmdListAlloc = currFrameResource->cmdListAlloc;
			ThrowIfFailedHr(cmdListAlloc->Reset());
			ThrowIfFailedHr(commandList->Reset(cmdListAlloc.Get(), nullptr));

			commandList->SetGraphicsRootSignature(mRootSignature.Get());

			ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResource->srvHeap.Get() };
			commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			commandList->RSSetViewports(1, &screenViewport);
			commandList->RSSetScissorRects(1, &scissorRect);
		}
		void JGraphicImpl::DrawSceneRenderTarget(const JGraphicDrawHelper helper)
		{
			const uint rtvVecIndex = CallGetTxtVectorIndex(*helper.cam);
			const uint rtvHeapIndex = CallGetTxtRtvHeapIndex(*helper.cam);

			commandList->RSSetViewports(1, &screenViewport);
			commandList->RSSetScissorRects(1, &scissorRect);

			ResourceTransition(graphicResource->mainDepthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResource->GetCpuRtvDescriptorHandle(rtvHeapIndex);
			CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(0);

			commandList->ClearRenderTargetView(rtv, Colors::DarkGray, 0, nullptr);
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			commandList->OMSetRenderTargets(1, &rtv, true, &dsv);

			currFrameResource->passCB->SetGraphicCBBufferView(commandList.Get(), 2, helper.passOffset);
			currFrameResource->cameraCB->SetGraphicCBBufferView(commandList.Get(), 3, helper.camOffset);
			currFrameResource->lightIndexCB->SetGraphicCBBufferView(commandList.Get(), 4, helper.litIndexOffset);

			commandList->OMSetStencilRef(2);
			DrawGameObject(commandList.Get(), helper.scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC), helper, false, false, true);
			DrawGameObject(commandList.Get(), helper.scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED), helper, false, helper.scene->IsAnimatorActivated(), true);
			if (option.IsHDOccActivated())
				commandList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
			commandList->OMSetStencilRef(Constant::commonStencilRef);
			DrawGameObject(commandList.Get(), helper.scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_LAYER, J_MESHGEOMETRY_TYPE::STATIC), helper, false, false, false);
			DrawGameObject(commandList.Get(), helper.scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::SKY, J_MESHGEOMETRY_TYPE::STATIC), helper, false, false, false);

			ResourceTransition(graphicResource->mainDepthStencil.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		}
		void JGraphicImpl::DrawSceneShadowMap(const JGraphicDrawHelper helper)
		{
			const uint shadowWidth = CallGetTxtWidth(*helper.lit);
			const uint shadowHeight = CallGetTxtHeight(*helper.lit);
			const uint rVecIndex = CallGetTxtVectorIndex(*helper.lit);
			const uint dsvHeapIndex = CallGetTxtDsvHeapIndex(*helper.lit);

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)shadowWidth, (float)shadowHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, shadowWidth, shadowHeight };

			commandList->RSSetViewports(1, &mViewport);
			commandList->RSSetScissorRects(1, &mScissorRect);

			ResourceTransition(graphicResource->shadowMapResource[rVecIndex].Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			
			D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

			currFrameResource->passCB->SetGraphicCBBufferView(commandList.Get(), 2, helper.passOffset);
			currFrameResource->shadowCalCB->SetGraphicCBBufferView(commandList.Get(), 5, helper.shadowOffset);

			DrawGameObject(commandList.Get(), helper.scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC), helper, true, false, false);
			DrawGameObject(commandList.Get(), helper.scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED), helper, true, helper.scene->IsAnimatorActivated(), false);

			ResourceTransition(graphicResource->shadowMapResource[rVecIndex].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		void JGraphicImpl::DrawOcclusionDepthMap(const JGraphicDrawHelper helper)
		{
			//commandList->SetGraphicsRootSignature(mRootSignature.Get());

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)info.occlusionWidth, (float)info.occlusionHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, info.occlusionWidth, info.occlusionHeight };

			commandList->RSSetViewports(1, &mViewport);
			commandList->RSSetScissorRects(1, &mScissorRect);

			ResourceTransition(graphicResource->occlusionDepthMap.Get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(graphicResource->GetOcclusionDsIndex());
			commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

			currFrameResource->cameraCB->SetGraphicCBBufferView(commandList.Get(), 3, helper.camOffset);
			DrawSceneBoundingBox(commandList.Get(), helper.scene->SpaceSpatialInterface()->GetAlignedObject(helper.scene->GetMainCamera()->GetBoundingFrustum()), helper, helper.scene->IsAnimatorActivated());
 
			ResourceTransition(graphicResource->occlusionDepthMap.Get(),D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);

			if (option.IsHZBOccActivated())
			{
				hzbOccHelper->DepthMapDownSampling(commandList.Get(),
					graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvOcclusionDepthMapStart()),
					graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvOcclusionMipMapStart()),
					graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetUavOcclusionMipMapStart()),
					info.occlusionMapCount,
					graphicResource->cbvSrvUavDescriptorSize);
				hzbOccHelper->OcclusuinCulling(commandList.Get(), graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvOcclusionMipMapStart()));

				if (option.allowHZBDepthMapDebug)
				{
					JVector2<uint> occlusionSize = JVector2<uint>(info.occlusionWidth, info.occlusionHeight);
					const float camNear = helper.cam->GetNear();
					const float camFar = helper.cam->GetFar();

					for (uint i = 0; i < graphicResource->occlusionCount; ++i)
					{
						depthMapDebug->DrawLinearDepthDebug(commandList.Get(),
							graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvOcclusionMipMapStart() + i),
							graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetUavOcclusionDebugStart() + i),
							occlusionSize,
							camNear,
							camFar);
						occlusionSize /= 2;
					}
				}
			}
			else if (option.IsHDOccActivated())
			{
				CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->GetOcclusionResult(), D3D12_RESOURCE_STATE_PREDICATION, D3D12_RESOURCE_STATE_COPY_DEST);
				commandList->ResourceBarrier(1, &rsBarrier);
				commandList->ResolveQueryData(graphicResource->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, 0, graphicResource->GetOcclusionQueryHeapCapacity(), graphicResource->GetOcclusionResult(), 0);
				rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->GetOcclusionResult(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PREDICATION);
				commandList->ResourceBarrier(1, &rsBarrier);
			}
		}
		void JGraphicImpl::DrawGameObject(ID3D12GraphicsCommandList* commandList,
			const std::vector<JGameObject*>& gameObject,
			const JGraphicDrawHelper helper,
			const bool isDrawShadowMap,
			const bool isAnimationActivated,
			const bool allowOcclusion)
		{
			const bool isOcclusionActivated = allowOcclusion && option.isOcclusionQueryActivated;

			JShader* shadowShader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SHADOW_MAP_SHADER);

			uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
			uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

			auto objectCB = currFrameResource->objectCB->Resource();
			auto skinCB = currFrameResource->skinnedCB->Resource();
			const uint gameObjCount = (uint)gameObject.size();

			for (uint i = 0; i < gameObjCount; ++i)
			{
				JRenderItem* renderItem = gameObject[i]->GetRenderItem();
				if (!renderItem->IsVisible())
					continue;

				JMeshGeometry* mesh = renderItem->GetMesh();
				const D3D12_VERTEX_BUFFER_VIEW vertexPtr = mesh->VertexBufferView();
				const D3D12_INDEX_BUFFER_VIEW indexPtr = mesh->IndexBufferView();

				commandList->IASetVertexBuffers(0, 1, &vertexPtr);
				commandList->IASetIndexBuffer(&indexPtr);
				commandList->IASetPrimitiveTopology(renderItem->GetPrimitiveType());

				const JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>();
				const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

				for (uint j = 0; j < submeshCount; ++j)
				{
					const uint finalObjOffset = (helper.objectMeshOffset + CallGetFirstFrameBuffOffset(*renderItem) + j);
					if (isOcclusionActivated && option.IsHZBOccActivated() && hzbOccHelper->IsCulled(finalObjOffset))
						continue;

					const JMaterial* mat = renderItem->GetValidMaterial(j);
					const JShader* shader = mat->GetShader();
					const size_t shaderGuid = shader->GetGuid();
					const bool onSkinned = animator != nullptr && isAnimationActivated;
					if (!isDrawShadowMap)
					{
						if (onSkinned)
							commandList->SetPipelineState(shader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::SKINNED)));
						else
							commandList->SetPipelineState(shader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::STATIC)));
					}
					else
					{
						if (onSkinned)
							commandList->SetPipelineState(shadowShader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::SKINNED)));
						else
							commandList->SetPipelineState(shadowShader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::STATIC)));
					}

					D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + finalObjOffset * objectCBByteSize;
					commandList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
					if (onSkinned)
					{
						D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + (helper.aniOffset + i) * skinCBByteSize;
						commandList->SetGraphicsRootConstantBufferView(1, skinObjCBAddress);
					}
					if (isOcclusionActivated && option.IsHDOccActivated())
						commandList->SetPredication(graphicResource->GetOcclusionResult(), finalObjOffset * 8, D3D12_PREDICATION_OP_EQUAL_ZERO);
					
					commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
				}
			}
		}
		void JGraphicImpl::DrawSceneBoundingBox(ID3D12GraphicsCommandList* commandList,
			const std::vector<JGameObject*>& gameObject,
			const JGraphicDrawHelper helper,
			const bool isAnimationActivated)
		{
			//JMeshGeometry* mesh = JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE);
			JMeshGeometry* mesh = JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE);
			JMaterial* mat = JResourceManager::Instance().GetDefaultMaterial(J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST);
			JShader* shader = mat->GetShader();

			const D3D12_VERTEX_BUFFER_VIEW vertexPtr = mesh->VertexBufferView();
			const D3D12_INDEX_BUFFER_VIEW indexPtr = mesh->IndexBufferView();

			commandList->IASetVertexBuffers(0, 1, &vertexPtr);
			commandList->IASetIndexBuffer(&indexPtr);
			commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			uint boundingObjectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JBoundingObjectConstants));
			auto boundingObjectCB = currFrameResource->bundingObjectCB->Resource();

			uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));
			auto skinCB = currFrameResource->skinnedCB->Resource();

			const uint gameObjCount = (uint)gameObject.size();
			for (uint i = 0; i < gameObjCount; ++i)
			{
				JRenderItem* renderItem = gameObject[i]->GetRenderItem();
				if (!renderItem->IsVisible())
					continue;

				JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>();
				const bool onSkinned = animator != nullptr && isAnimationActivated;

				if (onSkinned)
					commandList->SetPipelineState(shader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::SKINNED)));
				else
					commandList->SetPipelineState(shader->GetGraphicPso(JShaderType::ConvertToVertexLayout(J_MESHGEOMETRY_TYPE::STATIC)));

				const uint index = helper.objectMeshOffset + CallGetSecondFrameBuffOffset(*renderItem);
				D3D12_GPU_VIRTUAL_ADDRESS boundingObjectCBAddress = boundingObjectCB->GetGPUVirtualAddress() + index * boundingObjectCBByteSize;
				commandList->SetGraphicsRootConstantBufferView(6, boundingObjectCBAddress);

				if (onSkinned)
				{
					D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + (helper.aniOffset + i) * skinCBByteSize;
					commandList->SetGraphicsRootConstantBufferView(1, skinObjCBAddress);
				}
				if (option.isHZBOcclusionActivated)
					commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
				else
				{
					commandList->BeginQuery(graphicResource->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
					commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
					commandList->EndQuery(graphicResource->GetOcclusionQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION, index);
				}
			}
		}
		void JGraphicImpl::ResourceTransition(_In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
		{
			CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, stateBefore, stateAfter);
			commandList->ResourceBarrier(1, &rsBarrier);
		}
		bool JGraphicImpl::InitializeD3D()
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

			// Check 4X MSAA quality support for our back buffer format.
			// All Direct3D 11 capable devices support 4X MSAA for all render 
			// target formats, so we only need to check quality support.

			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
			msQualityLevels.Format = graphicResource->backBufferFormat;
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
			graphicResource->rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			graphicResource->dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			graphicResource->cbvSrvUavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			return true;
		}
		bool JGraphicImpl::InitializeResource()
		{
			FlushCommandQueue();
			StartCommand();
			BuildRootSignature();
			graphicResource->BuildRtvDescriptorHeaps(d3dDevice.Get());
			graphicResource->BuildDsvDescriptorHeaps(d3dDevice.Get());
			graphicResource->BuildSrvDescriptorHeaps(d3dDevice.Get());
			graphicResource->BuildOccQueryHeaps(d3dDevice.Get());
			graphicResource->CreateOcclusionQueryResource(d3dDevice.Get());
			graphicResource->CreateOcclusionHZBResource(d3dDevice.Get(), commandList.Get(), info.occlusionWidth, info.occlusionHeight);
			graphicResource->CreateMainDepthStencilResource(d3dDevice.Get(), commandList.Get(), info.width, info.height, m4xMsaaState, m4xMsaaQuality);
			BuildFrameResources();
			hzbOccHelper->Initialize(d3dDevice.Get(), info);
			depthMapDebug->Initialize(d3dDevice.Get(), graphicResource->backBufferFormat, graphicResource->depthStencilFormat);
			outlineHelper->Initialize(d3dDevice.Get(), graphicResource->backBufferFormat, graphicResource->depthStencilFormat, info.width, info.height, Constant::commonStencilRef);
			EndCommand();
			FlushCommandQueue();
			return true;
		}
		void JGraphicImpl::LogAdapters()
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
		void JGraphicImpl::LogAdapterOutputs(IDXGIAdapter* adapter)
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

				LogOutputDisplayModes(output, graphicResource->backBufferFormat);

				ReleaseCom(output);

				++i;
			}
		}
		void JGraphicImpl::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
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
		void JGraphicImpl::CreateCommandObjects()
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			ThrowIfFailedHr(d3dDevice->CreateCommandQueue(
				&queueDesc,
				IID_PPV_ARGS(&commandQueue)));

			ThrowIfFailedHr(d3dDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(directCmdListAlloc.GetAddressOf())));


			ThrowIfFailedHr(d3dDevice->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				directCmdListAlloc.Get(),
				nullptr,
				IID_PPV_ARGS(commandList.GetAddressOf())));

			commandList->Close();

		}
		void JGraphicImpl::CreateSwapChain()
		{
			swapChain.Reset();
			DXGI_SWAP_CHAIN_DESC sd;
			sd.BufferDesc.Width = info.width;
			sd.BufferDesc.Height = info.height;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferDesc.Format = graphicResource->backBufferFormat;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = graphicResource->swapChainBufferCount;
			sd.OutputWindow = JWindow::Instance().HandleInterface()->GetHandle();
			sd.Windowed = true;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			// Note: Swap chain uses queue to perform flush.

			ThrowIfFailedHr(dxgiFactory->CreateSwapChain(
				commandQueue.Get(),
				&sd,
				swapChain.GetAddressOf()));
		}
		void JGraphicImpl::BuildRootSignature()
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

			// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
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
		}
		void JGraphicImpl::BuildFrameResources()
		{
			for (int i = 0; i < Constant::gNumFrameResources; ++i)
				frameResources.push_back(std::make_unique<JFrameResource>(d3dDevice.Get(), info));
			currFrameResource = frameResources[currFrameResourceIndex].get();
		}
		void JGraphicImpl::ReBuildFrameResource(const J_UPLOAD_RESOURCE_TYPE type, const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowObjCount)
		{
			for (int i = 0; i < Constant::gNumFrameResources; ++i)
			{
				const uint newCapacity = CalculateCapacity(condition, frameResources[i]->GetElementCount(type), nowObjCount);
				frameResources[i]->BuildFrameResource(d3dDevice.Get(), type, newCapacity);
			}
		}
		void JGraphicImpl::ReCompileGraphicShader()
		{
			mRootSignature.Reset();
			BuildRootSignature();

			uint shaderCount = 0;
			auto handle = JResourceManager::Instance().GetResourceVectorHandle<JShader>(shaderCount);
			for (uint i = 0; i < shaderCount; ++i)
			{
				JShader* shader = static_cast<JShader*>(*(handle + i));
				if (!shader->IsComputeShader())
					shader->CompileInterface()->RecompileGraphicShader();
			}
		}
		J_UPLOAD_CAPACITY_CONDITION JGraphicImpl::IsPassRedefineCapacity(const uint capacity, const uint nowCount)const noexcept
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
		uint JGraphicImpl::CalculateCapacity(const J_UPLOAD_CAPACITY_CONDITION condition, const uint nowCapacity, const uint nowCount)const noexcept
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
		void JGraphicImpl::OnResize()
		{
			info.width = JWindow::Instance().GetClientWidth();
			info.height = JWindow::Instance().GetClientHeight();

			assert(d3dDevice);
			assert(swapChain);
			assert(directCmdListAlloc);

			// Flush before changing any resources.
			FlushCommandQueue();
			ThrowIfFailedHr(commandList->Reset(directCmdListAlloc.Get(), nullptr));

			// Release the previous resources we will be recreating.
			for (uint i = 0; i < graphicResource->swapChainBufferCount; ++i)
				graphicResource->swapChainBuffer[i].Reset();

			// Resize the swap chain. 
			ThrowIfFailedHr(swapChain->ResizeBuffers(
				graphicResource->swapChainBufferCount,
				(uint)info.width, (uint)info.height,
				graphicResource->backBufferFormat,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

			currBackBuffer = 0;
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(graphicResource->rtvHeap->GetCPUDescriptorHandleForHeapStart());
			for (uint i = 0; i < graphicResource->swapChainBufferCount; ++i)
			{
				ThrowIfFailedHr(swapChain->GetBuffer(i, IID_PPV_ARGS(&graphicResource->swapChainBuffer[i])));
				d3dDevice->CreateRenderTargetView(graphicResource->swapChainBuffer[i].Get(), nullptr, rtv);
				rtv.Offset(1, graphicResource->rtvDescriptorSize);
			}

			graphicResource->CreateMainDepthStencilResource(d3dDevice.Get(), commandList.Get(), info.width, info.height, m4xMsaaState, m4xMsaaQuality);

			outlineHelper->UpdatePassBuf(info.width, info.height, Constant::commonStencilRef);
			// Execute the resize commands.
			ThrowIfFailedHr(commandList->Close());
			ID3D12CommandList* cmdsLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
			// Wait until resize is complete.
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
		ID3D12Resource* JGraphicImpl::CurrentBackBuffer()const
		{
			return graphicResource->swapChainBuffer[currBackBuffer].Get();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE JGraphicImpl::CurrentBackBufferView()const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(
				graphicResource->rtvHeap->GetCPUDescriptorHandleForHeapStart(),
				currBackBuffer,
				graphicResource->rtvDescriptorSize);
		}
		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> JGraphicImpl::Sampler()const noexcept
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
		void JGraphicImpl::StoreData()
		{
			const std::wstring path = JApplicationVariable::GetProjectSettingPath() + L"\\GraphicOption.txt";
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
			JFileIOHelper::StoreAtomicData(stream, L"AllowHZBDepthMapDebug:", option.allowHZBDepthMapDebug);
			JFileIOHelper::StoreAtomicData(stream, L"AllowOutline:", option.allowOutline);
			stream.close();
		}
		void JGraphicImpl::LoadData()
		{
			const std::wstring path = JApplicationVariable::GetProjectSettingPath() + L"\\GraphicOption.txt";
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
			JFileIOHelper::LoadAtomicData(stream, newOption.allowHZBDepthMapDebug);
			JFileIOHelper::LoadAtomicData(stream, newOption.allowOutline);
			stream.close();
			SetGraphicOption(newOption);
		}
		JGraphicImpl::JGraphicImpl()
			:guid(JCUtil::CalculateGuid(typeid(JGraphicImpl).name()))
		{
			info.occlusionWidth = std::pow(2, JGraphicResourceManager::occlusionCapacity - 1);
			info.occlusionHeight = std::pow(2, JGraphicResourceManager::occlusionCapacity - 1);
			info.occlusionMinSize = graphicResource->minOcclusionSize;
			info.occlusionMapCapacity = JGraphicResourceManager::occlusionCapacity;
			info.occlusionMapCount = JMathHelper::Log2Int(info.occlusionWidth) - JMathHelper::Log2Int(graphicResource->minOcclusionSize) + 1;

			updateHelper.uData.resize((int)J_UPLOAD_RESOURCE_TYPE::COUNT);
			updateHelper.bData.resize((int)J_GRAPHIC_TEXTURE_TYPE::COUNT);

			auto objGetElementLam = []()
			{
				uint sum = 0;
				const uint drawListCount = JGraphicDrawList::GetListCount();
				for (uint i = 0; i < drawListCount; ++i)
					sum += (uint)JGraphicDrawList::GetDrawScene(i)->scene->GetMeshCount();
				return sum;
			};
			auto passGetElemenLam = []() {return JGraphicDrawList::GetListCount(); };
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
			auto materialGetElementLam = []() {return JResourceManager::Instance().GetResourceCount<JMaterial>(); };
			auto boundObjGetElementLam = []()
			{
				uint sum = 0;
				const uint drawListCount = JGraphicDrawList::GetListCount();
				for (uint i = 0; i < drawListCount; ++i)
					sum += JGraphicDrawList::GetDrawScene(i)->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				return sum;
			};

			using GetElementCount = JGraphicUpdateHelper::GetElementCountT::Ptr;
			std::unordered_map<J_UPLOAD_RESOURCE_TYPE, GetElementCount> frameGetFunc
			{
				{J_UPLOAD_RESOURCE_TYPE::OBJECT, objGetElementLam}, {J_UPLOAD_RESOURCE_TYPE::PASS, passGetElemenLam},
				{J_UPLOAD_RESOURCE_TYPE::ANIMATION, aniGetElementLam},{J_UPLOAD_RESOURCE_TYPE::CAMERA, camGetElementLam},
				{J_UPLOAD_RESOURCE_TYPE::LIGHT, litGetElementLam},	{J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX, litIndexGetElementLam},
				{J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT, shadowLitGetElementLam},{J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP, shadowMapElementLam},
				{J_UPLOAD_RESOURCE_TYPE::MATERIAL, materialGetElementLam},{J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT, boundObjGetElementLam}
			};

			using NotifyUpdateCapacity = JGraphicUpdateHelper::NotifyUpdateCapacityT::Callable;
			auto occlusionOnEvent = [](JGraphicImpl& graphic)
			{
				graphic.hzbOccHelper->UpdateObjectCapacity(graphic.d3dDevice.Get(), graphic.currFrameResource->GetElementCount(J_UPLOAD_RESOURCE_TYPE::OBJECT));
			};

			for (uint i = 0; i < (uint)J_UPLOAD_RESOURCE_TYPE::COUNT; ++i)
			{
				J_UPLOAD_RESOURCE_TYPE type = (J_UPLOAD_RESOURCE_TYPE)i;
				updateHelper.RegisterCallable(type, &frameGetFunc.find(type)->second);
			}

			updateHelper.RegisterListener(J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT, std::make_unique<NotifyUpdateCapacity>(occlusionOnEvent));

			auto texture2DGetCountLam = [](const JGraphicImpl& graphic) {return graphic.graphicResource->user2DTextureCount; };
			auto cubeMapGetCountLam = [](const JGraphicImpl& graphic) {return graphic.graphicResource->userCubeMapCount; };
			auto shadowMapGetCountLam = [](const JGraphicImpl& graphic) {return graphic.graphicResource->shadowMapCount; };

			auto texture2DGetCapacityLam = [](const JGraphicImpl& graphic) {return graphic.info.binding2DTextureCapacity; };
			auto cubeMapGetCapacityLam = [](const JGraphicImpl& graphic) {return graphic.info.bindingCubeMapCapacity; };
			auto shadowMapGetCapacityLam = [](const JGraphicImpl& graphic) {return graphic.info.bindingShadowTextureCapacity; };

			auto texture2DSetCapaLam = [](JGraphicImpl& graphic)
			{
				graphic.info.binding2DTextureCapacity = graphic.CalculateCapacity(graphic.updateHelper.bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D].recompileCondition,
					graphic.info.binding2DTextureCapacity,
					graphic.updateHelper.bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D].count);
			};
			auto cubeMapeSetCapaLam = [](JGraphicImpl& graphic)
			{
				graphic.info.bindingCubeMapCapacity = graphic.CalculateCapacity(graphic.updateHelper.bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE].recompileCondition,
					graphic.info.bindingCubeMapCapacity,
					graphic.updateHelper.bData[(int)J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE].count);
			};
			auto shadowMapSetCapaLam = [](JGraphicImpl& graphic)
			{
				graphic.info.bindingShadowTextureCapacity = graphic.CalculateCapacity(graphic.updateHelper.bData[(int)J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP].recompileCondition,
					graphic.info.bindingShadowTextureCapacity,
					graphic.updateHelper.bData[(int)J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP].count);
			};

			using BindTextureGetCount = JGraphicUpdateHelper::GetTextureCountT::Ptr;
			using BindTextureGetCapacity = JGraphicUpdateHelper::GetTextureCapacityT::Ptr;
			using BindTextureSetCapacity = JGraphicUpdateHelper::SetCapacityT::Ptr;

			std::unordered_map < J_GRAPHIC_TEXTURE_TYPE, bool> hasCallable
			{
				{J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D, true},{J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE, true},
				{J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_COMMON, false},{J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP, true}
			};
			std::unordered_map<J_GRAPHIC_TEXTURE_TYPE, BindTextureGetCount> bindTextureGetCountFunc
			{
				{J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D, texture2DGetCountLam}, {J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE, cubeMapGetCountLam},
				{J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP, shadowMapGetCountLam}
			};
			std::unordered_map<J_GRAPHIC_TEXTURE_TYPE, BindTextureGetCapacity> bindTextureGetCapacityFunc
			{
				{J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D, texture2DGetCapacityLam}, {J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE, cubeMapGetCapacityLam},
				{J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP, shadowMapGetCapacityLam}
			};
			std::unordered_map<J_GRAPHIC_TEXTURE_TYPE, BindTextureSetCapacity> bindTextureSetCapaFunc
			{
				{J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D, texture2DSetCapaLam}, {J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE, cubeMapeSetCapaLam},
				{J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP, shadowMapSetCapaLam}
			};

			for (uint i = 0; i < (uint)J_GRAPHIC_TEXTURE_TYPE::COUNT; ++i)
			{
				J_GRAPHIC_TEXTURE_TYPE type = (J_GRAPHIC_TEXTURE_TYPE)i;
				if (hasCallable.find(type)->second)
				{
					updateHelper.RegisterCallable(type,
						&bindTextureGetCountFunc.find(type)->second,
						&bindTextureGetCapacityFunc.find(type)->second,
						&bindTextureSetCapaFunc.find(type)->second);
				}
			}

		}
		JGraphicImpl::~JGraphicImpl()
		{}
	}
}