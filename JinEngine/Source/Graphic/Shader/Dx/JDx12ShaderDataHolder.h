#pragma once  
#include"JDxShaderDataHolderInterface.h"
#include"../../../Core/Math/JVector.h" 
#include"../../../Object/Resource/Shader/JShaderEnum.h"
#include<vector>
#include<wrl.h> 
#include<d3d12.h> 
#include<dxcapi.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicShaderDataHolder final: public JGraphicShaderDataHolder
		{
		public:
			Microsoft::WRL::ComPtr<IDxcBlob> vs = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> hs = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> ds = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> gs = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> ps = nullptr;
			std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
		public:
			//Main pso
			Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;
			//Extra pso 
			//Material이외에 오브젝트에서 Pso변경이 필요할시 사용
			//ex) gameobject select시 outline 
			Microsoft::WRL::ComPtr<ID3D12PipelineState> extraPso[(int)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT];
		public: 
			JDx12GraphicShaderDataHolder();
			~JDx12GraphicShaderDataHolder();
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			ID3D12PipelineState* GetExtraPso(const J_GRAPHIC_SHADER_EXTRA_FUNCTION type)const noexcept;
		public:
			bool HasExtra(const J_GRAPHIC_SHADER_EXTRA_FUNCTION type)const noexcept final;
		public:
			void Clear()final;
		};

		class JDx12ComputeShaderDataHolder final : public JComputeShaderDataHolder
		{
		public:
			Microsoft::WRL::ComPtr<IDxcBlob> cs = nullptr;							//Variable
			Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = nullptr;				//Constants 
			//ID3D12RootSignature* RootSignature = nullptr;							//Variable
		public:
			JDx12ComputeShaderDataHolder();
			~JDx12ComputeShaderDataHolder();
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void Clear()final;
		};
	}
}