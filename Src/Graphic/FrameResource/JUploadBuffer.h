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
        template<typename T>
        class JUploadBuffer
        {
        private:
            Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
            BYTE* mappedData = nullptr;

            uint elementcount;
            uint elementByteSize = 0;
            bool isConstantBuffer = false;

        public:
            JUploadBuffer(ID3D12Device* device, const uint elementcount, const  bool isConstantBuffer) :
                isConstantBuffer(isConstantBuffer), elementcount(elementcount)
            {
                elementByteSize = sizeof(T);

                // Constant buffer elements need to be multiples of 256 bytes.
                // This is because the hardware can only view constant data 
                // at m*256 byte offsets and of n*256 byte lengths. 
                // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
                // UINT64 OffsetInBytes; // multiple of 256
                // UINT   SizeInBytes;   // multiple of 256
                // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
                if (isConstantBuffer)
                    elementByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(T));

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
                // We do not need to unmap until we are done with the resource.  However, we must not write to
                // the resource while it is in use by the GPU (so we must use synchronization techniques).
            }

            JUploadBuffer(const JUploadBuffer& rhs) = delete;
            JUploadBuffer& operator=(const JUploadBuffer& rhs) = delete;
            ~JUploadBuffer()
            {
                if (uploadBuffer != nullptr)
                    uploadBuffer->Unmap(0, nullptr);

                mappedData = nullptr;
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