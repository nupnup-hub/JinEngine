#pragma once
#include"../JConvertColor.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
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

		class JDx12ConvertColor : public JConvertColor
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> toLinearRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> toDisplayRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> reverseYRootSignature;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> sdrToLienar;
			std::unique_ptr<JDx12ComputeShaderDataHolder> hdrToLienar;
		private: 
			std::unique_ptr<JDx12ComputeShaderDataHolder> fromSdrToDisplay;
			std::unique_ptr<JDx12ComputeShaderDataHolder> fromHdrToDisplay;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> reverseY;
		public:
			~JDx12ConvertColor();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyToLinearColor(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
			void ApplyToDisplayColor(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
			void ApplyConvertColor(JGraphicConvertColorComputeSet* computeSet)final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildToLinearColorRootSignature(ID3D12Device* device);
			void BuildToLinearColorPso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildToDisplayColorRootSignature(ID3D12Device* device);
			void BuildToDisplayColorPso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildReverseColorRootSignature(ID3D12Device* device);
			void BuildReverseColorPso(ID3D12Device* device, const JGraphicInfo& info);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso();
		};
	}
}