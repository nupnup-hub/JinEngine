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
#include"../JRaytracingGI.h"
#include"../../../../Accelerator/Dx/JDx12GpuAcceleratorManager.h"
#include"../../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../../../Buffer/Dx/JDx12GraphicBuffer.h"  
#include"../../../../Shader/Dx/JDx12ShaderDataHolder.h"  
#include"../../../../../Core/Storage/JStorageInterface.h"
#include"../../../../../Object/Component/Light/JLightType.h"
#include"../../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		using RestirTemporalAccumulationData = JDx12GraphicResourceShareData::RestirTemporalAccumulationData;

		struct JStateObjectBuildData;
		class JDx12CommandContext;
		class JDx12RaytracingGI : public JRaytracingGI
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			enum STATE_OBJECT_TYPE : uint
			{ 
				STATE_OBJECT_TYPE_DEFAULT = 0,
				STATE_OBJECT_TYPE_COUNT
			};
		private:
			struct HemisphereSample
			{
				JVector3F direction;
				uint padding = 0;
			}; 
			struct GIPassConstants
			{
			public:
				JMatrix4x4 camInvView = JMatrix4x4::Identity();
				JMatrix4x4 camPreViewProj = JMatrix4x4::Identity();	 
				JVector2F camNearFar = JVector2F::One();
				JVector2F uvToViewA = JVector2F::Zero();
				JVector2F uvToViewB = JVector2F::Zero();
				JVector2F rtSize = JVector2F::One();
				JVector2F invRtSize = JVector2F::One();
				JVector2F origianlRtSize = JVector2F::One();
				JVector2F invOrigianlRtSize = JVector2F::One();
				float tMax = 0;
				uint totalNumPixels = 0;

				JVector3F camPosW = JVector3F::Zero();
				float camNearMulFar = 1;
				JVector3F cameraPrePosW = JVector3F::Zero();
				uint sampleSetSize = 0;
				uint sampleSetMax = 0;
				uint currSampleSetIndex = 0; 
				uint updateCount = 0;

				uint directionalLightRange = 0;
				uint pointLightRange = 0;
				uint spotLightRange = 0;
				uint rectLightRange = 0;

				uint directionalLightOffset = 0;
				uint pointLightOffset = 0;
				uint spotLightOffset = 0;
				uint rectLightOffset = 0;

				uint totalLightCount = 0;
				float invTotalLightCount = 0; 
				uint forceClearReservoirs = 0; 
				uint pad00 = 0;
				uint pad01 = 0;
				//uint rectLightVerticesIndex = 0;
				//uint rectLightIndiciesIndex = 0;
			public:
				GIPassConstants() = default; 
			};
			struct UserPrivateData : public Core::JVolatileStorageInterface
			{
			public:
				JDx12GraphicBufferT<GIPassConstants> frameBuffer;
			public:
				//JUserPtr<JMeshGeometry> pointLitShape;
				///JUserPtr<JMeshGeometry> spotLitShape;
				//JUserPtr<JMeshGeometry> rectLitShape;
			public:
				JVector3F camPrePosW = JVector3F::Zero();
			public:
				int currReserviorIndex = 0;
				int preReserviorIndex = 1; 
			public:
				uint currSampleSetIndex = 0; 
			public: 
				UserPrivateData(JGraphicDevice* device);
				~UserPrivateData();
			public:
				void Begin(const JDrawHelper& helper);
				void End(const JDrawHelper& helper);
			};
			struct GIDataSet
			{ 
			public:
				JDx12CommandContext* context;
				JDx12GraphicDevice* device;
				UserPrivateData* userPrivate;
			public:
				RestirTemporalAccumulationData* sharedata;
			public:
				JUserPtr<JCamera> cam;
			public:
				JDx12GraphicResourceComputeSet rtSet;
				JDx12GraphicResourceComputeSet dsSet;
			public:
				JDx12GraphicResourceComputeSet albedoSet; 
				JDx12GraphicResourceComputeSet lightPropSet; 
				JDx12GraphicResourceComputeSet normalSet;
				//JDx12GraphicResourceComputeSet velocitySet; 
			public:
				//for rt spatial-temporal process
				JDx12GraphicResourceComputeSet preRsSet;
				JDx12GraphicResourceComputeSet preDsSet;
				JDx12GraphicResourceComputeSet preNormalSet;
				//JDx12GraphicResourceComputeSet preTangentSet;
			public:
				JDx12GraphicResourceComputeSet initialSampleSet;
				JDx12GraphicResourceComputeSet temporalReserviorSet[2];
				JDx12GraphicResourceComputeSet spatialReserviorSet[2];
				JDx12GraphicResourceComputeSet colorIntermediate;
				JDx12GraphicResourceComputeSet destSet;
			public:
				JDx12GraphicResourceComputeSet* preTemporalReserviorSet;
				JDx12GraphicResourceComputeSet* currTemporalReserviorSet;
				JDx12GraphicResourceComputeSet* preSpatialReserviorSet;
				JDx12GraphicResourceComputeSet* currSpatialReserviorSet;
			public:
				JDx12AcceleratorResourceComputeSet accelSet;
			public:
				JVector2<uint> halfResolution;
				JVector2<uint> oriResolution;
				JVector2<uint> threadDim;
			public:
				uint currFrameIndex = 0;
				uint preFrameIndex = 0;
			public: 
				int skyMapSrvHeapIndex= invalidIndex;
			public:
				GIDataSet(const JGraphicRtGiComputeSet* computeSet, const JDrawHelper& helper);
			public:
				void SetUserPrivate(UserPrivateData* data, const JDrawHelper& helper); 
			public:
				bool IsValid()const noexcept;
			};
		private:
			//step 1 resource
			Microsoft::WRL::ComPtr<ID3D12RootSignature> raytracingRootSignature; 
			Microsoft::WRL::ComPtr<ID3D12StateObject> stateObject[STATE_OBJECT_TYPE_COUNT];
			std::unique_ptr<JDx12GraphicBuffer> rayGenShaderTable;
			std::unique_ptr<JDx12GraphicBuffer> hitGroupShaderTable;
			std::unique_ptr<JDx12GraphicBuffer> missShaderTable;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> reuseRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> finalRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> upsampleRootSignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> reuseSamplingShader; 
			std::unique_ptr<JDx12ComputeShaderDataHolder> finalShader; 
			std::unique_ptr<JDx12ComputeShaderDataHolder> upsampleShader;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> clearRootSignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> clearShader;
		private:
			std::unique_ptr<JDx12GraphicBufferT<HemisphereSample>> hemiSample;
		private:
			std::unordered_map<size_t, std::unique_ptr<UserPrivateData>> userPrivate;
			uint computeCount = 0;
		public:
			~JDx12RaytracingGI();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public:
			void ComputeGI(const JGraphicRtGiComputeSet* computeSet, const JDrawHelper& helper)final;
		private:
			void Begin(GIDataSet& set, const JDrawHelper& helper);
			void InitializeSampling(const GIDataSet& set, const JDrawHelper& helper);
			void ReuseSampling(const GIDataSet& set, const JDrawHelper& helper); 
			void FinalColor(const GIDataSet& set, const JDrawHelper& helper);
			void Upsample(const GIDataSet& set, const JDrawHelper& helper);
			void ClearRestirResource(const GIDataSet& set, const JDrawHelper& helper);
			void InitializeSamplingTest(const GIDataSet& set, const JDrawHelper& helper);		//for debugging
			void End(const GIDataSet& set, const JDrawHelper& helper);
		private:
			void DispatchRays(JDx12CommandContext* context, const STATE_OBJECT_TYPE type, const uint width, const uint height);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM); 
			void BuildRootSignature(JDx12GraphicDevice* device);
			void BuildShaderResource(JDx12GraphicDevice* device);
		private:
			//build raytracing resource
			void BuildRtRootSignature(ID3D12Device5* device);
			void BuildRtStateObject(ID3D12Device5* device, const JGraphicInfo& info, const JGraphicOption& option, const STATE_OBJECT_TYPE type);
			void BuildDxilLibrarySubobject(JStateObjectBuildData& buildData, const JGraphicOption& option);
			void BuildHitGroupSubobjects(JStateObjectBuildData& buildData);
			void BuildShaderConfig(JStateObjectBuildData& buildData); 
			void BuildGlobalRootSignatureSubobjects(JStateObjectBuildData& buildData);
			void BuildPipelineConfig(JStateObjectBuildData& buildData);
			void BuildRtShaderTables(ID3D12Device5* device, const JGraphicInfo& info);
		private:
			//build compute resource
			void BuildComputeRootSignature(ID3D12Device* device);
			void BuildComputePso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildBuffer(JDx12GraphicDevice* device);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearStateObject();
			void ClearShaderTable();
			void ClearPso(); 
			void ClearBuffer();
			void ClearUserPrivateData();
		}; 
	}
}