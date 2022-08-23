#pragma once
#include"../../Lib/DirectX/d3dx12.h"
#include"../Core/JDataType.h" 
#include"JGraphicBufInterface.h"
#include<vector>
#include<string>
#include<memory>
#include<WindowsX.h> 
#include<wrl/client.h>  
#include<d3d12.h>
#include<dxgi1_4.h>  

namespace JinEngine
{
	namespace Editor
	{
		class JGraphicTextureHandle;
	}
	namespace Graphic
	{
		class JGraphicResourceManager : public JGraphicBufManagerInterface
		{
		private:
			friend class Editor::JGraphicResourceWatcher;
			friend class JGraphicImpl;
		private:
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
			uint rtvDescriptorSize = 0;
			uint dsvDescriptorSize = 0;
			uint cbvSrvUavDescriptorSize = 0;

			static constexpr uint swapChainBuffercount = 3;
			Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer[swapChainBuffercount];

			DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			//SrvHeap
			//Fixed: ImGuiTexture + editorTexture ..
			//Dynamic: userTexture ..
			//Dynamic: RenderResult(editorPreview .. Common .. editorShadowMap .. ShadowMap)

			//RtvHeap
			//Fixed: swapChain ..
			//Dynamic: renderResultHandle .. ShadowMap

			//DsvHeap
			//Fixed: 1( CommonDepth)
			//Dynamic: ShadowMap

			const uint imGuiTextureCount = 1;
			uint user2DTextureCount = 0;
			uint user2DTextureCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> user2DTextureHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> user2DTextureResouce;

			uint userCubeTextureCount = 0;
			uint userCubeTextureCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> userCubeTextureHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> userCubeTextureResouce;

			uint renderResultCount = 0;
			uint renderResultCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> renderResultHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> renderResultResource;

			uint shadowMapCount = 0;
			uint shadowMapCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> shadowMapHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> shadowMapResource;

			Microsoft::WRL::ComPtr<ID3D12Resource> depthStencil;
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuRtvDescriptorHandle(int index)const noexcept;

			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDsvDescriptorHandle(int index)const noexcept;

			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept;

			uint GetSrvUser2DTextureStart()const noexcept;
			uint GetSrvUserCubeTextureStart()const noexcept;
			uint GetSrvRenderResultStart()const noexcept;
			uint GetSrvShadowMapStart()const noexcept;

			uint GetRtvRenderResultStart()const noexcept;
			uint GetRtvShadowMapStart()const noexcept;

			uint GetDsvShadowMapStart()const noexcept;

			uint GetTotalRtvCount()const noexcept;
			uint GetTotalRtvCapacity()const noexcept;

			uint GetTotalDsvCount()const noexcept;
			uint GetTotalDsvCapacity()const noexcept;

			uint GetTotalRsCount()const noexcept;
			uint GetTotalRsCapacity()const noexcept;

			uint GetTotalTextureCount()const noexcept;
			uint GetTotalTextureCapacity()const noexcept;
		private:
			void BuildRtvDescriptorHeaps(ID3D12Device* device);
			void BuildDsvDescriptorHeaps(ID3D12Device* device);
			void BuildSrvDescriptorHeaps(ID3D12Device* device);
			void BuildDepthStencilResource(ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList,
				const uint viewWidth,
				const uint viewHeight,
				bool m4xMsaaState,
				uint m4xMsaaQuality);

			JGraphicTextureHandle* Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap,
				const std::string& path,
				ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList);
			JGraphicTextureHandle* CreateCubeTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap,
				const std::string& path,
				ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList);
			JGraphicTextureHandle* CreateRenderTargetTexture(ID3D12Device* device, const uint width, const uint height);
			JGraphicTextureHandle* CreateShadowMapTexture(ID3D12Device* device, const uint width, const uint height);
			bool DestroyGraphicTextureResource(ID3D12Device* device, JGraphicTextureHandle** handle);

			void ReBind2DTexture(ID3D12Device* device, const uint resourceIndex, const uint heapIndex);
			void ReBindCubeTexture(ID3D12Device* device, const uint resourceIndex, const uint heapIndex);
			void ReBindRenderTarget(ID3D12Device* device, const uint resourceIndex, const uint rtvHeapIndex, const uint srvHeapIndex);
			void ReBindShadowMapTexture(ID3D12Device* device, const uint resourceIndex, const uint dsvHeapIndex, const uint srvHeapIndex);
		public:
			JGraphicResourceManager();
			~JGraphicResourceManager();
		};
	}
}