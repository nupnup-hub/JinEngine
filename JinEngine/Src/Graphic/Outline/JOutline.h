#pragma once 
#include"JOutlineConstants.h" 
#include"../Upload/JUploadBuffer.h" 
#include"../../Object/Resource/Shader/JShaderData.h"  
#include"../../../Lib/DirectX/d3dx12.h"  
#include<wrl/client.h>   
#include<memory>
#include<dxgiformat.h>
 
namespace JinEngine
{
	namespace Graphic
	{
		class JOutline
		{
		private:
			std::unique_ptr<JUploadBuffer<JOutlineConstants>> outlineCB = nullptr;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			std::unique_ptr<JGraphicShaderData> gShaderData;
		public:
			void Initialize(ID3D12Device* device, 
				const DXGI_FORMAT& rtvFormat, 
				const DXGI_FORMAT& dsvFormat,
				const uint width,
				const uint height,
				const uint cbOffset);
			void Clear();
		public:
			void UpdatePassBuf(const uint width, const uint height, const uint cbOffset);
		public:
			void DrawOutline(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE dsHandle);
		private:
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device, const DXGI_FORMAT& rtvFormat, const DXGI_FORMAT& dsvFormat);
			void BuildUploadBuffer(ID3D12Device* device);
		};
	}
}