#include"JGraphic.h" 
#include"JGraphicResourceManager.h"
#include"JGraphicDrawList.h"

#include"../Window/JWindows.h"
#include"FrameResource/JFrameResource.h"
#include"FrameResource/JFrameResourceCash.h" 
#include"FrameResource/JObjectConstants.h" 
#include"FrameResource/JAnimationConstants.h" 
#include"FrameResource/JMaterialConstants.h" 
#include"FrameResource/JPassConstants.h" 
#include"FrameResource/JCameraConstants.h" 
#include"FrameResource/JLightConstants.h" 
#include"FrameResource/JShadowMapConstants.h" 

#include"../Core/GameTimer/JGameTimer.h"
#include"../Object/GameObject/JGameObject.h"
#include"../Object/Component/RenderItem/JRenderItem.h"
#include"../Object/Component/Transform/JTransform.h"
#include"../Object/Component/Camera/JCamera.h"
#include"../Object/Component/Animator/JAnimator.h"
#include"../Object/Component/Light/JLight.h"

#include"../Object/Resource/JResourceManager.h"
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Resource/Mesh/JMeshGeometry.h"
#include"../Object/Resource/Material/JMaterial.h"
#include"../Object/Resource/Shader/JShader.h"
#include"../Object/Resource/Shader/JShaderData.h"
#include"../Object/Resource/AnimationClip/JAnimationClip.h"
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
		JGraphicFrameInterface* JGraphicImpl::FrameInterface()noexcept
		{
			return this;
		}
		JGraphicImpl::JGraphicImpl()
			:guid(JCommonUtility::CalculateGuid(typeid(JGraphicImpl).name()))
		{
			guid = JCommonUtility::CalculateGuid(typeid(JGraphicImpl).name());
			width = JWindow::Instance().GetClientWidth();
			height = JWindow::Instance().GetClientHeight();

			graphicResource = std::make_unique<JGraphicResourceManager>();
			 
			InitializeD3D();
			InitializeResource();
			OnResize();
			AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_RESIZE);
		}
		JGraphicImpl::~JGraphicImpl()
		{
			if (d3dDevice != nullptr)
				FlushCommandQueue();
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
		JGraphicTextureHandle* JGraphicImpl::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::string& path)
		{
			if (!stCommand)
			{
				FlushCommandQueue();
				StartCommand();
				JGraphicTextureHandle* handle = graphicResource->Create2DTexture(uploadHeap, path, d3dDevice.Get(), commandList.Get());
				EndCommand();
				FlushCommandQueue();
				return handle;
			}
			else
				return graphicResource->Create2DTexture(uploadHeap, path, d3dDevice.Get(), commandList.Get());
		}
		JGraphicTextureHandle* JGraphicImpl::CreateCubeTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::string& path)
		{
			if (!stCommand)
			{
				FlushCommandQueue();
				StartCommand();
				JGraphicTextureHandle* handle = graphicResource->CreateCubeTexture(uploadHeap, path, d3dDevice.Get(), commandList.Get());
				EndCommand();
				FlushCommandQueue();
				return handle;
			}
			else
				return graphicResource->CreateCubeTexture(uploadHeap, path, d3dDevice.Get(), commandList.Get());
		}
		JGraphicTextureHandle* JGraphicImpl::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			if (textureWidth == 0 || textureHeight == 0)
			{
				textureWidth = JGraphicImpl::width;
				textureHeight = JGraphicImpl::height;
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
				textureWidth = JGraphicImpl::defaultShadowWidth;
				textureHeight = JGraphicImpl::defaultShadowHeight;
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
		void JGraphicImpl::StuffShaderPso(JShaderData* shaderData, J_SHADER_VERTEX_LAYOUT vertexLayout, J_SHADER_FUNCTION functionFlag)
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
			newShaderPso.PS =
			{
				reinterpret_cast<BYTE*>(shaderData->Ps->GetBufferPointer()),
				shaderData->Ps->GetBufferSize()
			};
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
			if ((functionFlag & SHADER_FUNCTION_SKY) > 0)
			{
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
				newShaderPso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			}
			if ((functionFlag & SHADER_FUNCTION_NONCULLING) > 0)
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

			newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			newShaderPso.SampleMask = UINT_MAX;
			newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			newShaderPso.NumRenderTargets = 1;
			newShaderPso.RTVFormats[0] = graphicResource->backBufferFormat;
			newShaderPso.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			newShaderPso.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			newShaderPso.DSVFormat = graphicResource->depthStencilFormat;

			if ((functionFlag & SHADER_FUNCTION_SHADOW_MAP) > 0)
			{
				newShaderPso.RasterizerState.DepthBias = 100000;
				newShaderPso.RasterizerState.DepthBiasClamp = 0.0f;
				newShaderPso.RasterizerState.SlopeScaledDepthBias = 1.0f;
				newShaderPso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
				newShaderPso.NumRenderTargets = 0;
			}
			if ((functionFlag & SHADER_FUNCTION_DEBUG) > 0)
			{
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
				newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			}
			ThrowIfFailedG(d3dDevice->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(shaderData->Pso.GetAddressOf())));

			EndCommand();
			FlushCommandQueue();
		}
		void JGraphicImpl::SetImGuiBackEnd()
		{
			ImGui_ImplDX12_Init(d3dDevice.Get(), gNumFrameResources,
				DXGI_FORMAT_R8G8B8A8_UNORM, graphicResource->srvHeap.Get(),
				graphicResource->srvHeap->GetCPUDescriptorHandleForHeapStart(),
				graphicResource->srvHeap->GetGPUDescriptorHandleForHeapStart());
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
				/*
					_In_opt_ LPSECURITY_ATTRIBUTES lpEventAttributes,
					_In_opt_ LPCWSTR lpName,
					_In_ DWORD dwFlags,
					_In_ DWORD dwDesiredAccess
				*/
				HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

				// Fire event when GPU hits current fence.  
				ThrowIfFailedHr(fence->SetEventOnCompletion(currentFence, eventHandle));

				// Wait until the GPU hits current fence event is fired.
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
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

			CD3DX12_RESOURCE_BARRIER rvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			commandList->ResourceBarrier(1, &rvBarrier);
			commandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightCoral, 0, nullptr);
			commandList->ClearDepthStencilView(graphicResource->GetCpuDsvDescriptorHandle(0), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			D3D12_CPU_DESCRIPTOR_HANDLE depthStencilViewHandle = graphicResource->GetCpuDsvDescriptorHandle(0);
			D3D12_CPU_DESCRIPTOR_HANDLE rendertargetViewHandle = CurrentBackBufferView();

			commandList->OMSetRenderTargets(1, &rendertargetViewHandle, true, &depthStencilViewHandle);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

			rvBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			commandList->ResourceBarrier(1, &rvBarrier);

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
			currBackBuffer = (currBackBuffer + 1) % graphicResource->swapChainBuffercount;
			// Advance the fence value to mark commands up to this fence point.
			currFrameResource->fence = ++currentFence;

			// Add an instruction to the command queue to set a new fence point. 
			// Because we are on the GPU timeline, the new fence point won't be 
			// set untwil the GPU finishes processing all the commands prior to this Signal().
			commandQueue->Signal(fence.Get(), currentFence);
		}
		void JGraphicImpl::UpdateWait()
		{
			currFrameResourceIndex = (currFrameResourceIndex + 1) % gNumFrameResources;
			currFrameResource = frameResources[currFrameResourceIndex].get();

			if (currFrameResource->fence != 0 && fence->GetCompletedValue() < currFrameResource->fence)
			{
				HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
				ThrowIfFailedHr(fence->SetEventOnCompletion(currFrameResource->fence, eventHandle));
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
		}
		void JGraphicImpl::UpdateEngine()
		{
			//JResourceManager::Instance().GetMainScene()->SpaceSpatialInterface()->ViewCulling();

			uint sceneObjCBoffset = 0;
			uint scenePassCBoffset = 0;
			uint sceneAniCBoffset = 0;
			uint sceneCamCBoffset = 0;
			uint sceneLightCBoffset = 0;
			uint sceneShadowCBoffset = 0;
			UpdateMaterialCB();

			const uint drawListCount = JGraphicDrawList::GetListCount();
			for (uint i = 0; i < drawListCount; ++i)
			{
				JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
				drawTarget->scene->SpaceSpatialInterface()->ViewCulling();

				uint objUpdateCount = UpdateSceneObjectCB(drawTarget->scene, sceneObjCBoffset);
				uint passUpdateCount = UpdateScenePassCB(drawTarget->scene, scenePassCBoffset);

				uint aniUpdateCount = UpdateSceneAnimationCB(drawTarget->scene, sceneAniCBoffset);
				uint camUpdateCount = UpdateSceneCameraCB(drawTarget->scene, sceneCamCBoffset);
				uint lightUpdateCount = UpdateSceneLightCB(drawTarget->scene, sceneLightCBoffset, sceneShadowCBoffset);

				int totalUpdateCount = objUpdateCount + aniUpdateCount + camUpdateCount + lightUpdateCount;
				if (totalUpdateCount > 0)
					drawTarget->hasUpdate = true;
			}
		}
		uint JGraphicImpl::UpdateSceneObjectCB(_In_ JScene* scene, uint& objCBoffset)
		{
			uint updateCount = 0;
			std::vector<JComponent*>& jRvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
			const uint renderItemCount = (uint)jRvec.size();

			JObjectConstants objectConstants;
			auto currObjectCB = currFrameResource->objectCB.get();
			for (uint i = 0; i < renderItemCount; ++i)
			{
				JRenderItem* renderItem = static_cast<JRenderItem*>(jRvec[i]);
				if (renderItem->CallUpdateFrame(objectConstants))
				{
					currObjectCB->CopyData(objCBoffset + i, objectConstants);
					++updateCount;
				}
			}

			objCBoffset += renderItemCount;
			return updateCount;
		}
		uint JGraphicImpl::UpdateMaterialCB()
		{
			uint updateCount = 0;
			auto currMaterialBuffer = currFrameResource->materialBuffer.get();
			uint matCount;
			std::vector<JResourceObject*>::const_iterator matBegin = JResourceManager::Instance().GetResourceVectorHandle<JMaterial>(matCount);
			for (uint i = 0; i < matCount; ++i)
			{
				JMaterialConstants materialConstant;
				JMaterial* material = static_cast<JMaterial*>(*(matBegin + i));
				if (material->CallUpdateFrame(materialConstant))
				{				  
					currMaterialBuffer->CopyData(i, materialConstant);
					++updateCount;
				}
			}
			return updateCount;
		}
		uint JGraphicImpl::UpdateScenePassCB(_In_ JScene* scene, uint& passCBoffset)
		{
			JPassConstants passContants;
			passContants.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
			passContants.totalTime = JGameTimer::Instance().TotalTime();
			passContants.deltaTime = JGameTimer::Instance().DeltaTime();

			auto currPassCB = currFrameResource->passCB.get();
			currPassCB->CopyData(passCBoffset, passContants);
			return 1;
		}
		uint JGraphicImpl::UpdateSceneAnimationCB(_In_ JScene* scene, uint& aniCBoffset)
		{
			uint updateCount = 0;
			std::vector<JComponent*>& jAvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
			const uint animatorCount = (uint)jAvec.size();

			auto currSkinnedCB = currFrameResource->skinnedCB.get();
			JAnimationConstants animationConstatns;
			if (scene->IsAnimatorActivated())
			{
				for (uint i = 0; i < animatorCount; ++i)
				{
					JAnimator* animator = static_cast<JAnimator*>(jAvec[i]);
					if (animator->CallUpdateFrame(animationConstatns))
					{
						currSkinnedCB->CopyData(aniCBoffset + i, animationConstatns);
						++updateCount;
					}
				}
			}
			aniCBoffset += animatorCount;
			return updateCount;
		}
		uint JGraphicImpl::UpdateSceneCameraCB(_In_ JScene* scene, uint& camCBoffset)
		{
			uint updateCount = 0;
			std::vector<JComponent*>& jCvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
			const uint cameraCount = (uint)jCvec.size();

			auto currCameraCB = currFrameResource->cameraCB.get();
			JCameraConstants camContants;
			for (uint i = 0; i < cameraCount; ++i)
			{
				JCamera* camera = static_cast<JCamera*>(jCvec[i]);
				if (camera->CallUpdateFrame(camContants))
				{
					currCameraCB->CopyData(camCBoffset + i, camContants);
					++updateCount;
				}
			}
			camCBoffset += cameraCount;
			return updateCount;
		}
		uint JGraphicImpl::UpdateSceneLightCB(_In_ JScene* scene, uint& lightCBoffset, uint& shadowCalCBoffset)
		{
			uint updateCount = 0;
			std::vector<JComponent*>& jLvec = scene->CashInterface()->GetComponentCashVec(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
			const uint lightCount = (uint)jLvec.size();

			auto currLightCB = currFrameResource->lightCB.get();
			auto currShadowCalCB = currFrameResource->shadowCalCB.get();

			uint shadowCount = 0;
			for (uint i = 0; i < lightCount; ++i)
			{
				JLightConstants lightConstants;
				JShadowMapConstants shadowCalConstant;

				JLight* light = static_cast<JLight*>(jLvec[i]);
				if (light->CallUpdateFrame(lightConstants, shadowCalConstant))
				{
					currLightCB->CopyData(lightCBoffset + i, lightConstants);
					if (light->IsShadowActivated())
						currShadowCalCB->CopyData(shadowCalCBoffset + shadowCount, shadowCalConstant);

				}

				if (light->IsShadowActivated())
					++shadowCount;
			}
			lightCBoffset += lightCount;
			shadowCalCBoffset += shadowCount;
			return updateCount;
		}
		void JGraphicImpl::DrawScene()
		{
			auto cmdListAlloc = currFrameResource->cmdListAlloc;
			ThrowIfFailedHr(cmdListAlloc->Reset());
			ThrowIfFailedHr(commandList->Reset(cmdListAlloc.Get(), nullptr));

			ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResource->srvHeap.Get() };
			commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			commandList->RSSetViewports(1, &screenViewport);
			commandList->RSSetScissorRects(1, &scissorRect);

			commandList->SetGraphicsRootSignature(mRootSignature.Get());

			auto matBuffer = currFrameResource->materialBuffer->Resource();
			commandList->SetGraphicsRootShaderResourceView(6, matBuffer->GetGPUVirtualAddress());

			JTexture* skyTxt = JResourceManager::Instance().GetEditorTexture(J_EDITOR_TEXTURE::DEFAULT_SKY);
			commandList->SetGraphicsRootDescriptorTable(7, GetGpuHandle(*skyTxt));
			commandList->SetGraphicsRootDescriptorTable(8, graphicResource->srvHeap->GetGPUDescriptorHandleForHeapStart());

			uint sceneObjCBoffset = 0;
			uint scenePassCBoffset = 0;
			uint sceneAniCBoffset = 0;
			uint sceneCamCBoffset = 0;
			uint sceneLightCBoffset = 0;
			uint sceneShadowCBoffset = 0;

			//수정필요
			//Shadow Map Draw
			//Update가 있는 광원만 추적해서 Draw
			//광원내에 있는 오브젝트 검색을 위한 공간분할 및 검색 필요
			const uint drawListCount = JGraphicDrawList::GetListCount();
			for (uint i = 0; i < drawListCount; ++i)
			{
				JGraphicDrawTarget* drawTarget = JGraphicDrawList::GetDrawScene(i);
				if (drawTarget->hasUpdate)
				{
					const uint shadowReqCount = (uint)drawTarget->shadowRequestor.size();
					for (uint j = 0; j < shadowReqCount; ++j)
					{
						JLight* jLight = static_cast<JLight*>(drawTarget->shadowRequestor[i]->jLight);
						DrawSceneShadowMap(drawTarget->scene,
							jLight,
							sceneObjCBoffset,
							scenePassCBoffset,
							sceneAniCBoffset,
							sceneShadowCBoffset + j);
					}

					CD3DX12_GPU_DESCRIPTOR_HANDLE shadowHandle = graphicResource->GetGpuSrvDescriptorHandle(graphicResource->GetSrvShadowMapStart());
					commandList->SetGraphicsRootDescriptorTable(9, shadowHandle);

					const uint sceneReqCount = (uint)drawTarget->sceneRequestor.size();
					for (uint j = 0; j < sceneReqCount; ++j)
					{
						JCamera* jCamera = static_cast<JCamera*>(drawTarget->sceneRequestor[i]->jCamera);
						DrawSceneRenderTarget(drawTarget->scene,
							jCamera,
							sceneObjCBoffset,
							scenePassCBoffset,
							sceneAniCBoffset,
							sceneCamCBoffset + j,
							sceneLightCBoffset);
					}
					drawTarget->hasUpdate = false;
				}

				sceneObjCBoffset = drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_RENDERITEM);
				scenePassCBoffset += 1;
				sceneAniCBoffset = drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR);
				sceneCamCBoffset = drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA);
				sceneLightCBoffset = drawTarget->scene->GetComponetCount(J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT);
				sceneShadowCBoffset += (uint)drawTarget->shadowRequestor.size();
			}
		}
		void JGraphicImpl::DrawProjectSelector()
		{
			currFrameResourceIndex = (currFrameResourceIndex + 1) % gNumFrameResources;
			currFrameResource = frameResources[currFrameResourceIndex].get();
			// Has the GPU finished processing the commands of the current frame resource?
			// If not, wait until the GPU has completed commands up to this fence point.
			if (currFrameResource->fence != 0 && fence->GetCompletedValue() < currFrameResource->fence)
			{
				HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
				ThrowIfFailedHr(fence->SetEventOnCompletion(currFrameResource->fence, eventHandle));
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
			auto cmdListAlloc = currFrameResource->cmdListAlloc;
			ThrowIfFailedHr(cmdListAlloc->Reset());
			ThrowIfFailedHr(commandList->Reset(cmdListAlloc.Get(), nullptr));

			ID3D12DescriptorHeap* descriptorHeaps[] = { graphicResource->srvHeap.Get() };
			commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			commandList->RSSetViewports(1, &screenViewport);
			commandList->RSSetScissorRects(1, &scissorRect);
		}
		void JGraphicImpl::DrawSceneRenderTarget(_In_ JScene* scene,
			_In_ JCamera* camera,
			const uint objCBoffset,
			const uint passCBoffset,
			const uint aniCBoffset,
			const uint camCBoffset,
			const uint lightCBoffset)
		{
			const uint rtvHeapIndex = GetTxtRtvHeapIndex(*camera); 

			commandList->RSSetViewports(1, &screenViewport);
			commandList->RSSetScissorRects(1, &scissorRect);
		
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = graphicResource->GetCpuRtvDescriptorHandle(rtvHeapIndex);
			CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->renderResultResource[rtvHeapIndex].Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

			commandList->ResourceBarrier(1, &rsBarrier);
			commandList->ClearRenderTargetView(rtv, Colors::Aqua, 0, nullptr);
			commandList->ClearDepthStencilView(graphicResource->GetCpuDsvDescriptorHandle(0), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			D3D12_CPU_DESCRIPTOR_HANDLE depthStencilViewHandle = graphicResource->GetCpuDsvDescriptorHandle(0);
			commandList->OMSetRenderTargets(1, &rtv, true, &depthStencilViewHandle);

			uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JPassConstants));
			auto passCB = currFrameResource->passCB->Resource();
			D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + passCBoffset * passCBByteSize;
			commandList->SetGraphicsRootConstantBufferView(2, passCBAddress);

			uint camCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JCameraConstants));
			auto camCB = currFrameResource->cameraCB->Resource();
			D3D12_GPU_VIRTUAL_ADDRESS camCBAddress = camCB->GetGPUVirtualAddress() + camCBoffset * camCBByteSize;
			commandList->SetGraphicsRootConstantBufferView(3, camCBAddress);

			uint lightCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JLightConstants));
			auto lightCB = currFrameResource->lightCB->Resource();
			D3D12_GPU_VIRTUAL_ADDRESS lightCBAddress = lightCB->GetGPUVirtualAddress() + lightCBoffset * lightCBByteSize;
			commandList->SetGraphicsRootConstantBufferView(4, lightCBAddress);

			DrawGameObject(commandList.Get(), scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC), objCBoffset, aniCBoffset, false, false);
			DrawGameObject(commandList.Get(), scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED), objCBoffset, aniCBoffset, false, scene->IsAnimatorActivated());
			DrawGameObject(commandList.Get(), scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_LAYER, J_MESHGEOMETRY_TYPE::STATIC), objCBoffset, aniCBoffset, false, false);
			DrawGameObject(commandList.Get(), scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::SKY, J_MESHGEOMETRY_TYPE::STATIC), objCBoffset, aniCBoffset, false, false);

			rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->renderResultResource[GetTxtVectorIndex(*camera)].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
			commandList->ResourceBarrier(1, &rsBarrier);
		}
		void JGraphicImpl::DrawSceneShadowMap(_In_ JScene* scene,
			_In_ JLight* light,
			const uint objCBoffset,
			const uint passCBoffset,
			const uint aniCBoffset,
			const uint shadowCalCBoffset)
		{
			const uint shadowWidth = GetTxtWidth(*light);
			const uint shadowHeight = GetTxtHeight(*light);
			const uint rVecIndex = GetTxtVectorIndex(*light);
			const uint dsvHeapIndex = GetTxtDsvHeapIndex(*light);

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)shadowWidth, (float)shadowHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, shadowWidth, shadowHeight };

			commandList->RSSetViewports(1, &mViewport);
			commandList->RSSetScissorRects(1, &mScissorRect);

			CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->shadowMapResource[rVecIndex].Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			commandList->ResourceBarrier(1, &rsBarrier);

			commandList->ClearDepthStencilView(graphicResource->GetCpuDsvDescriptorHandle(dsvHeapIndex),
				D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

			D3D12_CPU_DESCRIPTOR_HANDLE depthStencilViewHandle = graphicResource->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			commandList->OMSetRenderTargets(0, nullptr, false, &depthStencilViewHandle);

			uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JPassConstants));
			auto passCB = currFrameResource->passCB->Resource();
			D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + passCBoffset * passCBByteSize;
			commandList->SetGraphicsRootConstantBufferView(2, passCBAddress);

			uint shadowCalCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JShadowMapConstants));
			auto shadowCalCB = currFrameResource->shadowCalCB->Resource();
			D3D12_GPU_VIRTUAL_ADDRESS shadowCalCBAddress = shadowCalCB->GetGPUVirtualAddress() + shadowCalCBoffset * shadowCalCBByteSize;
			commandList->SetGraphicsRootConstantBufferView(5, shadowCalCBAddress);

			DrawGameObject(commandList.Get(), scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC), objCBoffset, aniCBoffset, false, false);
			DrawGameObject(commandList.Get(), scene->CashInterface()->GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED), objCBoffset, aniCBoffset, false, scene->IsAnimatorActivated());

			rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(graphicResource->shadowMapResource[GetTxtVectorIndex(*light)].Get(),
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
			commandList->ResourceBarrier(1, &rsBarrier);
		}
		void JGraphicImpl::DrawGameObject(ID3D12GraphicsCommandList* cmdList,
			const std::vector<JGameObject*>& gameObject,
			const uint objCBoffset,
			const uint aniCBoffset,
			const bool drawShadowMap,
			const bool onAnimation)
		{
			uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
			uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

			auto objectCB = currFrameResource->objectCB->Resource();
			auto skinCB = currFrameResource->skinnedCB->Resource();
			const uint gameObjCount = (uint)gameObject.size();

			for (uint i = 0; i < gameObjCount; ++i)
			{
				const JRenderItem* renderItem = gameObject[i]->GetRenderItem();
				//수정필요
				//포트폴리오용 코드
				const JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>();

				if (!renderItem->IsVisible())
					continue;

				const JMeshGeometry* mesh = renderItem->GetMesh();
				const JMaterial* mat = renderItem->GetMaterial();
				const JShader* shader = mat->GetShader();
				const size_t shaderGuid = shader->GetGuid();
				const bool onSkinned = animator != nullptr && onAnimation;

				if (!drawShadowMap)
				{
					if (onSkinned)
						cmdList->SetPipelineState(shader->GetPso(ConvertMeshTypeToShaderVertexLayout(J_MESHGEOMETRY_TYPE::SKINNED)));
					else
						cmdList->SetPipelineState(shader->GetPso(ConvertMeshTypeToShaderVertexLayout(J_MESHGEOMETRY_TYPE::STATIC)));
				}
				else
				{
					JShader* shadow = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_SHADER::DEFAULT_SHADOW_MAP_SHADER);
					if (onSkinned)
						commandList->SetPipelineState(shadow->GetPso(ConvertMeshTypeToShaderVertexLayout(J_MESHGEOMETRY_TYPE::SKINNED)));
					else
						commandList->SetPipelineState(shadow->GetPso(ConvertMeshTypeToShaderVertexLayout(J_MESHGEOMETRY_TYPE::STATIC)));
				}

				const D3D12_VERTEX_BUFFER_VIEW vertexPtr = mesh->VertexBufferView();
				const D3D12_INDEX_BUFFER_VIEW indexPtr = mesh->IndexBufferView();

				cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
				cmdList->IASetIndexBuffer(&indexPtr);
				cmdList->IASetPrimitiveTopology(renderItem->GetPrimitiveType());

				D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() +
					(objCBoffset + i) * objectCBByteSize;

				cmdList->SetGraphicsRootConstantBufferView(0, objectCBAddress);

				if (onSkinned)
				{
					D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + (aniCBoffset + i) * skinCBByteSize;
					cmdList->SetGraphicsRootConstantBufferView(1, skinObjCBAddress);
				}
				cmdList->DrawIndexedInstanced(renderItem->GetIndexCount(), 1, renderItem->GetStartIndexLocation(), renderItem->GetBaseVertexLocation(), 0);
			}
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

			ThrowIfFailedHr(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&fence)));

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
			// Reset the command list to prep for initialization commands.
			ThrowIfFailedHr(commandList->Reset(directCmdListAlloc.Get(), nullptr));
			// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
			// so we have to query this information. 		
			BuildRootSignature();
			graphicResource->BuildRtvDescriptorHeaps(d3dDevice.Get());
			graphicResource->BuildDsvDescriptorHeaps(d3dDevice.Get());
			graphicResource->BuildSrvDescriptorHeaps(d3dDevice.Get());
			graphicResource->BuildDepthStencilResource(d3dDevice.Get(), commandList.Get(), width, height, m4xMsaaState, m4xMsaaQuality);
			BuildFrameResources();
			// Execute the initialization commands.
			ThrowIfFailedHr(commandList->Close());
			ID3D12CommandList* cmdsLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
			// Wait until initialization is complete.
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
			sd.BufferDesc.Width = width;
			sd.BufferDesc.Height = height;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferDesc.Format = graphicResource->backBufferFormat;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
			sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = graphicResource->swapChainBuffercount;
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
			CD3DX12_DESCRIPTOR_RANGE texTable00;
			texTable00.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

			CD3DX12_DESCRIPTOR_RANGE texTable01;
			texTable01.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 300, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE texTable02;
			texTable02.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 300, 301, 0);

			static constexpr int slotCount = 10;
			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

			// Create root CBV.
			slotRootParameter[0].InitAsConstantBufferView(0);
			slotRootParameter[1].InitAsConstantBufferView(1);
			slotRootParameter[2].InitAsConstantBufferView(2);
			slotRootParameter[3].InitAsConstantBufferView(3);
			slotRootParameter[4].InitAsConstantBufferView(4);
			slotRootParameter[5].InitAsConstantBufferView(5);
			slotRootParameter[6].InitAsShaderResourceView(0, 1);
			slotRootParameter[7].InitAsDescriptorTable(1, &texTable00, D3D12_SHADER_VISIBILITY_ALL);
			slotRootParameter[8].InitAsDescriptorTable(1, &texTable01, D3D12_SHADER_VISIBILITY_ALL);
			slotRootParameter[9].InitAsDescriptorTable(1, &texTable02, D3D12_SHADER_VISIBILITY_ALL);

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
			for (int i = 0; i < gNumFrameResources; ++i)
			{
				frameResources.push_back(std::make_unique<JFrameResource>(d3dDevice.Get(),
					JResourceData::initMaterialCapacity,
					JResourceData::initGameObjectCapacity,
					JResourceData::initAnimationControllerCapacity,
					1 + JResourceData::initPreviewSceneCapacity,
					JResourceData::initSceneCameraCapacity + JResourceData::initPreviewSceneCapacity,
					maxLight));
			}
			currFrameResource = frameResources[currFrameResourceIndex].get();
		}
		void JGraphicImpl::OnResize()
		{
			width = JWindow::Instance().GetClientWidth();
			height = JWindow::Instance().GetClientHeight();

			assert(d3dDevice);
			assert(swapChain);
			assert(directCmdListAlloc);

			// Flush before changing any resources.
			FlushCommandQueue();
			ThrowIfFailedHr(commandList->Reset(directCmdListAlloc.Get(), nullptr));

			// Release the previous resources we will be recreating.
			for (uint i = 0; i < graphicResource->swapChainBuffercount; ++i)
				graphicResource->swapChainBuffer[i].Reset();

			// Resize the swap chain. 
			ThrowIfFailedHr(swapChain->ResizeBuffers(
				graphicResource->swapChainBuffercount,
				(uint)width, (uint)height,
				graphicResource->backBufferFormat,
				DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

			currBackBuffer = 0;
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(graphicResource->rtvHeap->GetCPUDescriptorHandleForHeapStart());
			for (uint i = 0; i < graphicResource->swapChainBuffercount; ++i)
			{
				ThrowIfFailedHr(swapChain->GetBuffer(i, IID_PPV_ARGS(&graphicResource->swapChainBuffer[i])));
				d3dDevice->CreateRenderTargetView(graphicResource->swapChainBuffer[i].Get(), nullptr, rtv);
				rtv.Offset(1, graphicResource->rtvDescriptorSize);
			}

			graphicResource->BuildDepthStencilResource(d3dDevice.Get(), commandList.Get(), width, height, m4xMsaaState, m4xMsaaQuality);

			// Execute the resize commands.
			ThrowIfFailedHr(commandList->Close());
			ID3D12CommandList* cmdsLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
			// Wait until resize is complete.
			FlushCommandQueue();
			// Update the viewport transform to cover the client area.
			screenViewport.TopLeftX = 0;
			screenViewport.TopLeftY = 0;
			screenViewport.Width = static_cast<float>(width);
			screenViewport.Height = static_cast<float>(height);
			screenViewport.MinDepth = 0.0f;
			screenViewport.MaxDepth = 1.0f;

			scissorRect = { 0, 0, width, height };
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
	}
}