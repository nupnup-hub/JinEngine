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
		class JGraphicResourceWatcher;
	}
	namespace Graphic
	{
		class JGraphicTextureHandle;
		class JGraphicResourceManager final : public JGraphicBufManagerInterface
		{
		private:
			friend class JGraphicImpl;
			friend class Editor::JGraphicResourceWatcher;
		private:
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
			Microsoft::WRL::ComPtr<ID3D12QueryHeap> occlusionQueryHeap;
			Microsoft::WRL::ComPtr<ID3D12QueryHeap> timeQueryHeap;
			uint rtvDescriptorSize = 0;
			uint dsvDescriptorSize = 0;
			uint cbvSrvUavDescriptorSize = 0;

			//fixed resource
			static constexpr uint mainBufDsCount = 1; 
			static constexpr uint swapChainBufferCount = 2;  
			//occlusion +  occlusionDebug

			Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer[swapChainBufferCount];

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

			Microsoft::WRL::ComPtr<ID3D12Resource> mainDepthStencil; 

			//Engine use
			//uint uavCapacity = depthDebugCapacity; 
			static constexpr uint occlusionDsCount = 1;
			static constexpr uint minOcclusionSize = 8;
			uint occlusionCount = 0;
			uint occlusionDebugCount = 0;
			uint occlusionQuaryCapacity = 2000;
			Microsoft::WRL::ComPtr<ID3D12Resource> occlusionQueryResult;
			Microsoft::WRL::ComPtr<ID3D12Resource> occlusionDepthMap;
			Microsoft::WRL::ComPtr<ID3D12Resource> occlusionDepthMipMap;
			Microsoft::WRL::ComPtr<ID3D12Resource> occlusionDebug;

			//User use
			const uint srvFixedCount = 1;
			uint user2DTextureCount = 0;
			uint user2DTextureCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> user2DTextureHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> user2DTextureResouce;

			uint userCubeMapCount = 0;
			uint userCubeMapCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> userCubeMapHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> userCubeMapResouce;

			uint renderResultCount = 0;
			uint renderResultCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> renderResultHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> renderResultResource;
			 
			uint shadowMapCount = 0;
			uint shadowMapCapacity = 1000;
			std::vector<std::unique_ptr<JGraphicTextureHandle>> shadowMapHandle;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> shadowMapResource;
			
			static constexpr uint occlusionDepthStencilCount = 1;
			static constexpr uint occlusionCapacity = 10;
			static constexpr uint occlusionDebugCapacity = occlusionCapacity;

			uint uavCount = 0;
			uint uavCapacity = occlusionCapacity - 1 + occlusionDebugCapacity;
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept;
		public:
			//Srv
			uint GetSrvOcclusionDepthMapStart()const noexcept;
			uint GetSrvOcclusionMipMapStart()const noexcept;
			uint GetSrvOcclusionDebugStart()const noexcept;
			uint GetSrvUser2DTextureStart()const noexcept;
			uint GetSrvUserCubeMapStart()const noexcept;
			uint GetSrvRenderResultStart()const noexcept;
			uint GetSrvShadowMapStart()const noexcept; 
			//Uav
			uint GetUavOcclusionMipMapStart()const noexcept;
			uint GetUavOcclusionDebugStart()const noexcept;
			//Rtv 
			uint GetRtvRenderResultStart()const noexcept;
			uint GetRtvShadowMapStart()const noexcept;
			//Dsv
			uint GetDsvOcclusionDepthStart()const noexcept;
			uint GetDsvShadowMapStart()const noexcept;
			//Total Rtv
			uint GetTotalRtvCount()const noexcept;
			uint GetTotalRtvCapacity()const noexcept;
			//TotalDsv
			uint GetTotalDsvCount()const noexcept;
			uint GetTotalDsvCapacity()const noexcept;
			//TotalRs
			uint GetTotalRsCount()const noexcept;
			uint GetTotalRsCapacity()const noexcept;
			//Total Texture
			uint GetTotalTextureCount()const noexcept;
			uint GetTotalTextureCapacity()const noexcept;
			//Occlusion
			uint GetOcclusionQueryHeapCapacity()const noexcept;
			uint GetOcclusionDsCount()const noexcept;
			//Engine DepthStencil
			static constexpr uint GetMainDsIndex()noexcept { return 0; }
			static constexpr uint GetOcclusionDsIndex()noexcept { return 1; }
		private:
			ID3D12QueryHeap* GetOcclusionQueryHeap()const noexcept;
			ID3D12Resource* GetOcclusionResult()const noexcept;
		private:
			void BuildRtvDescriptorHeaps(ID3D12Device* device);
			void BuildDsvDescriptorHeaps(ID3D12Device* device);
			void BuildSrvDescriptorHeaps(ID3D12Device* device); 
			void BuildOccQueryHeaps(ID3D12Device* device);
		private:
			JGraphicTextureHandle* Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap,
				const std::wstring& path,
				const std::wstring& oriFormat,
				ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList);
			JGraphicTextureHandle* CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap,
				const std::wstring& path,
				const std::wstring& oriFormat,
				ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList);
			JGraphicTextureHandle* CreateRenderTargetTexture(ID3D12Device* device, const uint width, const uint height);
			JGraphicTextureHandle* CreateShadowMapTexture(ID3D12Device* device, const uint width, const uint height);
			bool DestroyGraphicTextureResource(ID3D12Device* device, JGraphicTextureHandle** handle);
		private:
			void ReBind2DTexture(ID3D12Device* device, const uint resourceIndex, const uint heapIndex);
			void ReBindCubeMap(ID3D12Device* device, const uint resourceIndex, const uint heapIndex);
			void ReBindRenderTarget(ID3D12Device* device, const uint resourceIndex, const uint rtvHeapIndex, const uint srvHeapIndex);
			void ReBindShadowMapTexture(ID3D12Device* device, const uint resourceIndex, const uint dsvHeapIndex, const uint srvHeapIndex);
		private: 
			void CreateMainDepthStencilResource(ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList,
				const uint viewWidth,
				const uint viewHeight,
				bool m4xMsaaState,
				uint m4xMsaaQuality); 
			void CreateOcclusionQueryResource(ID3D12Device* device);
			void CreateOcclusionHZBResource(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const uint occWidth, const uint occHeight);
		private:
			void Clear();
		public:
			JGraphicResourceManager();
			~JGraphicResourceManager();
		};
	}
}