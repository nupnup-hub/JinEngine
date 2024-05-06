/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JLoadTextureFromFile.h"
#define STB_IMAGE_IMPLEMENTATION 
#include"../../../Core/Exception/JExceptionMacro.h"
#include"../../../Core/Utility/JCommonUtility.h"
#include"../../../../ThirdParty/stb-master/stb_image.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"  
#include"../../../../ThirdParty/Tif/tiffio.h"

namespace JinEngine
{
    namespace Private
    {
        BYTE* ReadTif(const std::wstring& path, int* image_width, int* image_height, const int channelCount)
        {
            *image_width = *image_height = 0;
            BYTE* result = nullptr;
            TIFF* tif = TIFFOpen(JCUtil::WstrToU8Str(path).c_str(), "r");
            if (tif) 
            {
                TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, image_width);
                TIFFGetField(tif, TIFFTAG_IMAGELENGTH, image_height);
         
                int w = *image_width;
                int h = *image_height;
                size_t npixels = w * h;
                uint32* raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32));
                 
                if (raster != NULL) 
                {
                    if (TIFFReadRGBAImage(tif, *image_width, *image_height, raster, 0))
                    {
                        result = (BYTE*)malloc(sizeof(BYTE) * npixels * channelCount);
                        int offset = 0;
                        for (int i = 0; i < h; ++i)
                        {
                            BYTE* p = &result[i * w * channelCount];
                            for (int j = 0; j < w; j++)
                            {
                                int rgba = raster[offset++];
                                p[j * channelCount] = TIFFGetR(rgba);
                                p[j * channelCount + 1] = TIFFGetG(rgba);
                                p[j * channelCount + 2] = TIFFGetB(rgba);
                                p[j * channelCount + 3] = TIFFGetA(rgba);
                            }
                        } 
                    }
                    _TIFFfree(raster);
                }
                TIFFClose(tif); 
            } 
            return result;
        }
    }
    bool LoadTextureFromFile(const std::wstring& path,
        const std::wstring& format,
        ID3D12Device* device,
        D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle,
        ComPtr<ID3D12Resource>& out_tex_resource,
        ComPtr<ID3D12Resource>& uploadBuffer,
        const int maxSize)
    {
        // Load from disk into a raw RGBA buffer
        int image_width = 0;
        int image_height = 0;
        BYTE* image_data = nullptr;
        if (format == L".tif" || format == L".tiff")
            image_data = Private::ReadTif(path, &image_width, &image_height, 4);
        else
            image_data = stbi_load(JCUtil::WstrToU8Str(path).c_str(), &image_width, &image_height, NULL, 4);
        if (image_data == NULL)
            return false;
 
        // Create texture resource
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Alignment = 0;
        desc.Width = image_width;
        desc.Height = image_height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ID3D12Resource* pTexture = NULL;
        device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&pTexture));

        // Create a temporary upload resource to move the data in
        UINT uploadPitch = (image_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
        UINT uploadSize = image_height * uploadPitch;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = uploadSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        props.Type = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        //uploadBuffer = NULL;
        HRESULT hr = device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(uploadBuffer.GetAddressOf()));
        assert(SUCCEEDED(hr));

        // Write pixels into the upload resource
        void* mapped = NULL;
        D3D12_RANGE length = { 0, uploadSize };
        hr = uploadBuffer->Map(0, &length, &mapped);
        assert(SUCCEEDED(hr));

        for (int y = 0; y < image_height; y++)
            memcpy((void*)((uintptr_t)mapped + y * uploadPitch), image_data + y * image_width * 4, image_width * 4);
        uploadBuffer->Unmap(0, &length);

        // Copy the upload resource content into the real resource
        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = uploadBuffer.Get();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srcLocation.PlacedFootprint.Footprint.Width = image_width;
        srcLocation.PlacedFootprint.Footprint.Height = image_height;
        srcLocation.PlacedFootprint.Footprint.Depth = 1;
        srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = pTexture;
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0;

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = pTexture;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        // Create a temporary command queue to do the copy with
        ID3D12Fence* fence = NULL;
        hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        assert(SUCCEEDED(hr));

        HANDLE event = CreateEvent(0, 0, 0, 0);
        assert(event != NULL);

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 1;

        ID3D12CommandQueue* cmdQueue = NULL;
        hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
        assert(SUCCEEDED(hr));

        ID3D12CommandAllocator* cmdAlloc = NULL;
        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
        assert(SUCCEEDED(hr));

        ID3D12GraphicsCommandList* cmdList = NULL;
        hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, NULL, IID_PPV_ARGS(&cmdList));
        assert(SUCCEEDED(hr));

        cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
        cmdList->ResourceBarrier(1, &barrier);

        hr = cmdList->Close();
        assert(SUCCEEDED(hr));

        // Execute the copy
        cmdQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
        hr = cmdQueue->Signal(fence, 1);
        assert(SUCCEEDED(hr));

        // Wait for everything to complete
        fence->SetEventOnCompletion(1, event);
        WaitForSingleObject(event, INFINITE);

        // Tear down our temporary command queue and release the upload resource
        cmdList->Release();
        cmdAlloc->Release();
        cmdQueue->Release();
        CloseHandle(event);
        fence->Release();
        //uploadBuffer->Release();

        // Return results
        out_tex_resource = pTexture; 
        stbi_image_free(image_data);

        return true;
    } 
}
