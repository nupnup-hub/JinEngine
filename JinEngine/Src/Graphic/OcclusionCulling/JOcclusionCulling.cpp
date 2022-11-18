#include"JOcclusionCulling.h" 
#include"../JGraphicInfo.h"
#include"../FrameResource/JBoundingObjectConstants.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Object/Component/Camera/JCamera.h"   
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Shader/JShader.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../Object/Resource/Material/JMaterial.h"
#include"../../Utility/JMathHelper.h" 
#include"../../../Lib/DirectX/d3dx12.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace JinEngine
{
	namespace Graphic
	{
		void JOcclusionCulling::Initialize(ID3D12Device* d3dDevice,
			const CD3DX12_STATIC_SAMPLER_DESC& samDesc,
			const uint occlusionDsvCapacity,
			const uint objectCapacity)
		{
			BuildRootSignature(d3dDevice, samDesc, occlusionDsvCapacity);
			BuildUploadBuffer(d3dDevice, objectCapacity);
		}
		void JOcclusionCulling::Clear()
		{
			camera.Clear(); 
		}
		ID3D12RootSignature* JOcclusionCulling::GetRootSignature()const noexcept
		{
			return mRootSignature.Get();
		}
		void JOcclusionCulling::SetCullingTriger(const bool value)noexcept
		{
			canCulling = value;
		}
		bool JOcclusionCulling::CanCulling()const noexcept
		{
			return canCulling;
		}
		void JOcclusionCulling::UpdateCamera(JCamera* updateCamera)
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
		void JOcclusionCulling::UpdateObjectCapacity(ID3D12Device* device, const uint objectCapacity)
		{
			bbSphereBuffer->Clear();
			bbSphereBuffer->Build(device, objectCapacity);

			queryResultBuffer->Clear();
			queryResultBuffer->Build(device, objectCapacity);

			JShader* shader =JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER);
			shader->CompileInterface()->RecompileGraphicShader();
		}
		void JOcclusionCulling::BuildRootSignature(ID3D12Device* d3dDevice, const CD3DX12_STATIC_SAMPLER_DESC& samDesc, const uint occlusionDsvCapacity)
		{
			static constexpr int slotCount = 5;
			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

			CD3DX12_DESCRIPTOR_RANGE texTable00;
			texTable00.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, occlusionDsvCapacity, 1);

			// Create root CBV. 
			slotRootParameter[0].InitAsShaderResourceView(0);
			slotRootParameter[1].InitAsUnorderedAccessView(0); 
			slotRootParameter[2].InitAsDescriptorTable(1, &texTable00);
			slotRootParameter[3].InitAsConstantBufferView(0);
			slotRootParameter[4].InitAsConstantBufferView(1);

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
		void JOcclusionCulling::BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity)
		{
			bbSphereBuffer = std::make_unique<JUploadBuffer<JBoundSphereConstants>>(false);
			queryResultBuffer = std::make_unique<JUploadBuffer<int>>(false);
			depthMapInfoCB = std::make_unique<JUploadBuffer<JDepthMapInfoConstants>>(true);
			occlusionPassCB = std::make_unique<JUploadBuffer<JOcclusionPassConstants>>(true);
			 
			bbSphereBuffer->Build(device, objectCapacity);
			queryResultBuffer->Build(device, objectCapacity);
			depthMapInfoCB->Build(device, 1);
			occlusionPassCB->Build(device, 1);
		}
	}
}