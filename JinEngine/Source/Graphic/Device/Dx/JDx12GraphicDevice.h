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
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> publicCmdListAlloc;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> publicCmdList;
		private:
			Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
			Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
			Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
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
		public:
			JDx12GraphicDevice() = default;
			~JDx12GraphicDevice() = default;
		public:
			bool CreateDeviceObject() final;
			bool CreateRefResourceObject(const JGraphicDeviceInitSet& dataSet) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			std::unique_ptr<JGraphicDevice::RefSet> GetDeviceRefSet()const noexcept final; 
			GraphicFence GetFenceValue()const noexcept final;
			void GetLastDeviceError(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg)final;
			ID3D12Device* GetDevice()const noexcept; 
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
			bool CanStartPublicCommand()const noexcept final;
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
			void LogAdapters(const JGraphicDeviceInitSet& refSet);
			void LogAdapterOutputs(const JGraphicDeviceInitSet& refSet, IDXGIAdapter* adapter);
			void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
			void CreateCommandObjects();
			void CreateSwapChain(const JGraphicDeviceInitSet& refSet);
		public:
			void ResizeWindow(const JGraphicDeviceInitSet& refSet) final;
		};
	}
}