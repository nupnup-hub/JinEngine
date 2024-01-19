#pragma once
#include"../JGraphicResourceManager.h"  
#include"../../../Core/JCoreEssential.h"  
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Handle/JDataHandleStructure.h"
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
			using CommonBinDesc = JGraphicResourceBindDesc;
			struct BindDesc
			{
			public:
				ID3D12Device* device = nullptr;
				uint resourceIndex = 0;
			public:
				CommonBinDesc cDesc;
			public:
				bool useMipmap = false;
			public:
				BindDesc(ID3D12Device* device, const uint resourceIndex, const JGraphicResourceCreationDesc& cDesc, const bool useMipmap = false);
				BindDesc(ID3D12Device* device, const uint resourceIndex, const CommonBinDesc& cDesc = CommonBinDesc(), const bool useMipmap = false);
			};
			struct BindDetailDesc
			{
			public:
				const BindDesc& resourceBindDesc;
				J_GRAPHIC_RESOURCE_TYPE rType;
			public:
				//for graphic option
				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			public:
				ID3D12Resource* counterResource = nullptr;	//for uav
			public:
				bool allowBindResource[(uint)J_GRAPHIC_BIND_TYPE::COUNT];
			public:
				BindDetailDesc(const BindDesc& resourceBindDesc);
			public:
				//init common desc
				void InitDsv(const bool isArray);
				void InitRtv(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray);
				void InitSrv(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray);
				void InitUav(const D3D12_RESOURCE_DESC& resourceDesc, const bool isArray, ID3D12Resource* counterResource = nullptr);
			public:
				void SetDsv(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc);
				void SetRtv(const D3D12_RENDER_TARGET_VIEW_DESC& desc);
				void SetSrv(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
				void SetUav(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
			public:
				void ClearAllowTrigger();
			};
			struct ExtraOption
			{
			public:
				bool bindResourceManually = false;
			};
		private:
			using CreationProcessPtr = JUserPtr<JGraphicResourceInfo>(JDx12GraphicResourceManager::*)(const J_GRAPHIC_RESOURCE_TYPE, JGraphicDevice*, const JGraphicResourceCreationDesc&, const ExtraOption&);
			using BindViewPtr = void(JDx12GraphicResourceManager::*)(const BindDesc&);
			using BindOptionViewPtr = void(JDx12GraphicResourceManager::*)(const BindDetailDesc&);
			using MPBStructure = Core::JDataHandleStructure<MPBCapactiy(), MPBInfo, true>;
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
			MPBStructure mpb;
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
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetMPBCpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetMPBGpuDescriptorHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			ID3D12DescriptorHeap* GetDescriptorHeap(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			uint GetDescriptorSize(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			static DXGI_FORMAT GetBackBufferFormat()noexcept;
			static DXGI_FORMAT GetDepthStencilFormat()noexcept;
			static DirectX::XMVECTORF32 GetBlackColor()noexcept;
			static DirectX::XMVECTORF32 GetBackBufferClearColor()noexcept;
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
			ResourceHandle GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bType, int index)const noexcept final;
			ResourceHandle GetMPBResourceCpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			ResourceHandle GetMPBResourceGpuHandle(const Core::JDataHandle& handle, const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			ID3D12Resource* GetResource(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			ID3D12Resource* GetOptionResource(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, int index)const noexcept;
			JGraphicResourceInfo* GetInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept final;
			JDx12GraphicResourceInfo* GetDxInfo(const J_GRAPHIC_RESOURCE_TYPE rType, int index)const noexcept;
			JDx12GraphicResourceInfo* GetDefaultSceneDsInfo()const noexcept;
		private:
			uint GetHeapIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType);
		public:
			bool CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool CanCreateResource(const J_GRAPHIC_RESOURCE_TYPE rType, JGraphicDevice* device)noexcept;
			bool CanCreateOptionResource(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
		public:
			static void CreateUploadBuffer(ID3D12Device* device,
				ID3D12Resource* resource,
				ID3D12Resource** uploadBuffer,
				const uint subResourceCount = 1);
		public:
			void CreateSwapChainBuffer(ID3D12Device* device,
				IDXGIFactory4* dxgiFactory,
				ID3D12CommandQueue* commandQueue,
				Microsoft::WRL::ComPtr<IDXGISwapChain>& swapChain,
				const uint width,
				const uint height,
				bool m4xMsaaState,
				uint m4xMsaaQuality);
			JUserPtr<JGraphicResourceInfo> CreateResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType)final;
			bool CreateOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)final;
			bool DestroyGraphicTextureResource(JGraphicDevice* device, JGraphicResourceInfo* info)final;
			bool DestroyGraphicOption(JGraphicDevice* device, JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE optype)final;
		private: 
			JUserPtr<JGraphicResourceInfo> CommonCreationProcess(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType, const ExtraOption& extraOption = ExtraOption());
			JUserPtr<JGraphicResourceInfo> CreateOcclusionResourceDebug(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc);
			JUserPtr<JGraphicResourceInfo> CreateTexture2D(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc);
			JUserPtr<JGraphicResourceInfo> CreateCubeMap(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc);
			JUserPtr<JGraphicResourceInfo> CreateLightLinkedList(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc); 
			JUserPtr<JGraphicResourceInfo> CreateBuffer(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType);
			void CreateResourceInfo(const J_GRAPHIC_RESOURCE_TYPE rType, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource);
			bool CreateOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, JDx12GraphicResourceInfo* dxInfo, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource);
		private:
			BindViewPtr GetResourceBindViewPtr(const J_GRAPHIC_RESOURCE_TYPE rType);
			BindOptionViewPtr GetResourceBindOptionViewPtr(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
			void SetViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const int nextViewIndex);
			void SetOptionViewCount(JDx12GraphicResourceInfo* handlePtr, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const int nextViewIndex);
			void ClearDescViewCountUntil(JDx12GraphicResourceInfo* dxInfo);
			void ReBind(ID3D12Device* device, const J_GRAPHIC_RESOURCE_TYPE rType, const uint index, const int additionalArrayIndex = 0);
			void BindSwapChain(const BindDesc& bDesc);
			void BindMainDepthStencil(const BindDesc& bDesc);
			void BindDebugDepthStencil(const BindDesc& bDesc);
			void BindDebugMap(const BindDesc& bDesc);
			void BindOcclusionDepthMap(const BindDesc& bDesc);
			void BindHZBOcclusionDepthMipmap(const BindDesc& bDesc);
			void BindHZBOcclusionDebug(const BindDesc& bDesc);
			void BindHdOcclusionDebug(const BindDesc& bDesc);
			void Bind2DTexture(const BindDesc& bDesc);
			void BindCubeMap(const BindDesc& bDesc);
			void BindRenderTarget(const BindDesc& bDesc);
			void BindRenderTargetForLigthCull(const BindDesc& bDesc);
			void BindShadowMap(const BindDesc& bDesc);
			void BindShadowMapArray(const BindDesc& bDesc);
			void BindShadowMapCube(const BindDesc& bDesc);
			void BindLightLinkedList(const BindDesc& bDesc);
			void BindLightClusterOffsetBuffer(const BindDesc& bDesc);
			void BindSsaoMap(const BindDesc& bDesc);
			void BindSsaoIntermediateMap(const BindDesc& bDesc);
			void BindSsaoInterleaveMap(const BindDesc& bDesc);
			void BindSsaoDepthMap(const BindDesc& bDesc);
			void BindSsaoDepthInterleaveMap(const BindDesc& bDesc);
			void BindAlbedoMap(const BindDetailDesc& opDesc);
			void BindNormalMap(const BindDetailDesc& opDesc);
			void BindTangentMap(const BindDetailDesc& opDesc);
			void BindLightingPropertyMap(const BindDetailDesc& opDesc);
			void BindBlur(const BindDetailDesc& opDesc);
			void CommonBind(const BindDetailDesc& opDesc);
			void CommonOptionBind(const BindDetailDesc& opDesc, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
		public:
			bool ReAllocTypePerAllResource(JGraphicDevice* device, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE rType) final;
		public:
			bool CopyResource(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& from, const JUserPtr<JGraphicResourceInfo>& to) final;
		public:
			bool SettingMipmapBind(JGraphicDevice* device, const JUserPtr<JGraphicResourceInfo>& info, const bool isReadOnly, _Out_ std::vector<Core::JDataHandle>& handle)final;
			void DestroyMPB(JGraphicDevice* device, Core::JDataHandle& handle) final;
			uint GetMPBOffset(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
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