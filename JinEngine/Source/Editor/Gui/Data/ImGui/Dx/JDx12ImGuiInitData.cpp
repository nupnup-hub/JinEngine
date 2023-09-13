#include"JDx12ImGuiInitData.h"

namespace JinEngine::Editor
{
	JDx12ImGuiInitData::JDx12ImGuiInitData(HANDLE windowHandle,
		ID3D12Device* device,
		ID3D12DescriptorHeap* srvHeap,
		DXGI_FORMAT backbufferFormat,
		uint numOfFrameCount)
		:windowHandle(windowHandle), 
		device(device),
		srvHeap(srvHeap), 
		backbufferFormat(backbufferFormat),
		numOfFrameCount(numOfFrameCount)
	{}
	Graphic::J_GRAPHIC_DEVICE_TYPE JDx12ImGuiInitData::GetDeviceType()const noexcept 
	{
		return Graphic::J_GRAPHIC_DEVICE_TYPE::DX12;
	}
}