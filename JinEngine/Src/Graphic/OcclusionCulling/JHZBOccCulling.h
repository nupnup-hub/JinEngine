#pragma once
#include"JOccBase.h"
#include"JOcclusionConstants.h"
#include"../Upload/JUploadBuffer.h"
#include"../Utility/JHlslDebug.h"
#include "../../Utility/JVector.h"  
#include<DirectXCollision.h> 
#include<vector>

struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace JinEngine
{
	class JCamera;
	class JGameObject;
	class JRenderItem;
	class JScene;

	namespace Graphic
	{
		struct JGraphicInfo;
		struct JGraphicOption;
		class JHZBOccCulling final: public JOccBase
		{ 
		private: 
			std::unique_ptr<JUploadBuffer<JOcclusionObjectConstants>> objectBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<float>> queryOutBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<float>> queryResultBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<JDepthMapInfoConstants>> depthMapInfoCB = nullptr;
			std::unique_ptr<JUploadBuffer<JOcclusionPassConstants>> occlusionPassCB = nullptr;
			//Debug
			//std::unique_ptr<JHlslDebug<HZBDebugInfo>> debugBuffer = nullptr;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		private:
			JOcclusionObjectConstants objectConstants;
			float* queryResult = nullptr; 
		public:
			void Initialize(ID3D12Device* d3dDevice, const JGraphicInfo& info);
			void Clear();
		public:
			ID3D12RootSignature* GetRootSignature()const noexcept;
		public:
			bool IsCulled(const uint objIndex)const noexcept;
		public:
			//Set upload buffer and shader macro
			void UpdateOcclusionMapInfo(ID3D12Device* device, const JGraphicInfo& info);
			void UpdateObjectCapacity(ID3D12Device* device, const uint objectCapacity);
		public:
			//update upload buffer data
			void UpdateObject(JRenderItem* rItem, const uint buffIndex);
			void UpdatePass(JScene* scene, const JGraphicInfo& info, const JGraphicOption& option, const uint queryCount, const uint cbIndex);
		public:
			void DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapUavHandle, 
				const uint samplingCount,
				const uint srvDescriptorSize);
			void OcclusuinCulling(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle);
		public:
			void ReadCullingResult();
		private:
			void BuildRootSignature(ID3D12Device* d3dDevice, const uint occlusionDsvCapacity);
			void BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity, const uint occlusionCapacity);
		};
	}
}