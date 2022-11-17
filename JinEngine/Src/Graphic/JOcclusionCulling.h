#pragma once
#include "../Utility/JVector.h"
#include"../Core/Pointer/JOwnerPtr.h"
#include<DirectXCollision.h>
#include<wrl/client.h>  

struct ID3D12RootSignature;
struct ID3D12Device;
struct CD3DX12_STATIC_SAMPLER_DESC;

namespace JinEngine
{
	class JCamera;
	class JShader;
	namespace Graphic
	{
		class JOcclusionCulling
		{ 
		private:
			Core::JUserPtr<JCamera> camera; 
			DirectX::BoundingFrustum updateFrustum;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		private:
			bool canCulling = false;
		private:
			float posFactor = 2;
			float rotFactor = 15;
		public:
			void Initialize(ID3D12Device* d3dDevice, const CD3DX12_STATIC_SAMPLER_DESC& samDesc,  const uint occlusionDsvCapacity);
			void Clear();
		public:
			void Update(JCamera* updateCamera);
		public: 
			bool CanCulling()const noexcept;
			void SetCullingTriger(const bool value)noexcept;
		public:
			ID3D12RootSignature* GetRootSignature()const noexcept; 
		private:
			void BuildRootSignature(ID3D12Device* d3dDevice, const CD3DX12_STATIC_SAMPLER_DESC& samDesc, const uint occlusionDsvCapacity);
		};
	}
}