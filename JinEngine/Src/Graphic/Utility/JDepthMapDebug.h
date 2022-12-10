#pragma once
#include"../../Object/Resource/Shader/JShaderData.h"
#include"../../Utility/JVector.h"
#include"../../../Lib/DirectX/d3dx12.h" 
#include<wrl/client.h>   
#include<memory>
#include<dxgiformat.h>
/*
struct ID3D12DescriptorHeap;
struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct CD3DX12_GPU_DESCRIPTOR_HANDLE;
*/

namespace JinEngine
{
	namespace Graphic
	{
		class JDepthMapDebug
		{
		private:
			//graphic
			Microsoft::WRL::ComPtr<ID3D12RootSignature> cRootSignature;
			std::unique_ptr<JComputeShaderData> linearDepthMapShaderData;
			std::unique_ptr<JComputeShaderData> nonLinearDepthMapShaderData;
		public: 
			void Initialize(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat);
			void Clear();
		public:
			void DrawLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
				const JVector2<uint> size,
				const float camNear,
				const float camFar);
			void DrawNonLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
				const JVector2<uint> size,
				const float camNear,
				const float camFar);
		private:
			void BuildComputeResource(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat);
		};
	}
}