#pragma once
#include "../../Utility/JVector.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../Upload/JUploadBuffer.h"
#include"JOcclusionConstants.h"
#include<DirectXCollision.h>
#include<wrl/client.h>  
#include<vector>

struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct CD3DX12_STATIC_SAMPLER_DESC;

namespace JinEngine
{
	class JCamera; 
	class JGameObject;
	namespace Graphic
	{ 
		struct JGraphicInfo;
		class JOcclusionCulling
		{
		private:
			std::unique_ptr<JUploadBuffer<JBoundSphereConstants>> bbSphereBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<int>> queryResultBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<JDepthMapInfoConstants>> depthMapInfoCB = nullptr;
			std::unique_ptr<JUploadBuffer<JOcclusionPassConstants>> occlusionPassCB = nullptr;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		private:
			Core::JUserPtr<JCamera> camera;
			DirectX::BoundingFrustum updateFrustum;
		private:
			bool canCulling = false;
		private:
			float posFactor = 2;
			float rotFactor = 15;
		public:
			void Initialize(ID3D12Device* d3dDevice, 
				const CD3DX12_STATIC_SAMPLER_DESC& samDesc, 
				const uint occlusionDsvCapacity, 
				const uint objectCapacity);
			void Clear();
		public:
			ID3D12RootSignature* GetRootSignature()const noexcept;
			void SetCullingTriger(const bool value)noexcept;
		public:
			bool CanCulling()const noexcept;
		public:
			void UpdateCamera(JCamera* updateCamera);
			void UpdateObjectCapacity(ID3D12Device* device, const uint objectCapacity);
		public:
			void DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
				ID3D12Resource* depthMap,
				CD3DX12_CPU_DESCRIPTOR_HANDLE depthMapHandle,
				const uint objOffset);
		private:
			void BuildRootSignature(ID3D12Device* d3dDevice, const CD3DX12_STATIC_SAMPLER_DESC& samDesc, const uint occlusionDsvCapacity);
			void BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity);
		};
	}
}