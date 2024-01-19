#include"JDx12Adaptee.h"
#include"../JGraphicUpdateHelper.h"
#include"../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../Culling/Dx/JDx12CullingManager.h"
#include"../Debug/Dx/JDx12GraphicDebug.h"
#include"../DepthMap/Dx/JDx12DepthTest.h"
#include"../Device/Dx/JDx12GraphicDevice.h"
#include"../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../Outline/Dx/JDx12Outline.h"
#include"../Culling/Occlusion/Dx/JDx12HardwareOccCulling.h"
#include"../Culling/Occlusion/Dx/JDx12HZBOccCulling.h"
#include"../Culling/Light/Dx/JDx12LightCulling.h"
#include"../Scene/Dx/JDx12SceneDraw.h"
#include"../ShadowMap/Dx/JDx12ShadowMap.h" 
#include"../Image/Dx/JDx12ImageProcessing.h"
#include"../FrameResource/Dx/JDx12FrameResource.h"
#include"../Utility/Dx/JD3DUtility.h" 

namespace JinEngine::Graphic
{
	namespace
	{
		static void SettingDescriptorHeaps(JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList)
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { dx12Gm->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
			cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		}
		static ID3D12Resource* CurrentBackBuffer(JDx12GraphicDevice* dx12Device, JDx12GraphicResourceManager* dx12Gm)
		{
			return dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, dx12Device->GetBackBufferIndex());
		}
		static D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView(JDx12GraphicDevice* dx12Device, JDx12GraphicResourceManager* dx12Gm)
		{
			return dx12Gm->GetCpuRtvDescriptorHandle(dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, dx12Device->GetBackBufferIndex())->
				GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::RTV));
		}
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Adaptee::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	std::unique_ptr<JGraphicDevice> JDx12Adaptee::CreateDevice()
	{
		return std::make_unique<JDx12GraphicDevice>();
	}
	std::unique_ptr<JGraphicResourceManager> JDx12Adaptee::CreateGraphicResourceManager()
	{
		return std::make_unique<JDx12GraphicResourceManager>();
	}
	std::unique_ptr<JCullingManager> JDx12Adaptee::CreateCullingManager()
	{
		return std::make_unique<JDx12CullingManager>();
	}
	void JDx12Adaptee::CreateFrameResource(_Out_ std::unique_ptr<JFrameResource>(&frame)[Constants::gNumFrameResources])
	{
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			frame[i] = std::make_unique<JDx12FrameResource>();
	}
	std::unique_ptr<JGraphicDebug> JDx12Adaptee::CreateDebug()
	{
		return std::make_unique<JDx12GraphicDebug>();
	}
	std::unique_ptr<JDepthTest> JDx12Adaptee::CreateDepthTest()
	{
		return std::make_unique<JDx12DepthTest>();
	}
	std::unique_ptr<JShadowMap> JDx12Adaptee::CreateShadowMapDraw()
	{
		return std::make_unique<JDx12ShadowMap>();
	}
	std::unique_ptr<JSceneDraw> JDx12Adaptee::CreateSceneDraw()
	{
		return std::make_unique<JDx12SceneDraw>();
	}
	std::unique_ptr<JHardwareOccCulling> JDx12Adaptee::CreateHdOcc()
	{
		return std::make_unique<JDx12HardwareOccCulling>();
	}
	std::unique_ptr<JHZBOccCulling> JDx12Adaptee::CreateHzbOcc()
	{
		return std::make_unique<JDx12HZBOccCulling>();
	}
	std::unique_ptr<JLightCulling> JDx12Adaptee::CreateLightCulling()
	{
		return std::make_unique<JDx12LightCulling>();
	}
	std::unique_ptr<JOutline> JDx12Adaptee::CreateOutlineDraw()
	{
		return std::make_unique<JDx12Outline>();
	} 
	std::unique_ptr<JImageProcessing> JDx12Adaptee::CreateImageProcessing()
	{
		return std::make_unique<JDx12ImageProcessing>();
	}
	void JDx12Adaptee::BeginUpdateStart(const JGraphicDrawReferenceSet& drawRefSet) 
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.cullingM))
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device); 
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);

		dx12Cm->CopyCullingResultGpuToCpu(dx12Device, drawRefSet.info.currFrameResourceIndex);
	}
	bool JDx12Adaptee::BeginDrawSceneSingleThread(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.graphicResourceM) ||
			!IsSameDevice(drawRefSet.currFrame) || !IsSameDevice(drawRefSet.cullingM) || !IsSameDevice(drawRefSet.depthDebug))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		 
		dx12Frame->ResetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		dx12Frame->ResetCmd(J_MAIN_THREAD_ORDER::END);

		//begin => draw scnee
		//end => draw gui
		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		ID3D12DescriptorHeap* descriptorHeaps[] = { dx12Gm->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
		cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		cmdList->OMSetStencilRef(Constants::commonStencilRef);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.sceneDraw = std::make_unique<JDx12GraphicSceneDrawSet>(dx12Device, dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.shadowMapDraw = std::make_unique<JDx12GraphicShadowMapDrawSet>(dx12Device, dx12Frame, dx12Gm, dx12Cm, drawRefSet.image, cmdList);
		dataSet.occDraw = std::make_unique<JDx12GraphicOccDrawSet>(dx12Device, dx12Frame, dx12Gm, dx12Cm, drawRefSet.depthTest, cmdList);
		dataSet.hzbCompute = std::make_unique<JDx12GraphicHzbOccComputeSet>(dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.hdExtract = std::make_unique<JDx12GraphicHdOccExtractSet>(dx12Cm, cmdList);
		dataSet.debugCompute = std::make_unique<JDx12GraphicDebugRsComputeSet>(dx12Device, dx12Gm, cmdList);
		dataSet.outline = std::make_unique<JDx12GraphicOutlineDrawSet>(dx12Device, dx12Gm, cmdList);
		dataSet.ssao = std::make_unique<JDx12GraphicSsaoComputeSet>(dx12Device, dx12Gm, dx12Frame, cmdList);
		dataSet.litCulling = std::make_unique<JDx12GraphicLightCullingTaskSet>(dx12Device, dx12Gm, dx12Cm, dx12Frame, cmdList);
		dataSet.litCullingDebug = std::make_unique<JDx12GraphicLightCullingDebugDrawSet>(dx12Device, dx12Gm, cmdList);
		return true;
	}
	bool JDx12Adaptee::EndDrawSceneSingeThread(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.graphicResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		ThrowIfFailedG(cmdList->Close());
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		return true;
	}
	bool JDx12Adaptee::SettingBeginFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.graphicResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		 
		dx12Frame->ResetCmd(drawRefSet.info.frameThreadCount);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, drawRefSet.cullingM, cmdList);
		dataSet.litCulling = std::make_unique<JDx12GraphicLightCullingTaskSet>(dx12Device, dx12Gm, dx12Cm, dx12Frame, cmdList);
		return true;
	}
	bool JDx12Adaptee::ExecuteBeginFrame(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		ThrowIfFailedG(cmdList->Close());
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		return true;
	}
	bool JDx12Adaptee::SettingMidFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMidFrameSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.graphicResourceM) ||
			!IsSameDevice(drawRefSet.currFrame) || !IsSameDevice(drawRefSet.cullingM) || !IsSameDevice(drawRefSet.depthDebug))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::MID);
		SettingDescriptorHeaps(dx12Gm, cmdList);
 
		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.sceneDraw = std::make_unique<JDx12GraphicSceneDrawSet>(dx12Device, dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.hzbCompute = std::make_unique<JDx12GraphicHzbOccComputeSet>(dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.hdExtract = std::make_unique<JDx12GraphicHdOccExtractSet>(dx12Cm, cmdList);
		dataSet.debugCompute = std::make_unique<JDx12GraphicDebugRsComputeSet>(dx12Device, dx12Gm, cmdList);
		dataSet.outline = std::make_unique<JDx12GraphicOutlineDrawSet>(dx12Device, dx12Gm, cmdList);
		dataSet.ssao = std::make_unique<JDx12GraphicSsaoComputeSet>(dx12Device, dx12Gm, dx12Frame, cmdList);
		dataSet.litCullingDebug = std::make_unique<JDx12GraphicLightCullingDebugDrawSet>(dx12Device, dx12Gm, cmdList);
		return true;
	}
	bool JDx12Adaptee::ExecuteMidFrame(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::MID);
		ThrowIfFailedG(cmdList->Close());
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		return true;
	}
	bool JDx12Adaptee::SettingEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.graphicResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		//J_MAIN_THREAD_ORDER::END reset 안됬을시 처리필요  
		if (!cond.isSceneDrawn)
			dx12Frame->ResetCmd(J_MAIN_THREAD_ORDER::END);
		 
		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::END);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		const uint dsIndex = dx12Gm->GetDefaultSceneDsInfo()->GetArrayIndex();
		auto mainDepthResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsIndex);
		auto mainDepthInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsIndex);

		JD3DUtility::ResourceTransition(cmdList, mainDepthResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		JD3DUtility::ResourceTransition(cmdList, CurrentBackBuffer(dx12Device, dx12Gm), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView(dx12Device, dx12Gm);
		D3D12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(mainDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::DSV));

		cmdList->ClearRenderTargetView(rtv, dx12Gm->GetBackBufferClearColor(), 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, true, nullptr);
		return true;
	}
	bool JDx12Adaptee::ExecuteEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.graphicResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::END);

		const uint dsIndex = dx12Gm->GetDefaultSceneDsInfo()->GetArrayIndex();
		auto mainDepthResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsIndex);
		 
		JD3DUtility::ResourceTransition(cmdList, CurrentBackBuffer(dx12Device, dx12Gm), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		JD3DUtility::ResourceTransition(cmdList, mainDepthResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		ThrowIfFailedG(cmdList->Close());
		 
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		 
		dx12Device->Present();
		const int nextBackBuffer = dx12Device->GetBackBufferIndex() + 1;
		//현재 태스크에 fence값
		//작업이 없을때는 fence 0
		//작업이 생겼을 경우 fence값을 증가시키므로
		//1부터 유효한 fence 값이다
		const int nextFence = dx12Device->GetFenceValue() + 1;

		const int backBufferIndex = nextBackBuffer % dx12Gm->GetResourceCapacity(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN);
		dx12Device->SetBackBufferIndex(backBufferIndex);
		dx12Device->SetFenceValue(nextFence);
		dx12Frame->SetFenceValue(nextFence);
		dx12Device->Signal();
		dx12Cm->UpdateFrameResourceIndex(drawRefSet.nextFrameIndex);
		return true;
	}
	bool JDx12Adaptee::SettingDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.graphicResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_THREAD_TASK_TYPE::OCC, threadIndex);

		SettingDescriptorHeaps(dx12Gm, cmdList);
		dataSet.occDraw = std::make_unique<JDx12GraphicOccDrawSet>(drawRefSet.device, dx12Frame, dx12Gm, drawRefSet.cullingM, drawRefSet.depthTest, cmdList);
		return true;
	}
	bool JDx12Adaptee::NotifyCompleteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ThrowIfFailedG(dx12Frame->GetCmd(J_THREAD_TASK_TYPE::OCC, threadIndex)->Close());
		SetEvent(dx12Frame->GetHandle(J_THREAD_TASK_TYPE::OCC, threadIndex));
		return true;
	}
	bool JDx12Adaptee::SettingDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadShadowMapTaskSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.graphicResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, drawRefSet.cullingM, cmdList);
		dataSet.shadowMapDraw = std::make_unique<JDx12GraphicShadowMapDrawSet>(drawRefSet.device, dx12Frame, dx12Gm, drawRefSet.cullingM, drawRefSet.image, cmdList);
		return true;
	}
	bool JDx12Adaptee::NotifyCompleteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ThrowIfFailedG(dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex)->Close());
		SetEvent(dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex));
		return true;
	}
	bool JDx12Adaptee::SettingDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadSceneTaskSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.graphicResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SCENE, threadIndex);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, drawRefSet.cullingM, cmdList);
		dataSet.occDraw = std::make_unique<JDx12GraphicOccDrawSet>(drawRefSet.device, dx12Frame, dx12Gm, drawRefSet.cullingM, drawRefSet.depthTest, cmdList);
		dataSet.sceneDraw = std::make_unique<JDx12GraphicSceneDrawSet>(drawRefSet.device, dx12Frame, dx12Gm, drawRefSet.cullingM, cmdList);
		return true;
	}
	bool JDx12Adaptee::NotifyCompleteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ThrowIfFailedG(dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SCENE, threadIndex)->Close());
		SetEvent(dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SCENE, threadIndex));
		return true;
	}
	bool JDx12Adaptee::ExecuteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12CommandQueue* commandQueue = dx12Device->GetCommandQueue();

		WaitForMultipleObjects(drawRefSet.info.frameThreadCount, dx12Frame->GetHandle(J_THREAD_TASK_TYPE::OCC), true, INFINITE);
		commandQueue->ExecuteCommandLists(drawRefSet.info.frameThreadCount, dx12Frame->GetBatchCmd(J_THREAD_TASK_TYPE::OCC));
		return true;
	}
	bool JDx12Adaptee::ExecuteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12CommandQueue* commandQueue = dx12Device->GetCommandQueue();

		WaitForMultipleObjects(drawRefSet.info.frameThreadCount, dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SHADOW_MAP), true, INFINITE);
		commandQueue->ExecuteCommandLists(drawRefSet.info.frameThreadCount, dx12Frame->GetBatchCmd(J_THREAD_TASK_TYPE::SHADOW_MAP));
		return true;
	}
	bool JDx12Adaptee::ExecuteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12CommandQueue* commandQueue = dx12Device->GetCommandQueue();

		WaitForMultipleObjects(drawRefSet.info.frameThreadCount, dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SCENE), true, INFINITE);
		commandQueue->ExecuteCommandLists(drawRefSet.info.frameThreadCount, dx12Frame->GetBatchCmd(J_THREAD_TASK_TYPE::SCENE));
		return true;
	}
	bool JDx12Adaptee::SettingBlurTask(const JGraphicDrawReferenceSet& drawRefSet,
		const ResourceHandle from,
		const ResourceHandle to,
		std::unique_ptr<JBlurDesc>&& desc,
		_Out_ std::unique_ptr<JGraphicBlurComputeSet>& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		ID3D12GraphicsCommandList* cmdList = dx12Device->GetPublicCmdList();

		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
		srcHandle.ptr = (UINT64)from;
		destHandle.ptr = (UINT64)to;

		dataSet = std::make_unique<JDx12GraphicBlurComputeSet>(dx12Device, std::move(desc), cmdList, srcHandle, destHandle);
		return true;
	}
	bool JDx12Adaptee::SettingBlurTask(const JGraphicDrawReferenceSet& drawRefSet,
		const JUserPtr<JGraphicResourceInfo>& info,
		std::unique_ptr<JBlurDesc>&& desc,
		_Out_ std::unique_ptr<JGraphicBlurComputeSet>& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
	 
		auto dx12Info = static_cast<JDx12GraphicResourceInfo*>(info.Get());
		if (!info->HasView(J_GRAPHIC_BIND_TYPE::UAV) || !dx12Info->HasOption(J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR))
			return false;
		//수정필요
		return false;
	}
	bool JDx12Adaptee::SettingMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet,
		const JUserPtr<JGraphicResourceInfo>& srcInfo,
		const JUserPtr<JGraphicResourceInfo>& modInfo,
		std::unique_ptr<JDownSampleDesc>&& desc,
		_Out_ std::unique_ptr<JGraphicDownSampleComputeSet>& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device))
			return false;

		std::vector<Core::JDataHandle> handle;
		if (!drawRefSet.graphicResourceM->SettingMipmapBind(drawRefSet.device, modInfo, false, handle))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12Device->GetPublicCmdList();

		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = dx12Gm->GetMPBGpuDescriptorHandle(handle[0], J_GRAPHIC_BIND_TYPE::SRV);		//0.. n-1
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = dx12Gm->GetMPBGpuDescriptorHandle(handle[1], J_GRAPHIC_BIND_TYPE::UAV);			//1.. n
		SettingDescriptorHeaps(dx12Gm, cmdList);

		dataSet = std::make_unique<JDx12GraphicDownSampleComputeSet>(dx12Device, dx12Gm, std::move(desc), std::move(handle), cmdList, srcHandle, destHandle);
		return true;
	}
}