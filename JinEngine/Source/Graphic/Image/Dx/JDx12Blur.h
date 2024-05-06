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
#include"../JBlur.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include"../../../Core/Math/JMatrix.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{ 
	namespace Graphic
	{ 
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;   

		class JDx12Blur : public JBlur
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			struct BlurConstants
			{   
				JVector2F size;
				JVector2F invSize;
				int mipLevel;
				float sharpness;
				int blurPad01;
				int blurPad02;
				JVector4F kernel[JKenelType::MaxSize()];	//112	
			};
			struct BlurExceptKernelConstants
			{  
				JVector2F size;
				JVector2F invSize;
				int mipLevel;
				float sharpness;
			};
		public: 
			static constexpr size_t constantsSize = sizeof(BlurConstants);
			static constexpr size_t constantsExceptKenelSize = sizeof(BlurExceptKernelConstants);
		private:
			const size_t guid;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> blurRootSignature;
		private:
			//blur * kenel * 2 ... 2 == (vertical, horizontal)
			std::unique_ptr<JDx12ComputeShaderDataHolder> blur[(uint)J_BLUR_TYPE::COUNT * (uint)J_KERNEL_SIZE::COUNT * 2];
		public:
			JDx12Blur();
			~JDx12Blur();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyBlur(JGraphicBlurComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option); 
		private:
			void BuildBlurRootSignature(ID3D12Device* device);
			void BuildBlurPso(ID3D12Device* device, const JGraphicInfo& info);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso(); 
		};
	}
}