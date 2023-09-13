#pragma once 
#include"../JDepthMapDebug.h" 
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h>
#include<memory>
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceManager;
		class JDx12ComputeShaderDataHolder;
		struct JGraphicInfo;
		struct JDrawHelper;
		class JDx12DepthMapDebug final : public JDepthMapDebug
		{
		private:
			//graphic
			Microsoft::WRL::ComPtr<ID3D12RootSignature> cRootSignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> linearDepthMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> nonLinearDepthMapShaderData;
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void DrawCamDepthDebug(const JGraphicDepthMapDebugObjectSet* debugSet, const JDrawHelper& helper)final;
			void DrawLitDepthDebug(const JGraphicDepthMapDebugObjectSet* debugSet, const JDrawHelper& helper)final;
		public:
			void DrawLinearDepthDebug(const JGraphicDepthMapDebugHandleSet* debugSet)final;
			void DrawNonLinearDepthDebug(const JGraphicDepthMapDebugHandleSet* debugSet)final;
		private:
			void DrawLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
				const JVector2<uint> size,
				const float nearF,
				const float farF);
			void DrawNonLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
				const JVector2<uint> size,
				const float nearF,
				const float farF);
		private:
			void BuildComputeResource(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat);
		};
	}
}