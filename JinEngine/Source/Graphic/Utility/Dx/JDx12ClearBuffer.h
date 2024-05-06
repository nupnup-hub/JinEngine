/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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