#pragma once
#include"../JPostProcessHistogram.h"   
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;
		class JDx12ClearBufferUseCs;

		class JDx12PostProcessHistogram : public JPostProcessHistogram
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> createHistogramRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> drawHistogramRootSignature;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> createHistogram;
			std::unique_ptr<JDx12ComputeShaderDataHolder> drawHistogram;
		private:
			std::unique_ptr<JDx12ClearBufferUseCs> clearHistogram;
		public:
			~JDx12PostProcessHistogram();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void CreateHistogram(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
			void DrawHistogram(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildHistogramRootSignature(ID3D12Device* device);
			void BuildHistogramPso(ID3D12Device* device, const JGraphicInfo& info);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso();
		};
	}
}