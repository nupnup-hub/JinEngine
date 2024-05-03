#pragma once
#include"../JToneMapping.h"  
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

		class JDx12ToneMapping : public JToneMapping
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		public:
			enum class TM_TYPE
			{
				SDR = 0,
				HDR, 
				COUNT
			};
			enum TM_ADDITIONAL_OPTION
			{
				TM_ADDITIONAL_OPTION_NONE = 0,
				TM_ADDITIONAL_OPTION_OUT_LUMA = 1 << 0,
				TM_ADDITIONAL_OPTION_EXPOSURE = 1 << 1,
				TM_ADDITIONAL_OPTION_BLOOM = 1 << 2,
				TM_ADDITIONAL_OPTION_VARIATION_COUNT = (1 << 3 ) - 1
			};
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> tmRootSignature;
		private: 
			std::unique_ptr<JDx12ComputeShaderDataHolder> tm[(uint)TM_TYPE::COUNT + (uint)TM_TYPE::COUNT * TM_ADDITIONAL_OPTION_VARIATION_COUNT];
		public:
			~JDx12ToneMapping();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		private: 
			uint CalIndex(const JGraphicOption& option)const noexcept;
		public: 
			void ApplyToneMapping(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildToneMappingRootSignature(ID3D12Device* device);
			void BuildToneMappingPso(ID3D12Device* device, const JGraphicInfo& info); 
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso(); 
		};
	}
}