#pragma once
#include"JDxGraphicBufferInterface.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceHolder.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JD3DUtility.h"   
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicBufferInterface : public JDxGraphicBufferInterface
		{
		public:
			virtual ID3D12Resource* GetResource()const noexcept = 0;
		public:
			virtual void SetGraphicCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset) = 0;
			virtual void SetComputeCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset) = 0;
			virtual void SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex) = 0;
		};
		template<typename T>
		class JDx12GraphicBuffer final : public JDx12GraphicBufferInterface
		{
		private:
			std::wstring name;
		private:
			JDx12GraphicResourceHolder holder;
			BYTE* mappedData = nullptr;
			uint elementcount = 0;
			uint elementByteSize = 0;
			J_GRAPHIC_BUFFER_TYPE type;
		public:
			bool canMapped = false;
		public:
			JDx12GraphicBuffer()
				:name(L""), type(J_GRAPHIC_BUFFER_TYPE::COMMON)
			{}
			JDx12GraphicBuffer(const std::wstring& name, const J_GRAPHIC_BUFFER_TYPE type)
				:name(name), type(type)
			{}
			JDx12GraphicBuffer(const JDx12GraphicBuffer& rhs) = delete;
			JDx12GraphicBuffer& operator=(const JDx12GraphicBuffer& rhs) = delete;
			JDx12GraphicBuffer(JDx12GraphicBuffer&& rhs) = default;
			JDx12GraphicBuffer& operator=(JDx12GraphicBuffer&& rhs) = default;
			~JDx12GraphicBuffer()
			{
				ClearOwnResource();
			}
		public:
			void Build(JGraphicDevice* device, const uint newElementcount) final
			{
				if (!IsSameDevice(device))
					return;

				if(IsValid())
					Clear();
				elementcount = newElementcount;
				elementByteSize = sizeof(T);

				// Constants buffer elements need to be multiples of 256 bytes.
				// This is because the hardware can only view constant data 
				// at m*256 byte offsets and of n*256 byte lengths. 
				// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
				// UINT64 OffsetInBytes; // multiple of 256
				// UINT   SizeInBytes;   // multiple of 256
				// } D3D12_CONSTANT_BUFFER_VIEW_DESC;
				if (type == J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT)
					elementByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(T));

				ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
				Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

				if (type == J_GRAPHIC_BUFFER_TYPE::READ_BACK)
				{
					CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
					CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);

					ThrowIfFailedHr(d3d12Device->CreateCommittedResource(
						&heapProperty,
						D3D12_HEAP_FLAG_NONE,
						&desc,
						D3D12_RESOURCE_STATE_COMMON,
						nullptr,
						IID_PPV_ARGS(&buffer)));
					canMapped = true;
				}
				else if (type == J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS)
				{
					CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount,
						D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

					ThrowIfFailedHr(d3d12Device->CreateCommittedResource(
						&heapProperty,
						D3D12_HEAP_FLAG_NONE,
						&desc,
						D3D12_RESOURCE_STATE_COMMON,
						//초기상태설정시 D3D12_RESOURCE_STATE_UNORDERED_ACCESS은 무시되며 자동으로 D3D12_RESOURCE_STATE_COMMON으로
						//내부에서 설정되며 리소스 사용시 D3D12_RESOURCE_STATE_UNORDERED_ACCESS로 수동변환이 필요하다
						//D3D12_RESOURCE_STATE_UNORDERED_ACCESS
						nullptr,
						IID_PPV_ARGS(&buffer)));
				}
				else if (type == J_GRAPHIC_BUFFER_TYPE::OCC_PREDICT)
				{
					CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
					auto queryResultDesc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);

					ThrowIfFailedG(d3d12Device->CreateCommittedResource(
						&heapProps,
						D3D12_HEAP_FLAG_NONE,
						&queryResultDesc,
						D3D12_RESOURCE_STATE_COMMON,
						//초기상태설정시 D3D12_RESOURCE_STATE_PREDICATION은 무시되며 자동으로 D3D12_RESOURCE_STATE_COMMON으로
						//내부에서 설정되며 리소스 사용시 D3D12_RESOURCE_STATE_PREDICATION로 수동변환이 필요하다
						//D3D12_RESOURCE_STATE_PREDICATION
						nullptr,
						IID_PPV_ARGS(&buffer)));
				}
				else if (type == J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER || type == J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT)
				{
					//upload
					CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
					CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);
					ThrowIfFailedHr(d3d12Device->CreateCommittedResource(
						&heapProperty,
						D3D12_HEAP_FLAG_NONE,
						&desc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(&buffer)));
					canMapped = true;
				}
				else if (type == J_GRAPHIC_BUFFER_TYPE::COMMON)
				{
					//common
					CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);

					ThrowIfFailedHr(d3d12Device->CreateCommittedResource(
						&heapProperty,
						D3D12_HEAP_FLAG_NONE,
						&desc,
						D3D12_RESOURCE_STATE_COMMON,
						nullptr,
						IID_PPV_ARGS(&buffer)));
				}
				else
					return;

				buffer->SetName(name.c_str());
				holder.Swap(std::move(buffer));
				if (canMapped)
					mappedData = holder.Map();
				SetValid(true);
			}
			void Clear()noexcept final
			{
				ClearOwnResource();
				SetValid(false);
			}
			void StuffValue(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T* data)
			{
				if (type != J_GRAPHIC_BUFFER_TYPE::READ_BACK || type != J_GRAPHIC_BUFFER_TYPE::COMMON)
					return;
				 
				JD3DUtility::UploadData(cmdList, holder.GetResource(), uploadBuffer, beforeState, afterState, data, elementcount, elementByteSize, 1);
			}
			void SutffClearValue(ID3D12GraphicsCommandList* cmdList,
				ID3D12Resource* uploadBuffer,
				const D3D12_RESOURCE_STATES beforeState,
				const D3D12_RESOURCE_STATES afterState,
				const T clearValue)
			{
				if (type != J_GRAPHIC_BUFFER_TYPE::READ_BACK || type != J_GRAPHIC_BUFFER_TYPE::COMMON)
					return;

				JD3DUtility::UploadClearData(cmdList, holder.GetResource(), uploadBuffer, beforeState, afterState, clearValue, elementcount, elementByteSize);
			}
		public:
			void CopyData(const int elementIndex, const void* data)final
			{
				memcpy(&mappedData[elementIndex * elementByteSize], static_cast<const T*>(data), sizeof(T));
			}
			void CopyData(const int elementIndex, const T& data)
			{
				memcpy(&mappedData[elementIndex * elementByteSize], &data, sizeof(T));
			}
		public:
			uint GetElementCount()const noexcept final
			{
				return elementcount;
			}
			BYTE* GetCpuBytePointer()const noexcept final
			{
				return mappedData;
			}
			T* GetCpuPointer()const noexcept
			{
				return static_cast<T*>(static_cast<void*>(mappedData));
			}
			J_GRAPHIC_BUFFER_TYPE GetBufferType()const noexcept final
			{
				return type;
			}
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final
			{
				return J_GRAPHIC_DEVICE_TYPE::DX12;
			}
			T GetData(const uint index)const noexcept
			{
				return (T)mappedData[index * elementByteSize];
			}
			ID3D12Resource* GetResource()const noexcept final
			{
				return holder.GetResource();
			}
		public:
			void SetGraphicCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)final
			{
				commandList->SetGraphicsRootConstantBufferView(rootIndex, holder.GetResource()->GetGPUVirtualAddress() + addressOffset * elementByteSize);
			}
			void SetComputeCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)final
			{
				commandList->SetComputeRootConstantBufferView(rootIndex, holder.GetResource()->GetGPUVirtualAddress() + addressOffset * elementByteSize);
			}
			void SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex)final
			{
				commandList->SetGraphicsRootShaderResourceView(rootIndex, holder.GetResource()->GetGPUVirtualAddress());
			}
		public:
			bool CanMappedCpuPointer()const noexcept
			{
				return true;
			}
		private:
			void ClearOwnResource()noexcept
			{
				if (canMapped && mappedData != nullptr)
					holder.UnMap();
				//if (canMapped && IsValid())
				//	holder.UnMap();
				mappedData = nullptr;
				holder.Clear();
			}
		};
	}
}