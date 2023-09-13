#pragma once
#include"../JOutline.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicShaderDataHolder;
		class JDx12GraphicResourceManager;
		class JDx12Outline final : public JOutline
		{
		private:
			std::unique_ptr<JDx12GraphicBuffer<JOutlineConstants>> outlineCB = nullptr;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			std::unique_ptr<JDx12GraphicShaderDataHolder> gShaderData;
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void UpdatePassBuf(const uint width, const uint height, const uint stencilRefOffset) final;
		public:
			//util func extract data and call DrawOutline
			void DrawCamOutline(const JGraphicOutlineObjectSet* drawSet, const JDrawHelper& helper) final;
			void DrawOutline(const JGraphicOutlineHandleSet* drawSet, const JDrawHelper& helper) final;
		private:
			void DrawOutline(ID3D12GraphicsCommandList* cmdList,
				JDx12GraphicResourceManager* dx12Gm,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle, 
				const CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle);
		private:
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device, const DXGI_FORMAT& rtvFormat, const DXGI_FORMAT& dsvFormat);
			void BuildUploadBuffer(JGraphicDevice* device);
		};
	}
}