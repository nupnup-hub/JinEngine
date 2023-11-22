#pragma once
#include"../JImageProcessing.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	class JTexture;
	namespace Graphic
	{
		class JDx12ComputeShaderDataHolder;
		class JDx12GraphicResourceManager;
		class JDx12ImageProcessing : public JImageProcessing
		{ 
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> blurRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> downSampleRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoRootSignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> blur[(uint)J_BLUR_TYPE::COUNT][(uint)J_KENEL_SIZE::COUNT];
			std::unique_ptr<JDx12ComputeShaderDataHolder> downSample[(uint)J_DOWN_SAMPLING_TYPE::COUNT][(uint)J_KENEL_SIZE::COUNT];
			std::unique_ptr<JDx12ComputeShaderDataHolder> ssao[(uint)J_SSAO_TYPE::COUNT];
		private:
			JUserPtr<JTexture> randomTexture;
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyBlur(JGraphicBlurTaskSet* taskSet, const JDrawHelper& helper) final;
			void ApplyMipmapGeneration(JGraphicDownSampleTaskSet* taskSet, const JDrawHelper& helper) final;
			void ApplySsao(JGraphicSsaoTaskSet* taskSet, const JDrawHelper& helper) final;
		private:
			void BuildBlurRootSignature(ID3D12Device* device);
			void BuildBlurPso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildDownSampleRootSignature(ID3D12Device* device);
			void BuildDownSamplePso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildSsaoRooutSignature(ID3D12Device* device);
			void BuildSsaoPso(ID3D12Device* device, const JGraphicInfo& info);
		};
	}
}