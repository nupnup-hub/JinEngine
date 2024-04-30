#pragma once 
#include"JDx12GraphicResourceHolder.h"
#include"../JGraphicResourceType.h"
#include"../../DataSet/JGraphicDataSet.h"
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../../Core/Interface/JValidInterface.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include"../../../../ThirdParty/DirectX/DDSTextureLoader11.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/ScreenGrab.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/WICTextureLoader.h" 
#include"../../../../ThirdParty/DirectX/TK/Inc/ResourceUploadBatch.h" 
#include"../../../../ThirdParty/Tif/tiffio.h"
#include<wrl/client.h>  
#include<Windows.h>
#include<dxgi1_4.h>  
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JDx12GraphicDevice;
		class JDx12GraphicResourceInfo;
		class JDx12GraphicResourceManager;
		struct JDx12SwapChainCreationData;

		using CommonBinDesc = JGraphicResourceBindDesc;
		class JDeviceData
		{
		public: 
			const JGraphicOption& option;
			JDx12GraphicDevice* dxDevice = nullptr;
			ID3D12Device* device = nullptr;
			ID3D12CommandQueue* commandQueue = nullptr;
			ID3D12GraphicsCommandList* commandList = nullptr;
			uint m4xMsaaQuality = 0;
			bool m4xMsaaState = false;
			bool startCommandThisCreation = false;
		public:
			JDeviceData(JGraphicDevice* device, const JGraphicOption& option);
		};
		struct JBindDesc : public Core::JValidInterface
		{
		public:
			using GetNextViewIndex = uint (*)(JDx12GraphicResourceManager*, J_GRAPHIC_RESOURCE_TYPE, J_GRAPHIC_BIND_TYPE);
			using AddViewIndex = void(*)(JDx12GraphicResourceManager*, J_GRAPHIC_RESOURCE_TYPE, J_GRAPHIC_BIND_TYPE);
		public:
			JDx12GraphicResourceManager* gm = nullptr;
			ID3D12Device* device = nullptr;
			GetNextViewIndex getNextViewIndex = nullptr;
			AddViewIndex addViewIndex = nullptr;
			JDx12GraphicResourceInfo* info = nullptr;
		public:
			CommonBinDesc cDesc; 
		public:
			JBindDesc(JDx12GraphicResourceManager* gm,
				ID3D12Device* device,
				GetNextViewIndex getNextViewIndex,
				AddViewIndex addViewIndex ,
				JDx12GraphicResourceInfo* info,
				const JGraphicResourceCreationDesc& cDesc);
			JBindDesc(JDx12GraphicResourceManager* gm,
				ID3D12Device* device,
				GetNextViewIndex getNextViewIndex,
				AddViewIndex addViewIndex,
				JDx12GraphicResourceInfo* info,
				const CommonBinDesc& cDesc = CommonBinDesc());
		};
		struct JBindDetailDesc
		{
		public:
			const JBindDesc& resourceBindDesc;
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
			JBindDetailDesc(const JBindDesc& resourceBindDesc);
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
 
		class JDx12GraphicResourceCreation
		{
		public:
			static Microsoft::WRL::ComPtr<ID3D12Resource> CreateOcclusionQueryResult(ID3D12Device* device, const size_t capa);
			static void CreateSwapChainBuffer(HWND windowHandle, JDx12SwapChainCreationData& creationData, const JGraphicOption& option);
			static void CreateUploadBuffer(ID3D12Device* device,
				ID3D12Resource* resource,
				ID3D12Resource** uploadBuffer,
				const uint subResourceCount = 1);
			static JDx12GraphicResourceHolderDesc CreateBufferAndUploadCpuBuffer(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
			static JDx12GraphicResourceHolderDesc Create(const JDeviceData& data, const JGraphicResourceCreationDesc& creationDesc, const J_GRAPHIC_RESOURCE_TYPE type);
			static JDx12GraphicResourceHolderDesc Create(const JDeviceData& data, ID3D12Resource* dxInfoResource, const J_GRAPHIC_RESOURCE_OPTION_TYPE type);
			static JDx12GraphicResourceHolderDesc Create(const JDeviceData& data,
				const JGraphicResourceCreationDesc& creationDesc,
				const J_GRAPHIC_RESOURCE_TYPE type, 
				Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer);
			static bool Load(const JDeviceData& data, 
				const JGraphicResourceCreationDesc& creationDesc,
				const J_GRAPHIC_RESOURCE_TYPE type,
				Microsoft::WRL::ComPtr<ID3D12Resource>& newResource,
				Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
				std::unique_ptr<DirectX::ResourceUploadBatch>& uploadBatch,
				const bool allowWIC = true); 
			static bool Bind(const JBindDesc& bDesc); 
			static bool Bind(const JBindDetailDesc& opDesc, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
		public:
			static bool CanBind(const J_GRAPHIC_RESOURCE_TYPE type);
		public:
			static void UploadData(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* resource,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const void* data,
				size_t byteSize,
				const uint subResourceCount);
			template<typename T>
			static void UploadData(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* resource,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T* data,
				const uint elementcount,
				const uint elementByteSize,
				const uint subResourceCount)
			{
				D3D12_SUBRESOURCE_DATA clearData = {};
				clearData.pData = data;
				clearData.RowPitch = elementcount * elementByteSize;
				clearData.SlicePitch = clearData.RowPitch;

				JDx12Utility::ResourceTransition(cmdList, resource, beforeState, D3D12_RESOURCE_STATE_COPY_DEST);
				UpdateSubresources<1>(cmdList, resource, uploadBuffer, 0, 0, subResourceCount, &clearData);
				JDx12Utility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, afterState);
			}
			template<typename T>
			static void UploadData(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* resource,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T* data,
				const uint width,
				const uint height,
				const uint elementByteSize,
				const uint subResourceCount)
			{
				D3D12_SUBRESOURCE_DATA clearData = {};
				clearData.pData = data;
				clearData.RowPitch = width * elementByteSize;
				clearData.SlicePitch = height * clearData.RowPitch;

				JDx12Utility::ResourceTransition(cmdList, resource, beforeState, D3D12_RESOURCE_STATE_COPY_DEST);
				UpdateSubresources<1>(cmdList, resource, uploadBuffer, 0, 0, subResourceCount, &clearData);
				JDx12Utility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, afterState);
			}
			template<typename T>
			static void UploadClearData(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* resource,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T clearValue,
				const uint elementcount,
				const uint elementByteSize)
			{
				std::vector<T> clearDataVec(elementcount, clearValue);
				D3D12_SUBRESOURCE_DATA clearData = {};
				clearData.pData = &clearDataVec[0];
				clearData.RowPitch = elementcount * elementByteSize;
				clearData.SlicePitch = clearData.RowPitch;

				JDx12Utility::ResourceTransition(cmdList, resource, beforeState, D3D12_RESOURCE_STATE_COPY_DEST);
				UpdateSubresources<1>(cmdList, resource, uploadBuffer, 0, 0, 1, &clearData);
				JDx12Utility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, afterState);
			}
		};
	}
}