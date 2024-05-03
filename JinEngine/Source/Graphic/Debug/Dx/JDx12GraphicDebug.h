#pragma once 
#include"../JGraphicDebug.h" 
#include"../../JGraphicEnum.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h>
#include<memory>
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12CommandContext; 
		class JDx12GraphicDevice;
		class JGraphicResourceManager;
		class JDx12GraphicResourceManager;

		struct JGraphicInfo;
		class JDrawHelper;

		class JDx12GraphicDebug : public JGraphicDebug
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			enum DEBUG_TYPE
			{
				DEBUG_TYPE_DEPTH,
				DEBUG_TYPE_SPECULAR,
				DEBUG_TYPE_NORMAL,
				DEBUG_TYPE_TANGENT, 
				DEBUG_TYPE_VELOCITY,
				DEBUG_TYPE_AO,
				DEBUG_TYPE_COUNT
			};
		private: 
			struct DebugDataSet
			{ 
			public:
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;  
				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
			public:
				JVector2<uint> size = JVector2<uint>::Zero();
				JVector2F nearFar = 0; 
				bool isNonLinearDepthMap = true;
			public:
				bool allowHzb = false;
				bool allowOccDepth = false;  
			public: 
				bool allowTrigger[DEBUG_TYPE_COUNT]; 
			public:
				DebugDataSet(JDx12CommandContext* context,
					const JGraphicDebugRsComputeSet* debugSet,
					const JDrawHelper& helper, 
					JDx12GraphicResourceComputeSetBufferBase& srcBase,
					JDx12GraphicResourceComputeSetBufferBase& destBase);
			public:
				void SetOcclusionBuffer(JDx12CommandContext* context,
					JGraphicResourceUserInterface* gRInterface,
					const JDrawHelper& helper,
					const J_GRAPHIC_RESOURCE_TYPE srcType,
					const J_GRAPHIC_RESOURCE_TYPE destType,
					JDx12GraphicResourceComputeSetBufferBase& srcBase,
					JDx12GraphicResourceComputeSetBufferBase& destBase)const;
			}; 
		private:
			//graphic
			Microsoft::WRL::ComPtr<ID3D12RootSignature> cRootSignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> specularMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> linearDepthMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::COUNT];
			std::unique_ptr<JDx12ComputeShaderDataHolder> normalMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> tangentMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> velocityMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> aoMapShaderData;
		public:
			~JDx12GraphicDebug();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ComputeCamDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)final;
			void ComputeLitDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)final;
		private:
			void ComputeOccDebug(JDx12CommandContext* context, DebugDataSet& set, const JDrawHelper& helper);
		private:
			void Execute(JDx12CommandContext* context, const DebugDataSet& set, JDx12ComputeShaderDataHolder* holder);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info);
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device); 
		private:
			void ClearResource();
		};
	}
}