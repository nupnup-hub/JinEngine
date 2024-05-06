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
#include"JDxGraphicDeviceInterface.h"  
#include<dxgidebug.h> 
#include<wrl/client.h> 
#include<dxgi1_4.h>
#include<d3d12.h>  
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicManager; 
		class JDx12GraphicDevice final: public JDxGraphicDeviceInterface
		{ 
		public: 
			struct RefSet : public JGraphicDevice::RefSet
			{
			public:
				ID3D12Device* device;
			public:
				RefSet(ID3D12Device* device);
			public:
				J_GRAPHIC_DEVICE_TYPE GetDeviceType() const noexcept;
			};
		private: 
			D3D12_VIEWPORT screenViewport;
			D3D12_RECT scissorRect; 
		private: 
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
		private:
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> publicCmdListAlloc;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> publicCmdList;
		private:
			Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
			Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
			Microsoft::WRL::ComPtr<ID3D12Device5> raytracingDevice;
			Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
			Microsoft::WRL::ComPtr<ID3D12Fence> fence; 
		private:
			UINT64 currentFence = 0; 
			int currBackBuffer = 0;
		private:
			// Set true to use 4X MSAA (?.1.8).  The default is false.
			uint m4xMsaaQuality = 0;      // quality level of 4X MSAA
			bool m4xMsaaState = false;    // 4X MSAA enabled
		private:
			int syncInterval = 0;
		private:
			bool stCommand = false;
			bool isRaytracingSupported = false;
		public:
			JDx12GraphicDevice() = default;
			~JDx12GraphicDevice();
		public:
			bool CreateDeviceObject() final;
			bool CreateRefResourceObject(const JGraphicDeviceInitSet& dataSet)final;
		public:
			void Clear()noexcept final;
		private:
			void ClearResource()noexcept;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			std::unique_ptr<JGraphicDevice::RefSet> GetDeviceRefSet()const noexcept final; 
			GraphicFence GetFenceValue()const noexcept final;
			void GetLastDeviceError(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)final;
			ID3D12Device* GetDevice()const noexcept; 
			ID3D12Device5* GetRaytracingDevice()const noexcept;
			ID3D12CommandQueue* GetCommandQueue()const noexcept; 
			ID3D12GraphicsCommandList* GetPublicCmdList()const noexcept;
			HWND GetSwapChainOutputWindowHandle()const noexcept;
			D3D12_VIEWPORT GetViewPort()const noexcept;
			D3D12_RECT GetRect()const noexcept;
			uint GetM4xMsaaQuality()const noexcept;
			bool GetM4xMsaaState()const noexcept;
			int GetBackBufferIndex()const noexcept;
		public:
			void SetFenceValue(const GraphicFence value)noexcept;
			void SetBackBufferIndex(const int value)noexcept;
		public: 
			bool IsSupportPublicCommand()const noexcept final; 
			bool IsPublicCommandStared()const noexcept final;
			bool IsRaytracingSupported()const noexcept final;
			bool CanStartPublicCommand()const noexcept final;
			bool CanBuildGpuAccelerator()const noexcept final;
		public:
			void CalViewportAndRect(const JVector2F rtSize, const bool restrictRange, _Out_ D3D12_VIEWPORT& viweport, _Out_ D3D12_RECT& rect)const noexcept;
		public: 
			void Present();
			void Signal();
		public:
			void StartPublicCommand()final;
			void EndPublicCommand()final;
			void FlushCommandQueue()final; 
		public:
			void UpdateWait(const GraphicFence frameFence) final;
		private: 
			void LogAdapters(const JGraphicDeviceInitSet& dataSet);
			void LogAdapterOutputs(const JGraphicDeviceInitSet& dataSet, IDXGIAdapter* adapter);
			void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
			void CreateCommandObjects();
			void CreateSwapChain(const JGraphicDeviceInitSet& dataSet);
		public:
			void ResizeWindow(const JGraphicDeviceInitSet& dataSet) final;
			void NotifyChangedBackBufferFormat(const JGraphicDeviceInitSet& dataSet) final;
		};
	}
}