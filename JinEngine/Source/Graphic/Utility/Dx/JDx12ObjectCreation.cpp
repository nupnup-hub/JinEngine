#include"JDx12ObjectCreation.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include<d3dx12.h>
namespace JinEngine::Graphic
{
	void JDx12ObjectCreation::Create(ID3D12Device* device,
		const CD3DX12_ROOT_SIGNATURE_DESC* desc,
		const std::wstring& name,
		ID3D12RootSignature** outRoot)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(desc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailedHr(hr);
		ThrowIfFailedHr(device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(outRoot)));

		(*outRoot)->SetName(name.c_str());
	}
 
	void JDx12ComputePso::Create(ID3D12Device* device,
		JDx12ComputeShaderDataHolder* holder,
		ID3D12RootSignature* root,
		D3D12_PIPELINE_STATE_FLAGS flag)
	{  
		D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		newShaderPso.pRootSignature = root;
		newShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(holder->cs->GetBufferPointer()),
			holder->cs->GetBufferSize()
		};
		newShaderPso.Flags = flag;
		ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(holder->GetPsoAddress())));
	}

}