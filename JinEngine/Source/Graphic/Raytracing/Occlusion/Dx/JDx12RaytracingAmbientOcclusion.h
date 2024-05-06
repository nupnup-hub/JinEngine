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
#include"../JRaytracingAmbientOcclusion.h"
#include"../../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../Buffer/Dx/JHlslDebug.h"
#include"../../../Image/Dx/JDx12ImageConstants.h"
#include"../../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../../FrameResource/JRaytracingConstants.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12CommandContext;
		class JGraphicResourceInfo;
		struct JStateObjectBuildData;
		//미개발
		//raytracing gi 개발후 진행예정.
		class JDx12RaytracingAmbientOcclusion : public JRaytracingAmbientOcclusion
		{
		private:
			static constexpr uint debuggingSampleCount = 32;
		private:
			using HlslDebug = JHlslDebug<JRaytracingDebugConstants<debuggingSampleCount>>;
		private:
			struct SsaoSampleConstants
			{
				JVector4F sample[Constants::ssaoMaxSampleCount];
			};
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> globalRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> localRootSignature;
			Microsoft::WRL::ComPtr<ID3D12StateObject> stateObject;
			std::unique_ptr<JDx12GraphicBuffer> rayGenShaderTable;
			std::unique_ptr<JDx12GraphicBuffer> hitGroupShaderTable;
			std::unique_ptr<JDx12GraphicBuffer> missShaderTable; 
		private:
			std::unique_ptr<JDx12GraphicBufferT<SsaoSampleConstants>> sampleCB;
			JUserPtr<JGraphicResourceInfo> randomVecInfo;
		private:
			std::unique_ptr<HlslDebug> debugHandle;
		public:
			~JDx12RaytracingAmbientOcclusion();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final; 
		public:
			void StreamOutDebugInfo() final;
		public:
			void ComputeAmbientOcclusion(const JGraphicRtAoComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void DispatchRays(JDx12CommandContext* context, ID3D12Resource* rayGenShaderTable, uint width, uint height);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM); 
			void BuildRootSignature(JDx12GraphicDevice* device);
			void BuildShaderResource(JDx12GraphicDevice* device);
		private:
			void BuildRtAoRootSignature(ID3D12Device5* device);
			void BuildRtAoStateObject(ID3D12Device5* device, const JGraphicInfo& info);
			void BuildDxilLibrarySubobject(JStateObjectBuildData& buildData);
			void BuildHitGroupSubobjects(JStateObjectBuildData& buildData);
			void BuildShaderConfig(JStateObjectBuildData& buildData);
			void BuildLocalRootSignatureSubobjects(JStateObjectBuildData& buildData);
			void BuildGlobalRootSignatureSubobjects(JStateObjectBuildData& buildData);
			void BuildPipelineConfig(JStateObjectBuildData& buildData);
			void BuildShaderTables(ID3D12Device5* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildBuffer(JGraphicDevice* device, JDx12GraphicResourceManager* gm);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearStateObject();
			void ClearShaderTable();
			void ClearBuffer();
		};
	}
}