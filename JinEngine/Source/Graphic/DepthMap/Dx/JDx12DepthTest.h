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
#include"../JDepthTest.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Core/Utility/JTypeTraitUtility.h" 
#include"../../../Core/Geometry/Mesh/JMeshType.h"  
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12FrameResource;
		class JDx12CommandContext;
		class JDx12DepthTest final : public JDepthTest
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			Core::EnumCountArray<std::unique_ptr<JDx12GraphicShaderDataHolder>, TEST_TYPE> gShaderData;
			//std::unique_ptr<JGraphicShaderData> gShaderData[(uint)TEST_TYPE::COUNT];
		public:
			~JDx12DepthTest();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void DrawSceneBoundingBox(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition)final;
			void DrawHdOcclusionQueryObject(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition) final;
		public:
			bool BindGraphicResource(const JGraphicBindSet* bindSet, const JDrawHelper& helper) final;
		private:
			bool BindGraphicResource(JDx12CommandContext* context, const JDrawHelper& helper);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info);
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device, const DXGI_FORMAT depthStencilFormat);
		private:
			void ClearResource();
		};
	}
}
 