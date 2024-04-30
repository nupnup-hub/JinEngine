#pragma once
#include"../JOutline.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12CommandContext; 
		class JDx12GraphicResourceManager;
		class JDx12Outline final : public JOutline
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<1>;
		private:
			std::unique_ptr<JDx12GraphicBufferT<JOutlineConstants>> outlineCB = nullptr;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			std::unique_ptr<JDx12GraphicShaderDataHolder> gShaderData;
		public:
			~JDx12Outline();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public:
			void UpdatePassBuf(const uint width, const uint height, const uint stencilRefOffset) final;
		public:
			//util func extract data and call DrawOutline
			void DrawCamOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper) final;
			void DrawOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper) final;
		private:
			void DrawOutline(JDx12CommandContext* context,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle, 
				const CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle);
		private:
			void BuildResource(JGraphicDevice* device);
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device);
			void BuildUploadBuffer(JGraphicDevice* device);
		private:
			void ClearResource();
		};
	}
}