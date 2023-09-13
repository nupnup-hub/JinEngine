#pragma once
#include"../JImGuiInitData.h"
#include<d3dx12.h>

namespace JinEngine
{
	namespace Editor
	{
		struct JDx12ImGuiInitData : public JImGuiInitData
		{
		public:
			HANDLE windowHandle;
			ID3D12Device* device;
			ID3D12DescriptorHeap* srvHeap;
			DXGI_FORMAT backbufferFormat;
			uint numOfFrameCount;
		public:
			JDx12ImGuiInitData(HANDLE windowHandle,
				ID3D12Device* device,
				ID3D12DescriptorHeap* srvHeap,
				DXGI_FORMAT backbufferFormat,
				uint numOfFrameCount);
		public:
			Graphic::J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
	}
}