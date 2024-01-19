#pragma once
#include"../../../Core/Exception/JExceptionMacro.h"
#include<d3dx12.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12RootSignature
		{  
		public:
			static void Create(ID3D12Device* device,
				const CD3DX12_ROOT_SIGNATURE_DESC* desc,
				const std::wstring& name,
				_Out_ ID3D12RootSignature** outRoot)
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
		};
	}
}