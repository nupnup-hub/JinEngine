#pragma once
#include"../JPostProcessExposure.h" 
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include"../../../Core/Math/JMatrix.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;

		class JDx12PostProcessExposure : public JPostProcessExposure
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> extractRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> updateAdaptRootSignature;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> extract;
			std::unique_ptr<JDx12ComputeShaderDataHolder> updateAdapt;
		public:
			~JDx12PostProcessExposure();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ExtractExposure(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
			void UpdateExposure(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildExtractRootSignature(ID3D12Device* device);
			void BuildExtractPso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildUpdateAdaptRootSignature(ID3D12Device* device);
			void BuildUpdateAdaptPso(ID3D12Device* device, const JGraphicInfo& info);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso();
		};
	}
}