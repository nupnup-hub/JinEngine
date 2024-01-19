#pragma once
#include"../JLightCulling.h" 
#include"../../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../../Object/Component/Light/JLightType.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicDevice;
		class JGraphicResourceInfo;
		class JDx12GraphicResourceManager;
		class JDx12FrameResource;
		class JDx12CullingManager;  

		//referenced by gpu pro7  ch 2.1 by Kevin Ortegren and Emil Persson
		class JDx12LightCulling : public JLightCulling
		{
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mGRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mCRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mDRootSignature;;
		private:
			std::unique_ptr<JDx12GraphicShaderDataHolder> drawLightShader[(uint)JLightType::GetLocalLightCount()];
			std::unique_ptr<JDx12ComputeShaderDataHolder> computeLightClusterShader;
			std::unique_ptr<JDx12ComputeShaderDataHolder> clearOffsetBufferShader;
			std::unique_ptr<JDx12GraphicShaderDataHolder> drawDebugShader;
		private:
			std::unique_ptr<JDx12GraphicBuffer<uint>> resultOutBuffer;
			std::unique_ptr<JDx12GraphicBuffer<uint>> resultOutClearBuffer;
			std::unique_ptr<JDx12GraphicBuffer<uint>> counterClearBuffer; 	 
		private:
			//for debugging
			std::unique_ptr<JDx12GraphicBuffer<uint>> offsetDebugBuffer;
			std::unique_ptr<JDx12GraphicBuffer<uint64>> listDebugBuffer;
		private:
			//Intermediate
			std::unique_ptr<JDx12GraphicBuffer<uint>> resultClearUploadBuffer;
			std::unique_ptr<JDx12GraphicBuffer<uint>> counterClearUploadBuffer;
		private:
			JUserPtr<JGraphicResourceInfo> lightRt[(uint)JLightType::GetLocalLightCount()];
		/*
		private:
			JUserPtr<JGraphicResourceInfo> lowSphereVertex;
			JUserPtr<JGraphicResourceInfo> lowSphereIndex;
			JUserPtr<JGraphicResourceInfo> lowConeVertex;
			JUserPtr<JGraphicResourceInfo> lowConeIndex;
		private: 
			uint lowSphereVertexCount;
			uint lowSphereIndexCount;
			uint lowConeVertexCount;
			uint lowConeIndexCount;
		*/
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicBaseDataSet& baseDataSet) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;  
		public:
			void NotifyNewClusterOption(JGraphicDevice* device, const JGraphicBaseDataSet& baseDataSet) final;
			void NotifyLocalLightCapacityChanged(JGraphicDevice* device, JGraphicResourceManager* gM, const size_t capacity) final;
		public:
			void BindDrawResource(const JGraphicBindSet* bindSet) final; 
			void BindDebugResource(const JGraphicBindSet* bindSet) final;
		public:
			void ExecuteLightClusterTask(const JGraphicLightCullingTaskSet* taskSet, const JDrawHelper& helper)final;
			void ExecuteLightClusterDebug(const JGraphicLightCullingDebugDrawSet* drawSet, const JDrawHelper& helper)final;
		public: 
			void StreamOutDebugInfo(const std::wstring& path, const JGraphicBaseDataSet& baseDataSet)final;
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
		private:
			void DrawLight(ID3D12GraphicsCommandList* cmdList, 
				JDx12GraphicResourceManager* dx12Gm,
				JDx12CullingManager* dx12Cm, 
				JDx12FrameResource* dx12Frame,
				const JDrawHelper& helper);
			void ComputeLightCluster(ID3D12GraphicsCommandList* cmdList,
				JDx12GraphicResourceManager* dx12Gm,
				JDx12CullingManager* dx12Cm,
				JDx12FrameResource* dx12Frame,
				const JDrawHelper& helper);
			void DrawLightClusterDebug(ID3D12GraphicsCommandList* cmdList,
				JDx12GraphicDevice* dx12Device,
				JDx12GraphicResourceManager* dx12Gm, 
				const JDrawHelper& helper);
		private:
			void BuildGraphicRootSignature(ID3D12Device* device);
			void BuildComputeRootSignature(ID3D12Device* device);
			void BuildDebugRootSignature(ID3D12Device* device);
			void BuildGraphicPso(ID3D12Device* device, const JGraphicBaseDataSet& baseDataSet);
			void BuildComputePso(ID3D12Device* device, const JGraphicBaseDataSet& baseDataSet);
			void BuildDebugPso(ID3D12Device* device, const JGraphicBaseDataSet& baseDataSet);
			void BuildCounterClearBuffer(JDx12GraphicDevice* device);
			void BuildResultBuffer(JDx12GraphicDevice* device, const uint localLightCapacity);
			void BuildDebugBuffer(JDx12GraphicDevice* device, const JGraphicBaseDataSet& baseDataSet);
			void BuildRtResource(JDx12GraphicDevice* device, JDx12GraphicResourceManager* dx12Gm, const JGraphicBaseDataSet& baseDataSet);
		private:
			void LoadLightShape(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gM);
		};
	}
}