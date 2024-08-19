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
#include"../JSceneVelocity.h"
#include"../../DataSet/Dx/JDx12GraphicTaskDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/JGraphicResourceInterface.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"   
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12CommandContext;
		class JDx12FrameResource;
		class JDx12CullingManager;
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;
		class JDx12SceneVelocity final : public JSceneVelocity
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<(uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT>;
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> velocityRootsignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> velocityShader;
		public:
			~JDx12SceneVelocity();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;  
		public:
			void Compute(const JGraphicVelocityComputeSet* set, const JDrawHelper& helper)final;
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void ClearResource();
		};
	}
}