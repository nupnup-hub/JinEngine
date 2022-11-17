#include"JOcclusionCulling.h"
#include"../Core/Exception/JExceptionMacro.h"
#include"../Object/Component/Camera/JCamera.h"  
#include"../Object/Resource/JResourceObjectFactory.h"
#include"../Object/Resource/Shader/JShader.h"
#include"../Utility/JMathHelper.h" 
#include"../../Lib/DirectX/d3dx12.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace JinEngine
{
	namespace Graphic
	{
		void JOcclusionCulling::Initialize(ID3D12Device* d3dDevice, const CD3DX12_STATIC_SAMPLER_DESC& samDesc,  const uint occlusionDsvCapacity)
		{
			BuildRootSignature(d3dDevice, samDesc, occlusionDsvCapacity);
		}
		void JOcclusionCulling::Clear()
		{
			camera.Clear();
			mRootSignature.Reset();
		}
		void JOcclusionCulling::Update(JCamera* updateCamera)
		{
			if (!camera.IsValid())
			{
				camera = Core::GetUserPtr(updateCamera);
				canCulling = true;
				updateFrustum = camera->GetBoundingFrustum();
				return;
			}

			const BoundingFrustum nowFrustum = camera->GetBoundingFrustum();
			XMVECTOR distanceV = XMVector3Length(XMVectorSubtract(XMLoadFloat3(&nowFrustum.Origin), XMLoadFloat3(&updateFrustum.Origin)));
			XMFLOAT3 distanceF;
			XMStoreFloat3(&distanceF, distanceV);

			XMVECTOR quaternionV = XMQuaternionDot(XMLoadFloat4(&nowFrustum.Orientation), XMLoadFloat4(&updateFrustum.Orientation));
			XMFLOAT3 quaternionF;
			XMStoreFloat3(&quaternionF, quaternionV);

			float distanceFactor = abs(distanceF.x);
			float quaternionFactor = (1 - quaternionF.x) * 10;

			if (distanceFactor + quaternionFactor > 2)
			{
				canCulling = true;
				updateFrustum = camera->GetBoundingFrustum();
			}
		}
		bool JOcclusionCulling::CanCulling()const noexcept
		{
			return canCulling;
		}
		void JOcclusionCulling::SetCullingTriger(const bool value)noexcept
		{
			canCulling = value;
		}
		ID3D12RootSignature* JOcclusionCulling::GetRootSignature()const noexcept
		{
			return mRootSignature.Get();
		}
		void JOcclusionCulling::BuildRootSignature(ID3D12Device* d3dDevice, const CD3DX12_STATIC_SAMPLER_DESC& samDesc, const uint occlusionDsvCapacity)
		{
			static constexpr int slotCount = 5;
			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

			CD3DX12_DESCRIPTOR_RANGE texTable00;
			texTable00.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, occlusionDsvCapacity, 1);

			// Create root CBV.
			slotRootParameter[0].InitAsConstantBufferView(0);
			slotRootParameter[1].InitAsConstantBufferView(1);
			slotRootParameter[1].InitAsConstantBufferView(2);
			slotRootParameter[2].InitAsUnorderedAccessView(0);
			slotRootParameter[3].InitAsDescriptorTable(1, &texTable00); 

			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, 1, &samDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
			ComPtr<ID3DBlob> serializedRootSig = nullptr;
			ComPtr<ID3DBlob> errorBlob = nullptr;
			HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
				serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

			if (errorBlob != nullptr)
			{
				::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			ThrowIfFailedHr(hr);
			ThrowIfFailedHr(d3dDevice->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(mRootSignature.GetAddressOf())));
		}
	}
}