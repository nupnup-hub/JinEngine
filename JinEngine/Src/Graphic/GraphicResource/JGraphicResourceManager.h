#pragma once
#include"JGraphicResourceType.h"
#include"JGraphicResourceInfo.h"
#include"../JGraphicConstants.h"
#include"../../Core/JDataType.h"  
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../../Lib/DirectX/Tk/Src/d3dx12.h" 
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
		class JGraphicResourceManager
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JGraphicResourceManager)
		private:
			using BindViewPtr = void(JGraphicResourceManager::*)(ID3D12Device*, const uint);
		private:
			struct ResourceViewInfo
			{
			public:
				int count = 0;
				int capacity = 0;
				int offset = 0;
			public:
				void ClearCount();
				void ClearAllData();
			public:
				int GetNextViewIndex()const noexcept;
				bool HasSpace()const noexcept;
			};
			struct ResourceTypeDesc
			{
			public:
				int count = 0;
				int capacity = 0;
			public:
				ResourceViewInfo viewInfo[(int)J_GRAPHIC_BIND_TYPE::COUNT];
			public:
				void ClearCount();
				void ClearAllData();
			public:
				bool HasSpace()const noexcept;
			};
		private:
			//friend class JGraphic;
			friend class Editor::JGraphicResourceWatcher;
		private:
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
			Microsoft::WRL::ComPtr<ID3D12QueryHeap> occlusionQueryHeap;
			uint rtvDescriptorSize = 0;
			uint dsvDescriptorSize = 0;
			uint cbvSrvUavDescriptorSize = 0;
		private:
			Microsoft::WRL::ComPtr<ID3D12Resource> occlusionQueryResult;
		private:
			std::vector<JOwnerPtr<JGraphicResourceInfo>> resource[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			ResourceTypeDesc typeDesc[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		private:
			const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			const DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		public:
			void Initialize(ID3D12Device* device);
			void Clear();
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetFirstGpuSrvDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			ID3D12DescriptorHeap* GetDescriptorHeap(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			DXGI_FORMAT GetBackBufferFormat()const noexcept;
			DXGI_FORMAT GetDepthStencilFormat()const noexcept;
			DirectX::XMVECTORF32 GetBackBufferClearColor()const noexcept;
		public:
			static uint GetSwapChainBufferCount()noexcept;
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetViewOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetTotalViewCount(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetTotalViewCapacity(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			ID3D12Resource* GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			JGraphicResourceInfo* GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
		public:
			static uint GetOcclusionMipMapViewCapacity()noexcept;
			static uint GetOcclusionMinSize()noexcept;
			uint GetOcclusionQueryCapacity()const noexcept;
			ID3D12QueryHeap* GetOcclusionQueryHeap()const noexcept;
			ID3D12Resource* GetOcclusionQueryResult()const noexcept;
		private:
			uint GetHeapIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType);
		public:
			void CreateSwapChainBuffer(ID3D12Device* device, IDXGISwapChain* swapChain, const uint width, const uint height);
			void CreateMainDepthStencilResource(ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList,
				const uint viewWidth,
				const uint viewHeight,
				bool m4xMsaaState,
				uint m4xMsaaQuality);
			void CreateDebugDepthStencilResource(ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList,
				const uint viewWidth,
				const uint viewHeight);
			void CreateOcclusionQueryResource(ID3D12Device* device);
			void CreateOcclusionHZBResource(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const uint occWidth, const uint occHeight);
			JUserPtr<JGraphicResourceInfo> Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
				const std::wstring& path,
				const std::wstring& oriFormat,
				ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList);
			JUserPtr<JGraphicResourceInfo> CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
				const std::wstring& path,
				const std::wstring& oriFormat,
				ID3D12Device* device,
				ID3D12GraphicsCommandList* commandList);
			JUserPtr<JGraphicResourceInfo> CreateRenderTargetTexture(ID3D12Device* device, const uint width, const uint height);
			JUserPtr<JGraphicResourceInfo> CreateShadowMapTexture(ID3D12Device* device, const uint width, const uint height);
			bool DestroyGraphicTextureResource(ID3D12Device* device, JGraphicResourceInfo* info);
		private:
			JOwnerPtr<JGraphicResourceInfo> CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource);
		private:
			BindViewPtr GetResourceBindViewPtr(const J_GRAPHIC_RESOURCE_TYPE rType);
			void SetViewCount(JGraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const int nextViewIndex);
			void BindSwapChain(ID3D12Device* device, const uint resourceIndex);
			void BindMainDepthStencil(ID3D12Device* device, const uint resourceIndex);
			void BindMainDepthStencilDebug(ID3D12Device* device, const uint resourceIndex);
			void BindDebugDepthStencil(ID3D12Device* device, const uint resourceIndex);
			void BindOcclusionHZBDepthMap(ID3D12Device* device, const uint resourceIndex);
			void BindOcclusionHZBDepthMipMap(ID3D12Device* device, const uint resourceIndex);
			void BindOcclusionHZBDebug(ID3D12Device* device, const uint resourceIndex);
			void Bind2DTexture(ID3D12Device* device, const uint resourceIndex);
			void BindCubeMap(ID3D12Device* device, const uint resourceIndex);
			void BindRenderTarget(ID3D12Device* device, const uint resourceIndex);
			void BindShadowMap(ID3D12Device* device, const uint resourceIndex);
		private:
			void BuildRtvDescriptorHeaps(ID3D12Device* device);
			void BuildDsvDescriptorHeaps(ID3D12Device* device);
			void BuildSrvDescriptorHeaps(ID3D12Device* device);
			void BuildOccQueryHeaps(ID3D12Device* device); 
		public:
			static void RegisterTypeData();
		public:
			JGraphicResourceManager();
			~JGraphicResourceManager();
		};
	}
}