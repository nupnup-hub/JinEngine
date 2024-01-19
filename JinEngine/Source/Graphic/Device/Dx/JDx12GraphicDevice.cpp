#include"JDx12GraphicDevice.h"
#include"../../GraphicResource/JGraphicResourceManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../Utility/Dx/JD3DUtility.h" 
#include"../../../Core/Exception/JExceptionMacro.h"
#include<assert.h>
#include<vector>

namespace JinEngine::Graphic
{
	JDx12GraphicDevice::RefSet::RefSet(ID3D12Device* device)
		:device(device)
	{}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDevice::RefSet::GetDeviceType() const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	bool JDx12GraphicDevice::CreateDeviceObject()
	{
#if defined(GRAPIC_DEBUG) 
		// Enable the D3D12 DEBUG layer.
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailedHr(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
#endif
		ThrowIfFailedHr(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
		//ThrowIfFailedHr(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,  IID_PPV_ARGS(&dxgiFactory)));

		HRESULT hardwareResult = D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&d3dDevice));
		d3dDevice->SetName(L"JinEngineDx12 Device");

#if defined(GRAPIC_DEBUG) 
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(d3dDevice.As(&d3dInfoQueue)))
		{
			//d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			//d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			D3D12_MESSAGE_ID hide[1] =
			{
				//RESOLVE_QUERY_INVALID_QUERY_STATE
				//occ hd query는 항상 유효한 파라미터만 참조하므로 resolve시 정확하지않은 data를 가져와도 된다.
				//D3D12_MESSAGE_ID에 정의되어있지않으므로 오류번호로 대체
				(D3D12_MESSAGE_ID)1319
			};
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
#endif 

		if (FAILED(hardwareResult))
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> pWrapAdapter;
			ThrowIfFailedHr(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWrapAdapter)));

			ThrowIfFailedHr(D3D12CreateDevice(
				pWrapAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&d3dDevice)));
		}

		ThrowIfFailedHr(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		CreateCommandObjects();
		return true;
	}
	bool JDx12GraphicDevice::CreateRefResourceObject(const JGraphicDeviceInitSet& dataSet)
	{
		if (dataSet.graphicResourceM == nullptr || dataSet.graphicResourceM->GetDeviceType() != GetDeviceType())
			return false;

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = static_cast<JDx12GraphicResourceManager*>(dataSet.graphicResourceM)->GetBackBufferFormat();
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
		LogAdapters(dataSet);
#endif  
		CreateSwapChain(dataSet);
		return true;
	}
	void JDx12GraphicDevice::Clear()
	{
		FlushCommandQueue();
		swapChain.Reset();
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
#ifdef GRAPIC_DEBUG
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
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDevice::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	std::unique_ptr<JGraphicDevice::RefSet> JDx12GraphicDevice::GetDeviceRefSet()const noexcept
	{
		return std::make_unique<JDx12GraphicDevice::RefSet>(d3dDevice.Get());
	}
	GraphicFence JDx12GraphicDevice::GetFenceValue()const noexcept
	{
		return currentFence;
	}
	void JDx12GraphicDevice::GetLastDeviceError(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)
	{
		HRESULT gfxHr = d3dDevice->GetDeviceRemovedReason();

		std::wostringstream oss;
		oss << std::hex << gfxHr << std::endl;
		errorCode = oss.str();
		errorMsg = Core::JException::TranslateErrorCode(gfxHr);
	}
	ID3D12Device* JDx12GraphicDevice::GetDevice()const noexcept
	{
		return d3dDevice.Get();
	}
	ID3D12CommandQueue* JDx12GraphicDevice::GetCommandQueue()const noexcept
	{
		return commandQueue.Get();
	}
	ID3D12GraphicsCommandList* JDx12GraphicDevice::GetPublicCmdList()const noexcept
	{
		return publicCmdList.Get();
	}
	HWND JDx12GraphicDevice::GetSwapChainOutputWindowHandle()const noexcept
	{
		if (swapChain == nullptr)
			return NULL;

		DXGI_SWAP_CHAIN_DESC desc;
		return swapChain->GetDesc(&desc) == S_OK ? desc.OutputWindow : NULL;
	}
	D3D12_VIEWPORT JDx12GraphicDevice::GetViewPort()const noexcept
	{
		return screenViewport;
	}
	D3D12_RECT JDx12GraphicDevice::GetRect()const noexcept
	{
		return scissorRect;
	}
	uint JDx12GraphicDevice::GetM4xMsaaQuality()const noexcept
	{
		return m4xMsaaQuality;
	}
	bool JDx12GraphicDevice::GetM4xMsaaState()const noexcept
	{
		return m4xMsaaState;
	}
	int JDx12GraphicDevice::GetBackBufferIndex()const noexcept
	{
		return currBackBuffer;
	}
	void JDx12GraphicDevice::SetFenceValue(const GraphicFence value)noexcept
	{
		currentFence = value;
	}
	void JDx12GraphicDevice::SetBackBufferIndex(const int value)noexcept
	{
		currBackBuffer = value;
	}
	bool JDx12GraphicDevice::IsSupportPublicCommand()const noexcept
	{
		return true;
	}
	bool JDx12GraphicDevice::IsPublicCommandStared()const noexcept
	{
		return stCommand;
	}
	bool JDx12GraphicDevice::CanStartPublicCommand()const noexcept
	{
		return !stCommand;
	}
	void JDx12GraphicDevice::CalViewportAndRect(const JVector2F rtSize, const bool restrictRange, _Out_ D3D12_VIEWPORT& viweport, _Out_ D3D12_RECT& rect)const noexcept
	{
		viweport = screenViewport;
		rect = scissorRect;

		if (restrictRange)
		{
			if (rtSize.x < viweport.Width)
			{
				viweport.Width = rtSize.x;
				rect.right = rtSize.x;
			}
			if (rtSize.y < viweport.Height)
			{
				viweport.Height = rtSize.y;
				rect.bottom = rtSize.y;
			}
		}
		else
		{
			viweport.Width = rect.right = rtSize.x;
			viweport.Height = rect.bottom = rtSize.y;
		} 
	}
	void JDx12GraphicDevice::Present()
	{
		ThrowIfFailedG(swapChain->Present(syncInterval, 0));
	}
	void JDx12GraphicDevice::Signal()
	{
		commandQueue->Signal(fence.Get(), currentFence);
	}
	void JDx12GraphicDevice::StartPublicCommand()
	{
		if (!stCommand)
		{
			ThrowIfFailedHr(publicCmdList->Reset(publicCmdListAlloc.Get(), nullptr));
			stCommand = true;
		}
	}
	void JDx12GraphicDevice::EndPublicCommand()
	{
		if (stCommand)
		{
			ThrowIfFailedG(publicCmdList->Close());
			ID3D12CommandList* cmdsLists[] = { publicCmdList.Get() };
			commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
			stCommand = false;
		}
	}
	void JDx12GraphicDevice::FlushCommandQueue()
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
	void JDx12GraphicDevice::UpdateWait(const GraphicFence frameFence)
	{
		if (frameFence != 0 && fence->GetCompletedValue() < frameFence)
		{
			//nullptr, FALSE, FALSE, EVENT_ALL_ACCESS 
			HANDLE eventHandle = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
			ThrowIfFailedHr(fence->SetEventOnCompletion(frameFence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}
	void JDx12GraphicDevice::LogAdapters(const JGraphicDeviceInitSet& dataSet)
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
			LogAdapterOutputs(dataSet, adapterList[i]);
			ReleaseCom(adapterList[i]);
		}
	}
	void JDx12GraphicDevice::LogAdapterOutputs(const JGraphicDeviceInitSet& dataSet, IDXGIAdapter* adapter)
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

			LogOutputDisplayModes(output, static_cast<JDx12GraphicResourceManager*>(dataSet.graphicResourceM)->GetBackBufferFormat());
			ReleaseCom(output);
			++i;
		}
	}
	void JDx12GraphicDevice::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
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
	void JDx12GraphicDevice::CreateCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ThrowIfFailedHr(d3dDevice->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&commandQueue)));
		commandQueue->SetName(L"commandQueue");

		ThrowIfFailedHr(d3dDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(publicCmdListAlloc.GetAddressOf())));
		publicCmdListAlloc->SetName(L"PublicCmdAlloc");

		ThrowIfFailedHr(d3dDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			publicCmdListAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(publicCmdList.GetAddressOf())));

		publicCmdList->Close();
		publicCmdList->SetName(L"PublicCmd");
	}
	void JDx12GraphicDevice::CreateSwapChain(const JGraphicDeviceInitSet& dataSet)
	{
		static_cast<JDx12GraphicResourceManager*>(dataSet.graphicResourceM)->CreateSwapChainBuffer(d3dDevice.Get(),
			dxgiFactory.Get(),
			commandQueue.Get(),
			swapChain,
			dataSet.info.width,
			dataSet.info.height,
			m4xMsaaState,
			m4xMsaaQuality);
	}
	void JDx12GraphicDevice::ResizeWindow(const JGraphicDeviceInitSet& dataSet)
	{
		assert(d3dDevice);
		assert(swapChain);
		assert(publicCmdListAlloc);

		// Flush before changing any resources.
		//FlushCommandQueue();
		currBackBuffer = 0;
		CreateSwapChain(dataSet);
		//FlushCommandQueue();
		screenViewport.TopLeftX = 0;
		screenViewport.TopLeftY = 0;
		screenViewport.Width = static_cast<float>(dataSet.info.width);
		screenViewport.Height = static_cast<float>(dataSet.info.height);
		screenViewport.MinDepth = 0.0f;
		screenViewport.MaxDepth = 1.0f;
		scissorRect = { 0, 0, dataSet.info.width, dataSet.info.height };
	}
}