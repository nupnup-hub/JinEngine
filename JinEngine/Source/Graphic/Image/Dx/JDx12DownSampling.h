#pragma once
#include"../JDownSampling.h" 
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

		class JDx12DownSampling : public JDownSampling
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			enum class DOWN_SAMPLE_STATE
			{
				EXIST,
				WIDTH_DOWN,
				HEIGHT_DOWN,
				WIDTH_HEIGHT_DOWN,
				COUNT
			};
		private:
			const size_t guid;
		private: 
			Microsoft::WRL::ComPtr<ID3D12RootSignature> downSampleRootSignature; 
		private: 
			std::unique_ptr<JDx12ComputeShaderDataHolder> downSample[(uint)DOWN_SAMPLE_STATE::COUNT];
		public:
			JDx12DownSampling();
			~JDx12DownSampling();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyMipmapGeneration(JGraphicDownSampleComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private: 
			void BuildDownSampleRootSignature(ID3D12Device* device);
			void BuildDownSamplePso(ID3D12Device* device, const JGraphicInfo& info); 
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso(); 
		};
	}
}