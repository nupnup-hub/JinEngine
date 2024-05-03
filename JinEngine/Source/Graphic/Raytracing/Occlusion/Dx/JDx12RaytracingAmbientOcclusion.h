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