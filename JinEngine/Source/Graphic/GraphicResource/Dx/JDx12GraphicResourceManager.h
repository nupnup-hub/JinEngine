#pragma once
#include"../JGraphicResourceManager.h"  
#include"../../../Core/JCoreEssential.h"  
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
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
	class JMeshGeometry;
	namespace Graphic
	{
		class JDx12GraphicResourceInfo;
		class JDx12GraphicResourceManager final : public JGraphicResourceManager
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDx12GraphicResourceManager)
		private:
			using BindViewPtr = void(JDx12GraphicResourceManager::*)(ID3D12Device*, const uint);
		private:
			//friend class JGraphic;
			friend class Editor::JGraphicResourceWatcher;
		private:
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
			uint rtvDescriptorSize = 0;
			uint dsvDescriptorSize = 0;
			uint cbvSrvUavDescriptorSize = 0;
		private:
			std::vector<JOwnerPtr<JDx12GraphicResourceInfo>> resource[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			ResourceTypeDesc typeDesc[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		private:
			JUserPtr<JGraphicResourceInfo> defaultSceneDsInfo;	//cashing
		private:
			const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			const DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		public:
			void Initialize(JGraphicDevice* device)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuRtvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDsvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvDescriptorHandle(int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept;
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex);
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType,
				const J_GRAPHIC_BIND_TYPE bType,
				const int rIndex,
				const int bIndex);
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetFirstGpuSrvDescriptorHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			ID3D12DescriptorHeap* GetDescriptorHeap(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			DXGI_FORMAT GetBackBufferFormat()const noexcept;
			DXGI_FORMAT GetDepthStencilFormat()const noexcept;
			DirectX::XMVECTORF32 GetBackBufferClearColor()const noexcept; 
			D3D12_VERTEX_BUFFER_VIEW VertexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept;
			D3D12_INDEX_BUFFER_VIEW IndexBufferView(const JUserPtr<JMeshGeometry>& mesh)const noexcept;
		public: 
			static uint GetSwapChainBufferCount()noexcept;
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			uint GetResourceCapacity(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetViewCapacity(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetViewOffset(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetTotalViewCount(const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetTotalViewCapacity(const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			ID3D12Resource* GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			JGraphicResourceInfo* GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept final; 
			JDx12GraphicResourceInfo* GetDxInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			JDx12GraphicResourceInfo* GetDefaultSceneDsInfo()const noexcept;
		private:
			uint GetHeapIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType);
		public:
			bool CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE type)const noexcept final;
		public:
			void CreateSwapChainBuffer(ID3D12Device* device,
				IDXGIFactory4* dxgiFactory,
				ID3D12CommandQueue* commandQueue,
				Microsoft::WRL::ComPtr<IDXGISwapChain>& swapChain,
				const uint width,
				const uint height,
				bool m4xMsaaState,
				uint m4xMsaaQuality);
			JUserPtr<JGraphicResourceInfo> CreateSceneDepthStencilResource(JGraphicDevice* device,
				const uint viewWidth,
				const uint viewHeight)final;
			//for display scene depth stencil map
			//for debug ui or debug object
			JUserPtr<JGraphicResourceInfo> CreateDebugDepthStencilResource(JGraphicDevice* device,
				const uint viewWidth,
				const uint viewHeight)final;
			JUserPtr<JGraphicResourceInfo> CreateLayerDepthDebugResource(JGraphicDevice* device,
				const uint viewWidth,
				const uint viewHeight)final;
			void CreateHZBOcclusionResource(JGraphicDevice* device,
				const uint occWidth,
				const uint occHeight,
				_Out_ JUserPtr<JGraphicResourceInfo>& outOccDsInfo,
				_Out_ JUserPtr<JGraphicResourceInfo>& outOccMipMapInfo)final;
			JUserPtr<JGraphicResourceInfo> CreateOcclusionResourceDebug(JGraphicDevice* device,
				const uint occWidth,
				const uint occHeight,
				const bool isHzb)final;
			JUserPtr<JGraphicResourceInfo> Create2DTexture(JGraphicDevice* device,
				const std::wstring& path,
				const std::wstring& oriFormat)final;
			JUserPtr<JGraphicResourceInfo> CreateCubeMap(JGraphicDevice* device,
				const std::wstring& path,
				const std::wstring& oriFormat)final;
			JUserPtr<JGraphicResourceInfo> CreateRenderTargetTexture(JGraphicDevice* device,
				const uint width,
				const uint height)final;
			JUserPtr<JGraphicResourceInfo> CreateShadowMapTexture(JGraphicDevice* device,
				const uint width,
				const uint height)final;
			JUserPtr<JGraphicResourceInfo> CreateShadowMapArrayTexture(JGraphicDevice* device,
				const uint width,
				const uint height,
				const uint count)final;
			JUserPtr<JGraphicResourceInfo> CreateShadowMapCubeTexture(JGraphicDevice* device,
				const uint width,
				const uint height)final;
			JUserPtr<JGraphicResourceInfo> CreateVertexBuffer(JGraphicDevice* device, const std::vector<Core::JStaticMeshVertex>& vertex) final;
			JUserPtr<JGraphicResourceInfo> CreateVertexBuffer(JGraphicDevice* device, const std::vector<Core::JSkinnedMeshVertex>& vertex) final;
			JUserPtr<JGraphicResourceInfo> CreateIndexBuffer(JGraphicDevice* device, const std::vector<uint32>& index) final;
			JUserPtr<JGraphicResourceInfo> CreateIndexBuffer(JGraphicDevice* device, const std::vector<uint16>& index) final;
			bool DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info)final;
		private:
			JOwnerPtr<JDx12GraphicResourceInfo> CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource);
			JUserPtr<JGraphicResourceInfo> CreateBuffer(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, JGraphicDevice* device, const void* bufferData, size_t bufferByteSize);
		private:
			BindViewPtr GetResourceBindViewPtr(const J_GRAPHIC_RESOURCE_TYPE rType);
			void SetViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const int nextViewIndex);
			void BindSwapChain(ID3D12Device* device, const uint resourceIndex);
			void BindMainDepthStencil(ID3D12Device* device, const uint resourceIndex);
			void BindDebugDepthStencil(ID3D12Device* device, const uint resourceIndex);
			void BindLayerDetphMapDebug(ID3D12Device* device, const uint resourceIndex);
			void BindOcclusionDepthMap(ID3D12Device* device, const uint resourceIndex);
			void BindHZBOcclusionDepthMipMap(ID3D12Device* device, const uint resourceIndex);
			void BindHZBOcclusionDebug(ID3D12Device* device, const uint resourceIndex);
			void BindHdOcclusionDebug(ID3D12Device* device, const uint resourceIndex);
			void Bind2DTexture(ID3D12Device* device, const uint resourceIndex);
			void BindCubeMap(ID3D12Device* device, const uint resourceIndex);
			void BindRenderTarget(ID3D12Device* device, const uint resourceIndex);
			void BindShadowMap(ID3D12Device* device, const uint resourceIndex);
			void BindShadowMapArray(ID3D12Device* device, const uint resourceIndex);
			void BindShadowMapCube(ID3D12Device* device, const uint resourceIndex);
		private:
			void BuildRtvDescriptorHeaps(ID3D12Device* device);
			void BuildDsvDescriptorHeaps(ID3D12Device* device);
			void BuildSrvDescriptorHeaps(ID3D12Device* device);
		public:
			void ResizeWindow(const JGraphicBaseDataSet& base, JGraphicDevice* device)final;
		public:
			void StoreTexture(JGraphicDevice* device, const J_GRAPHIC_RESOURCE_TYPE rType, const int index, const std::wstring& path)final;
		public:
			static void RegisterTypeData();
		public:
			JDx12GraphicResourceManager();
			~JDx12GraphicResourceManager();
		};
	}
}