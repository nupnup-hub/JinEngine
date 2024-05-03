#pragma once  
#include"JDxShaderDataHolderInterface.h"
#include"../../../Core/Math/JVector.h"  
#include"../../../Core/Utility/JMacroUtility.h"
#include<vector>
#include<wrl.h> 
#include<d3d12.h> 
#include<dxcapi.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12RasterizeShaderDataHolderInterface 
		{
		public:
			virtual ID3D12PipelineState* GetPso(const uint index = 0)const noexcept = 0;
			virtual ID3D12PipelineState** GetPsoAddress(const uint index = 0) noexcept = 0;
		};

		template<uint psoVariation>
		class JDx12GraphicShaderDataHolder final: public JGrahicShaderDataHolder, 
			public JDx12RasterizeShaderDataHolderInterface
		{
		public:
			Microsoft::WRL::ComPtr<IDxcBlob> vs = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> hs = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> ds = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> gs = nullptr;
			Microsoft::WRL::ComPtr<IDxcBlob> ps = nullptr;
			std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
		public: 
			Microsoft::WRL::ComPtr<ID3D12PipelineState> pso[psoVariation];
		public: 
			JDx12GraphicShaderDataHolder()
			{

			}
			~JDx12GraphicShaderDataHolder()
			{
				ClearResource();
			}
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final
			{
				return J_GRAPHIC_DEVICE_TYPE::DX12;
			}
			J_SHADER_TYPE GetShaderType()const noexcept final
			{
				return J_SHADER_TYPE::GRAPHIC;
			}
			ResourceHandle GetShaderData(const uint index)const noexcept final
			{
				return GetPso(index);
			}
			ID3D12PipelineState* GetPso(const uint index = 0)const noexcept final
			{
				return pso[index].Get();
			}
			ID3D12PipelineState** GetPsoAddress(const uint index = 0)noexcept final
			{
				return pso[index].GetAddressOf();
			}
			uint GetVariationCount()const noexcept final
			{
				return psoVariation;
			}
		public:
			void Clear()final
			{
				ClearResource();
			}
		private:
			void ClearResource()
			{
				vs = nullptr;
				hs = nullptr;
				ds = nullptr;
				gs = nullptr;
				ps = nullptr;
				inputLayout.clear(); 
				for (uint i = 0; i < SIZE_OF_ARRAY(pso); ++i)
					pso[i] = nullptr;
			}
		};

		template<uint psoVariation>
		class JDx12ComputeShaderDataHolder final : public JComputeShaderDataHolder, 
			public JDx12RasterizeShaderDataHolderInterface
		{
		public:
			Microsoft::WRL::ComPtr<IDxcBlob> cs = nullptr;											//Variable
			Microsoft::WRL::ComPtr<ID3D12PipelineState> pso[psoVariation];				//Constants 
			//ID3D12RootSignature* RootSignature = nullptr;											//Variable
		public:
			JDx12ComputeShaderDataHolder()
			{

			}
			~JDx12ComputeShaderDataHolder()
			{
				ClearResource();
			}
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final
			{
				return J_GRAPHIC_DEVICE_TYPE::DX12;
			}
			J_SHADER_TYPE GetShaderType()const noexcept final
			{
				return J_SHADER_TYPE::COMPUTE;
			}
			ResourceHandle GetShaderData(const uint index)const noexcept final
			{
				return GetPso(index);
			}
			ID3D12PipelineState* GetPso(const uint index = 0)const noexcept final
			{
				return pso[index].Get();
			}
			ID3D12PipelineState** GetPsoAddress(const uint index = 0)noexcept final
			{
				return pso[index].GetAddressOf();
			}
			uint GetVariationCount()const noexcept final
			{
				return psoVariation;
			}
		public:
			void Clear()final
			{
				ClearResource();
			}
		private:
			void ClearResource()
			{ 
				cs = nullptr;
				for (uint i = 0; i < SIZE_OF_ARRAY(pso); ++i)
					pso[i] = nullptr;
			}
		};
	}
}