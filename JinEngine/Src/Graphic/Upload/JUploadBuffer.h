#pragma once 
#include"../../Core/JDataType.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Utility/JD3DUtility.h"
#include"../../../Lib/DirectX/d3dx12.h"
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_UPLOAD_BUFFER_TYPE
		{
			CONSTANT,
			COMMON,
			UNORDERED_ACCEESS,
			READ_BACK,
		};

		class JUploadBufferBase
		{
		public:
			virtual ~JUploadBufferBase() = default;
		public:
			virtual void Build(ID3D12Device* device, const uint elementcount) = 0;
			virtual void Clear()noexcept = 0;
			virtual uint ElementCount()const noexcept = 0;
		};

		template<typename T>
		class JUploadBuffer : public JUploadBufferBase
		{
		private:
			Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
			BYTE* mappedData = nullptr;
			uint elementcount;
			uint elementByteSize = 0;
			const J_UPLOAD_BUFFER_TYPE type;
		public:
			JUploadBuffer(const J_UPLOAD_BUFFER_TYPE type)
				:type(type)
			{ }
			JUploadBuffer(const JUploadBuffer& rhs) = delete;
			JUploadBuffer& operator=(const JUploadBuffer& rhs) = delete;
			~JUploadBuffer()
			{
				Clear();
			}
		public:
			void Build(ID3D12Device* device, const uint elementcount) final
			{
				JUploadBuffer::elementcount = elementcount;

				elementByteSize = sizeof(T);

				// Constant buffer elements need to be multiples of 256 bytes.
				// This is because the hardware can only view constant data 
				// at m*256 byte offsets and of n*256 byte lengths. 
				// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
				// UINT64 OffsetInBytes; // multiple of 256
				// UINT   SizeInBytes;   // multiple of 256
				// } D3D12_CONSTANT_BUFFER_VIEW_DESC;
				if (type == J_UPLOAD_BUFFER_TYPE::CONSTANT)
					elementByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(T));

				if (type == J_UPLOAD_BUFFER_TYPE::READ_BACK)
				{
					CD3DX12_HEAP_PROPERTIES uploadProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
					CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);

					ThrowIfFailedHr(device->CreateCommittedResource(
						&uploadProperty,
						D3D12_HEAP_FLAG_NONE,
						&uploadDesc,
						D3D12_RESOURCE_STATE_COPY_DEST,
						nullptr,
						IID_PPV_ARGS(&uploadBuffer)));
				}
				else if (type == J_UPLOAD_BUFFER_TYPE::UNORDERED_ACCEESS)
				{
					CD3DX12_HEAP_PROPERTIES uploadProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount,
						D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS); 
					 
					ThrowIfFailedHr(device->CreateCommittedResource(
						&uploadProperty,
						D3D12_HEAP_FLAG_NONE,
						&uploadDesc,
						D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
						nullptr,
						IID_PPV_ARGS(&uploadBuffer)));
				}
				else
				{
					CD3DX12_HEAP_PROPERTIES uploadProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
					CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementcount);
					ThrowIfFailedHr(device->CreateCommittedResource(
						&uploadProperty,
						D3D12_HEAP_FLAG_NONE,
						&uploadDesc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(&uploadBuffer)));
					ThrowIfFailedHr(uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));
				}
			}
			void Clear()noexcept final
			{
				if (uploadBuffer != nullptr && (type == J_UPLOAD_BUFFER_TYPE::COMMON || type == J_UPLOAD_BUFFER_TYPE::CONSTANT))
					uploadBuffer->Unmap(0, nullptr);
				mappedData = nullptr;
				uploadBuffer.Reset();
			}
		public:
			uint ElementCount()const noexcept final
			{
				return elementcount;
			}
		public:
			T Data(const uint index)
			{
				return (T)mappedData[index * elementByteSize];
			}
			ID3D12Resource* Resource()const
			{
				return uploadBuffer.Get();
			}
			void CopyData(const int elementIndex, const T& data)
			{
				memcpy(&mappedData[elementIndex * elementByteSize], &data, sizeof(T));
			}
		};
	}
}