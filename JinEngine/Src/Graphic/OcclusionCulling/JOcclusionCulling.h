#pragma once
#include "../../Utility/JVector.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../Upload/JUploadBuffer.h"
#include"../Utility/JHlslDebug.h"
#include"JOcclusionConstants.h"
#include<DirectXCollision.h>
#include<wrl/client.h>  
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
		class JOcclusionCulling
		{
		private:
			std::unique_ptr<JUploadBuffer<JOcclusionObjectConstants>> objectBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<float>> queryOutBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<float>> queryResultBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<JDepthMapInfoConstants>> depthMapInfoCB = nullptr;
			std::unique_ptr<JUploadBuffer<JOcclusionPassConstants>> occlusionPassCB = nullptr;
			JOcclusionObjectConstants objectConstants;
		private:
			std::unique_ptr<JHlslDebug<HZBDebugInfo>> debugBuffer = nullptr;
		private:
			float* queryResult = nullptr;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		private:
			Core::JUserPtr<JCamera> camera;
			DirectX::BoundingFrustum updateFrustum;
		private:
			bool canCulling = false;
			bool isHZBCulling = true;
			bool isQueryUpdated = false;
		private:
			float posFactor = 2;
			float rotFactor = 15;
		public:
			void Initialize(ID3D12Device* d3dDevice, const JGraphicInfo& info);
			void Clear();
		public:
			ID3D12RootSignature* GetRootSignature()const noexcept;
			void SetCullingTriger(const bool value)noexcept;
		public:
			bool CanCulling()const noexcept;
			bool IsCulled(const uint objIndex)const noexcept;
		public:
			void UpdateCamera(JCamera* updateCamera);
			void UpdateOcclusionMapInfo(ID3D12Device* device, const JGraphicInfo& info);
			void UpdateObjectCapacity(ID3D12Device* device, const uint objectCapacity);
		public:
			void UpdateObject(JRenderItem* rItem, const uint submeshIndex, const uint buffIndex);
			void UpdatePass(JScene* scene, const uint queryCount, const uint cbIndex);
		public:
			void DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapUavHandle,
				std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& depthResource,
				const uint samplingCount,
				const uint srvDescriptorSize);
			void OcclusuinCulling(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle);
		public:
			void ReadCullingResult();
		private:
			void BuildRootSignature(ID3D12Device* d3dDevice, const uint occlusionDsvCapacity);
			void BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity, const uint occlusionCapacity);
		};
	}
}