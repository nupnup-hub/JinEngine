#pragma once
#include"../JAntialise.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include"../../../Core/Math/JMatrix.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	class JTexture;
	namespace Graphic
	{ 
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;

		class JDx12Antialise : public JAntialise
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> fxaaRootSignature;
			Microsoft::WRL::ComPtr<ID3D12CommandSignature> fxaaCommandSignature;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> fxaaPass1InputLinearColor;
			std::unique_ptr<JDx12ComputeShaderDataHolder> fxaaPass1InputLuma;
			std::unique_ptr<JDx12ComputeShaderDataHolder> fxaaPass2Vertical;
			std::unique_ptr<JDx12ComputeShaderDataHolder> fxaaPass2Horizontal;
			std::unique_ptr<JDx12ComputeShaderDataHolder> fxaaResolveWork;
		public: 
			~JDx12Antialise();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyAA(JGraphicAAComputeSet* computeSet, const JDrawHelper& helper) final;
			void ApplyFxaa(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)final;
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet) final;
		private:
			void BuildResource(JDx12GraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(JDx12GraphicDevice* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildFxaaRootSignature(ID3D12Device* device);
			void BuildFxaaCommandSignature(ID3D12Device* device);
			void BuildFxaaPso(ID3D12Device* device, const JGraphicInfo& info);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso(); 
		};
	}
}