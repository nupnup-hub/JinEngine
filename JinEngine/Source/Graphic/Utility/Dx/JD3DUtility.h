#pragma once  
#include <wrl.h>  
#include <string>
#include <memory>    
#include"../../../Core/JCoreEssential.h" 
#include"../../../Object/Component/RenderItem/JRenderItemPrimitive.h"
#include "../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"   

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

namespace JinEngine
{  
	class JD3DUtility
	{
	public: 	 
		static uint CalcConstantBufferByteSize(const uint byteSize)noexcept;
		static D3D12_PRIMITIVE_TOPOLOGY ConvertRenderPrimitive(const J_RENDER_PRIMITIVE renderPrimitive)noexcept;
		static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename); 
	public:
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBeforeT, D3D12_RESOURCE_STATES stateBeforeF, const bool condition, D3D12_RESOURCE_STATES stateAfter);
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfterT, D3D12_RESOURCE_STATES stateAfterF, const bool condition);
		template<size_t ...Is>
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource** pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, std::index_sequence<Is...>)
		{
			(ResourceTransition(commandList, pResource[Is], stateBefore, stateAfter), ...);
		}
	public:
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

			JD3DUtility::ResourceTransition(cmdList, resource, beforeState, D3D12_RESOURCE_STATE_COPY_DEST);
			UpdateSubresources<1>(cmdList, resource, uploadBuffer, 0, 0, subResourceCount, &clearData);
			JD3DUtility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, afterState);
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

			JD3DUtility::ResourceTransition(cmdList, resource, beforeState, D3D12_RESOURCE_STATE_COPY_DEST);
			UpdateSubresources<1>(cmdList, resource, uploadBuffer, 0, 0, subResourceCount, &clearData);
			JD3DUtility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, afterState);
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

			JD3DUtility::ResourceTransition(cmdList, resource, beforeState, D3D12_RESOURCE_STATE_COPY_DEST);
			UpdateSubresources<1>(cmdList, resource, uploadBuffer, 0, 0, 1, &clearData);
			JD3DUtility::ResourceTransition(cmdList, resource, D3D12_RESOURCE_STATE_COPY_DEST, afterState);
		}
	};
}