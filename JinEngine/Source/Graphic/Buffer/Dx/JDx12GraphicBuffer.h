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
		template<typename T>
		class JDx12GraphicBuffer final : public JDxGraphicBufferInterface
		{
		private:
			const std::wstring name;
		private:
			JDx12GraphicResourceHolder holder;
			BYTE* mappedData = nullptr;
			uint elementcount = 0;
			uint elementByteSize = 0;
			const J_GRAPHIC_BUFFER_TYPE type;
		public:
			bool canMapped = false;
		public:
			JDx12GraphicBuffer(const std::wstring& name, const J_GRAPHIC_BUFFER_TYPE type)
				:name(name), type(type)
			{ }
			JDx12GraphicBuffer(const JDx12GraphicBuffer& rhs) = delete;
			JDx12GraphicBuffer& operator=(const JDx12GraphicBuffer& rhs) = delete;
			~JDx12GraphicBuffer()
			{
				Clear();
			}
		public:
			void Build(JGraphicDevice* device, const uint newElementcount) final
			{
				if (!IsSameDevice(device))
					return;

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
					CD3DX12_HEAP_PROPERTIES uploadProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
					CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);

					ThrowIfFailedHr(d3d12Device->CreateCommittedResource(
						&uploadProperty,
						D3D12_HEAP_FLAG_NONE,
						&uploadDesc,
						D3D12_RESOURCE_STATE_COMMON,
						nullptr,
						IID_PPV_ARGS(&buffer)));
					canMapped = true; 
				}
				else if (type == J_GRAPHIC_BUFFER_TYPE::UNORDERED_ACCEESS)
				{
					CD3DX12_HEAP_PROPERTIES uploadProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount,
						D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

					ThrowIfFailedHr(d3d12Device->CreateCommittedResource(
						&uploadProperty,
						D3D12_HEAP_FLAG_NONE,
						&uploadDesc,
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
				else
				{ 
					//upload
					CD3DX12_HEAP_PROPERTIES uploadProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
					CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);
					ThrowIfFailedHr(d3d12Device->CreateCommittedResource(
						&uploadProperty,
						D3D12_HEAP_FLAG_NONE,
						&uploadDesc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(&buffer)));
					canMapped = true;
				}
				buffer->SetName(name.c_str());
				holder.Swap(std::move(buffer));
				if (canMapped)
					mappedData = holder.Map();
				SetValid(true);
			}
			void Clear()noexcept final
			{
				if (canMapped && IsValid())
					holder.UnMap();
				mappedData = nullptr;
				holder.Clear();
				SetValid(false);
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
			T GetData(const uint index)
			{
				return (T)mappedData[index * elementByteSize];
			}
			ID3D12Resource* GetResource()const noexcept
			{
				return holder.GetResource();
			}
		public:
			void SetGraphicCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
			{
				commandList->SetGraphicsRootConstantBufferView(rootIndex, holder.GetResource()->GetGPUVirtualAddress() + addressOffset * elementByteSize);
			}
			void SetComputeCBBufferView(ID3D12GraphicsCommandList* commandList, const uint rootIndex, const uint addressOffset)
			{
				commandList->SetComputeRootConstantBufferView(rootIndex, holder.GetResource()->GetGPUVirtualAddress() + addressOffset * elementByteSize);
			}
			void SetGraphicsRootShaderResourceView(ID3D12GraphicsCommandList* commandList, const uint rootIndex)
			{
				commandList->SetGraphicsRootShaderResourceView(rootIndex, holder.GetResource()->GetGPUVirtualAddress());
			}
		public:
			bool CanMappedCpuPointer()const noexcept
			{
				return true;
			}
		};
	}
}