#pragma once  
#include<vector>
#include<wrl.h> 
#include<d3d12.h>
#include"JExtraPsoType.h"
#include"../../../Utility/JVector.h"

namespace JinEngine
{  
	struct JGraphicShaderData
	{
	public:
		Microsoft::WRL::ComPtr<ID3DBlob> vs = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> hs = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> ds = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> gs = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> ps = nullptr; 
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	public:
		//Main pso
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;
		//Extra pso 
		//Material이외에 오브젝트에서 Pso변경이 필요할시 사용
		//ex) gameobject select시 outline 
		Microsoft::WRL::ComPtr<ID3D12PipelineState> extraPso[(int)J_GRAPHIC_EXTRA_PSO_TYPE::COUNT];
	};

	struct JComputeShaderData
	{
	public:
		struct DispatchInfo
		{
		public:
			JVector3<uint> groupDim;
			JVector3<uint> threadDim; 
			uint taskOriCount;			
		public:
			uint GetTotalThreadCount()const noexcept;
		};
	public: 
		Microsoft::WRL::ComPtr<ID3DBlob> cs = nullptr;							//Variable
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;				//Constants 
		ID3D12RootSignature* RootSignature = nullptr;	//Constants
	public:
		DispatchInfo dispatchInfo;													//Variable
	};
}