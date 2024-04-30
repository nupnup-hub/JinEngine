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