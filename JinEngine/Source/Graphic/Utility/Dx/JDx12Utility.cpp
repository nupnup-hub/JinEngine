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


#include"JDx12Utility.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include<fstream>
#include<algorithm>
#include<d3dcompiler.h>

namespace JinEngine
{ 
    Microsoft::WRL::ComPtr<ID3DBlob> JDx12Utility::LoadBinary(const std::wstring& filename)
    {
        std::ifstream fin(filename, std::ios::binary);

        fin.seekg(0, std::ios_base::end);
        std::ifstream::pos_type size = (int)fin.tellg();
        fin.seekg(0, std::ios_base::beg);

        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        ThrowIfFailedHr(D3DCreateBlob(size, blob.GetAddressOf()));

        fin.read((char*)blob->GetBufferPointer(), size);
        fin.close();

        return blob;
    }   
     void JDx12Utility::ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
    {
        CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, stateBefore, stateAfter);
        commandList->ResourceBarrier(1, &rsBarrier);
    }
    void JDx12Utility::ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBeforeT, D3D12_RESOURCE_STATES stateBeforeF, const bool condition, D3D12_RESOURCE_STATES stateAfter)
    {
        if (condition)
            ResourceTransition(commandList, pResource, stateBeforeT, stateAfter);
        else
            ResourceTransition(commandList, pResource, stateBeforeF, stateAfter);
    }
    void JDx12Utility::ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfterT, D3D12_RESOURCE_STATES stateAfterF, const bool condition)
    {
        if (condition)
            ResourceTransition(commandList, pResource, stateBefore, stateAfterT);
        else
            ResourceTransition(commandList, pResource, stateBefore, stateAfterF);
    }
}