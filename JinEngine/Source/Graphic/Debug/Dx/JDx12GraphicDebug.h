#pragma once 
#include"../JGraphicDebug.h" 
#include"../../JGraphicEnum.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h>
#include<memory>
#include<dxgiformat.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12ComputeShaderDataHolder; 
		class JDx12GraphicDevice;
		class JGraphicResourceManager;
		class JDx12GraphicResourceManager;

		struct JGraphicInfo;
		struct JDrawHelper;

		class JDx12GraphicDebug : public JGraphicDebug
		{
		private:
			enum DEBUG_TYPE
			{
				DEPTH,
				NORMAL,
				AO
			};
		private:
			struct DebugDataSet
			{
			public:
				ID3D12GraphicsCommandList* cmdList;
				JDx12GraphicDevice* dx12Device;
				JDx12GraphicResourceManager* dx12Gm;
			public:
				//depth
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthSrcHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthDestHandle;
			public:
				//normal
				CD3DX12_GPU_DESCRIPTOR_HANDLE normalSrcHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE normalDestHandle;
			public:
				//ao
				CD3DX12_GPU_DESCRIPTOR_HANDLE aoSrcHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE aoDestHandle; 
			public:
				JVector2<uint> size = JVector2<uint>::Zero();
				float nearF = 0;
				float farF = 0;
				bool isNonLinearDepthMap = true;
			public:
				bool allowDepth = false;
				bool allowNormal = false;
				bool allowAo = false;
				bool allowOccDepth = false;
			public: 
				DebugDataSet(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper); 
			};
		private:
			//graphic
			Microsoft::WRL::ComPtr<ID3D12RootSignature> cRootSignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> linearDepthMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::COUNT];
			std::unique_ptr<JDx12ComputeShaderDataHolder> normalMapShaderData;
			std::unique_ptr<JDx12ComputeShaderDataHolder> aoMapShaderData;
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ComputeCamDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)final;
			void ComputeLitDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)final;
		private: 
			void ComputeOccDebug(DebugDataSet set, const JDrawHelper& helper);
		private:
			void ComputeDebug(const DebugDataSet& set, const DEBUG_TYPE type, JDx12ComputeShaderDataHolder* holder);
		private:
			void BuildRootSignature(ID3D12Device* device);
			void BuildDepthPso(ID3D12Device* device);
			void BuildNormalPso(ID3D12Device* device);
			void BuildAoPso(ID3D12Device* device);
		};
	}
}