#pragma once  
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Core/Math/JVector.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12CommandContext; 
		class JDx12GraphicResourceManager;
		class JGraphicDevice;
		class JGraphicResourceInfo; 
		class JDx12ClearBufferUseCs
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> clear;
			uint dataSize = 0; 
		public:
			~JDx12ClearBufferUseCs();
		public:
			void Initialize(ID3D12Device* device,  
				const JVector3<uint> threadDim,
				const uint dataSize,
				const size_t clearValue);
			void Clear();
		public:
			void Execute(JDx12CommandContext* context, const JUserPtr<JGraphicResourceInfo>& info);
			void Execute(JDx12CommandContext* context, const JUserPtr<JGraphicResourceInfo>& info, const D3D12_RESOURCE_STATES beforeState);
		};
	}
}