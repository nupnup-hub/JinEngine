#include"JDx12Adaptee.h"

#include"../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../Culling/Dx/JDx12CullingManager.h"
#include"../DepthMap/Dx/JDx12DepthMapDebug.h"
#include"../DepthMap/Dx/JDx12DepthTest.h"
#include"../Device/Dx/JDx12GraphicDevice.h"
#include"../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../Outline/Dx/JDx12Outline.h"
#include"../Culling/Occlusion/Dx/JDx12HardwareOccCulling.h"
#include"../Culling/Occlusion/Dx/JDx12HZBOccCulling.h"
#include"../Scene/Dx/JDx12SceneDraw.h"
#include"../ShadowMap/Dx/JDx12ShadowMap.h" 
#include"../Blur/Dx/JDx12Blur.h"
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
	std::unique_ptr<JDepthMapDebug> JDx12Adaptee::CreateDepthMapDebug()
	{
		return std::make_unique<JDx12DepthMapDebug>();
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
	std::unique_ptr<JOutline> JDx12Adaptee::CreateOutlineDraw()
	{
		return std::make_unique<JDx12Outline>();
	}
	std::unique_ptr<JBlur> JDx12Adaptee::CreateBlur()
	{
		return std::make_unique<JDx12Blur>();
	}
	bool JDx12Adaptee::BeginDrawSceneSingleThread(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.gResourceM) ||
			!IsSameDevice(drawRefSet.currFrame) || !IsSameDevice(drawRefSet.cullingM) || !IsSameDevice(drawRefSet.depthDebug))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
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
		dataSet.shadowMapDraw = std::make_unique<JDx12GraphicShadowMapDrawSet>(dx12Device, dx12Frame, dx12Gm, dx12Cm, drawRefSet.blur, cmdList);
		dataSet.occDraw = std::make_unique<JDx12GraphicOccDrawSet>(dx12Device, dx12Frame, dx12Gm, dx12Cm, drawRefSet.depthTest, cmdList);
		dataSet.occDebug = std::make_unique<JDx12GraphicOccDebugDrawSet>(dx12Device, dx12Gm, drawRefSet.depthDebug, cmdList);
		dataSet.hzbCompute = std::make_unique<JDx12GraphicHzbOccComputeSet>(dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.hdExtract = std::make_unique<JDx12GraphicHdOccExtractSet>(dx12Cm, cmdList);
		dataSet.depthMapDebug = std::make_unique<JDx12GraphicDepthMapDebugObjectSet>(dx12Gm, cmdList);
		dataSet.outline = std::make_unique<JDx12GraphicOutlineObjectSet>(dx12Device, dx12Gm, cmdList); 
		return true;
	}
	bool JDx12Adaptee::EndDrawSceneSingeThread(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.gResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		ThrowIfFailedG(cmdList->Close());
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		return true;
	}
	bool JDx12Adaptee::SettingBeginFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.gResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		 
		dx12Frame->ResetCmd(drawRefSet.info.frameThreadCount);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, drawRefSet.cullingM, cmdList);
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
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.gResourceM) ||
			!IsSameDevice(drawRefSet.currFrame) || !IsSameDevice(drawRefSet.cullingM) || !IsSameDevice(drawRefSet.depthDebug))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::MID);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.sceneDraw = std::make_unique<JDx12GraphicSceneDrawSet>(dx12Device, dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.occDebug = std::make_unique<JDx12GraphicOccDebugDrawSet>(dx12Device, dx12Gm, drawRefSet.depthDebug, cmdList);
		dataSet.hzbCompute = std::make_unique<JDx12GraphicHzbOccComputeSet>(dx12Frame, dx12Gm, dx12Cm, cmdList);
		dataSet.hdExtract = std::make_unique<JDx12GraphicHdOccExtractSet>(dx12Cm, cmdList);
		dataSet.depthMapDebug = std::make_unique<JDx12GraphicDepthMapDebugObjectSet>(dx12Gm, cmdList);
		dataSet.outline = std::make_unique<JDx12GraphicOutlineObjectSet>(dx12Device, dx12Gm, cmdList);
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
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.gResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
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
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.gResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
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
		return true;
	}
	bool JDx12Adaptee::SettingDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.gResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
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
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.gResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(dx12Frame, dx12Gm, drawRefSet.cullingM, cmdList);
		dataSet.shadowMapDraw = std::make_unique<JDx12GraphicShadowMapDrawSet>(drawRefSet.device, dx12Frame, dx12Gm, drawRefSet.cullingM, drawRefSet.blur, cmdList);
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
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.gResourceM) || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.gResourceM);
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
	J_GRAPHIC_DEVICE_TYPE JDx12Adaptee::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
}