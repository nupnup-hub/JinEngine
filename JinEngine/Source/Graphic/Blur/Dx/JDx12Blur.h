#pragma once
#include"../JBlur.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12ComputeShaderDataHolder;
		class JDx12GraphicResourceManager;
		class JDx12Blur : public JBlur
		{ 
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> blur[(uint)J_BLUR_TYPE::COUNT][(uint)J_BLUR_KENEL_SIZE::COUNT];
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyBlur(JGraphicBlurTaskSet* taskSet, const JDrawHelper& helper) final;
		private:
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info);
		};
	}
}