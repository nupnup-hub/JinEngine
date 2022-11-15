#pragma once  
#include<vector>
#include<wrl.h> 
#include<d3d12.h>

namespace JinEngine
{  
	struct JGraphicShaderData
	{
	public:
		Microsoft::WRL::ComPtr<ID3DBlob> Vs = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> Hs = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> Ds = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> Gs = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> Ps = nullptr; 
		Microsoft::WRL::ComPtr<ID3D12PipelineState> Pso = nullptr;
		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	};

	struct JComputeShaderData
	{
	public:
		Microsoft::WRL::ComPtr<ID3DBlob> Cs = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> Pso = nullptr;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature = nullptr;
	};
}