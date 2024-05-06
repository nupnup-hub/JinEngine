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
#include"../JBloom.h"  
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include<wrl/client.h> 
#include<dxgiformat.h> 

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12GraphicResourceManager; 
		class JDx12GraphicDevice;  
		class JDx12CommandContext;
		class JDx12Bloom : public JBloom
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			struct BloomDataSet
			{
			public:
				JDx12CommandContext* context;
			public:  
				ImageProcessingShareData* imageShare = nullptr;
				const JDrawHelper& helper;
			public:
				JDx12GraphicResourceComputeSet exposureSet;
				JDx12GraphicResourceComputeSet srcSet;
				JDx12GraphicResourceComputeSet lumaSet;  
			public:
				//aligned 0 ~ Constants::bloomSampleCount - 1
				JDx12GraphicResourceComputeSet bloomSet[Constants::bloomSampleCount][2]; 
			public:
				JVector2F bloomSize;		//same as lumaSize
				const bool useHighQuality = true;
			private:
				bool isValid = false; 
			public:
				BloomDataSet(JPostProcessComputeSet* computeSet, const JDrawHelper& helper);
			public:
				bool IsValid() const noexcept;
			};
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;		 
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> bloomExtract;					 
			std::unique_ptr<JDx12ComputeShaderDataHolder> bloomExtractAndExtractLuma;		 
			std::unique_ptr<JDx12ComputeShaderDataHolder> blur;
			std::unique_ptr<JDx12ComputeShaderDataHolder> upSample;			//upsample + blur
			std::unique_ptr<JDx12ComputeShaderDataHolder> downSample2;
			std::unique_ptr<JDx12ComputeShaderDataHolder> downSample4; 
		public:
			~JDx12Bloom();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyBloom(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void BeginBloom(const BloomDataSet& set);
			void ExtractLumaAndDonwSample(const BloomDataSet& set);
			void DownSample(const BloomDataSet& set);
			void UpSample(const BloomDataSet& set);
			void UpSample(const BloomDataSet& set, const uint bufferIndex, const JDx12GraphicResourceComputeSet& lowerRefBuf, float upsampleBlendFactor);
			void EndBloom(const BloomDataSet& set);
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet) final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildBloomRootSignature(ID3D12Device* device);
			void BuildBloomPso(ID3D12Device* device, const JGraphicInfo& info);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso();
		};
	}
}