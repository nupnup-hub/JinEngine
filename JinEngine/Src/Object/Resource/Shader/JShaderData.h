#pragma once  
#include<vector>
#include<wrl.h> 
#include<d3d12.h>
#include"../../../Utility/JVector.h"

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
		struct DispatchInfo
		{
		public:
			JVector3<uint> groupDim;
			JVector3<uint> threadDim;											 
			uint threadCount;	
			uint taskOriCount;
		};
	public: 
		Microsoft::WRL::ComPtr<ID3DBlob> Cs = nullptr;							//Variable
		Microsoft::WRL::ComPtr<ID3D12PipelineState> Pso = nullptr;				//Constant 
		ID3D12RootSignature* RootSignature = nullptr;	//Constant
	public:
		DispatchInfo dispatchInfo;													//Variable
	};
}