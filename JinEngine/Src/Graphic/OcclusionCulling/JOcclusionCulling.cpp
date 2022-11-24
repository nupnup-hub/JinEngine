#include"JOcclusionCulling.h" 
#include"../JGraphicInfo.h"
#include"../FrameResource/JBoundingObjectConstants.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Component/Camera/JCamera.h"   
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Component/Transform/JTransform.h"
#include"../../Object/Resource/JResourceManager.h" 
#include"../../Object/Resource/Shader/JShader.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../Object/Resource/Material/JMaterial.h"
#include"../../Utility/JMathHelper.h" 
#include"../../../Lib/DirectX/d3dx12.h"

//Debug
#include"../../Core/File/JFileIOHelper.h"
using namespace DirectX;
using namespace Microsoft::WRL;

namespace JinEngine
{
	namespace Graphic
	{
		void JOcclusionCulling::Initialize(ID3D12Device* d3dDevice, const JGraphicInfo& info)
		{
			BuildRootSignature(d3dDevice, info.occlusionMapCapacity);
			BuildUploadBuffer(d3dDevice, info.upObjCapacity);

			JDepthMapInfoConstants constants;
			constants.maxWidth = info.occlusionWidth;
			constants.maxHeight = info.occlusionHeight;
			constants.validMapCount = info.occlusionMapCount - 1;

			depthMapInfoCB->CopyData(0, constants);
		}
		void JOcclusionCulling::Clear()
		{
			camera.Clear();
			if (queryResult != nullptr)
				queryResultBuffer->Resource()->Unmap(0, nullptr);
			queryResult = nullptr;
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
		bool JOcclusionCulling::IsCulled(const uint objIndex)const noexcept
		{
			return queryResult != nullptr ? queryResult[objIndex] : false;
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
		void JOcclusionCulling::UpdateOcclusionMapInfo(const JGraphicInfo& info)
		{
			JDepthMapInfoConstants constants;
			constants.maxWidth = info.occlusionWidth;
			constants.maxHeight = info.occlusionHeight;
			constants.validMapCount = info.occlusionMapCount;

			depthMapInfoCB->CopyData(0, constants);

			JShader* shader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER);
			shader->CompileInterface()->RecompileGraphicShader();
		}
		void JOcclusionCulling::UpdateObjectCapacity(ID3D12Device* device, const uint objectCapacity)
		{
			objectBuffer->Clear();
			objectBuffer->Build(device, objectCapacity);

			queryOutBuffer->Clear();
			queryOutBuffer->Build(device, objectCapacity);

			queryResultBuffer->Clear();
			queryResultBuffer->Build(device, objectCapacity);
			 
			JShader* shader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER);
			shader->CompileInterface()->RecompileGraphicShader();
		}
		void JOcclusionCulling::UpdateObject(JRenderItem* rItem, const uint submeshIndex, const uint buffIndex)
		{
			//DirectX::BoundingSphere bSphere = rItem->GetMesh()->GetBoundingSphere();
			objectConstants.queryResultIndex = buffIndex;
			objectConstants.center = rItem->GetMesh()->GetBoundingSphereCenter();
			objectConstants.radius = rItem->GetBoundingSphere().Radius;
			objectConstants.isValid = rItem->GetRenderLayer() == J_RENDER_LAYER::OPAQUE_OBJECT;
			XMStoreFloat4x4(&objectConstants.objWorld, XMMatrixTranspose(rItem->GetOwner()->GetTransform()->GetWorld()));

			objectBuffer->CopyData(buffIndex, objectConstants); 
		}
		void JOcclusionCulling::UpdatePass(JScene* scene, const uint queryCount, const uint cbIndex)
		{
			JCamera* mainCam = scene->GetMainCamera(); 
			JOcclusionPassConstants passConstatns;
			XMStoreFloat4x4(&passConstatns.viewProj, XMMatrixTranspose(XMMatrixMultiply(mainCam->GetView(), mainCam->GetProj())));
			XMStoreFloat4x4(&passConstatns.camWorld, XMMatrixTranspose(mainCam->GetTransform()->GetWorld()));
			passConstatns.camPos = mainCam->GetTransform()->GetPosition();
			passConstatns.validQueryCount = queryCount;
			 
			occlusionPassCB->CopyData(cbIndex, passConstatns);
		}
		void JOcclusionCulling::DepthMapDownSampling(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE downsamplingStartHandle)
		{
			commandList->SetComputeRootSignature(mRootSignature.Get());
			commandList->SetComputeRootDescriptorTable(0, depthMapHandle);
			commandList->SetComputeRootDescriptorTable(1, downsamplingStartHandle);
			commandList->SetComputeRootConstantBufferView(4, depthMapInfoCB->Resource()->GetGPUVirtualAddress());

			JShader* shader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER);
			commandList->SetPipelineState(shader->GetComputePso());

			JVector3<uint> groupDim = shader->GetComputeGroupDim();
			commandList->Dispatch(groupDim.x, groupDim.y, groupDim.z);
		}
		void JOcclusionCulling::OcclusuinCulling(ID3D12GraphicsCommandList* commandList)
		{
			commandList->SetComputeRootShaderResourceView(2, objectBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootUnorderedAccessView(3, queryOutBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootConstantBufferView(5, occlusionPassCB->Resource()->GetGPUVirtualAddress());

			JShader* shader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER);
			commandList->SetPipelineState(shader->GetComputePso());

			JVector3<uint> groupDim = shader->GetComputeGroupDim();
			commandList->Dispatch(groupDim.x, groupDim.y, groupDim.z);

			CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(queryOutBuffer->Resource(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
			commandList->ResourceBarrier(1, &rsBarrier);

			commandList->CopyResource(queryResultBuffer->Resource(), queryOutBuffer->Resource());

			rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(queryOutBuffer->Resource(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
			commandList->ResourceBarrier(1, &rsBarrier);
			isQueryUpdated = true;
		}
		void JOcclusionCulling::ReadCullingResult()
		{
			if (!isQueryUpdated)
				return;

			ThrowIfFailedHr(queryResultBuffer->Resource()->Map(0, nullptr, reinterpret_cast<void**>(&queryResult)));
			
			static int count = 0;
			if (count < 2)
			{
				std::wofstream stream;
				stream.open(L"D:\\JinWooJung\\HZB_OBJ_DEPTH.txt", std::ios::app | std::ios::out);
				if (stream.is_open())
				{
					JFileIOHelper::StoreJString(stream, L"OBJ_DEPTH", L"");
					for (uint i = 0; i < queryResultBuffer->ElementCount(); ++i)
						JFileIOHelper::StoreAtomicData(stream, std::to_wstring(i) + L" Depth: ", queryResult[i]);
					JFileIOHelper::InputSpace(stream, 1);
				}
				stream.close();
				++count;
			}
			//for (uint i = 0; i < queryResultBuffer->ElementCount(); ++i)
			//{
			//	MessageBox(0, std::to_wstring(queryResult[i]).c_str(), std::to_wstring(i).c_str(), 0);
			//}
			//queryResult = nullptr;
			//queryResultBuffer->Resource()->Unmap(0, nullptr);
			isQueryUpdated = false;
		}
		void JOcclusionCulling::BuildRootSignature(ID3D12Device* d3dDevice, const uint occlusionDsvCapacity)
		{
			static constexpr int slotCount = 6;
			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

			CD3DX12_DESCRIPTOR_RANGE texTable00;
			texTable00.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			CD3DX12_DESCRIPTOR_RANGE texTable01;
			texTable01.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, occlusionDsvCapacity, 0, 1);

			// Create root CBV. 
			slotRootParameter[0].InitAsDescriptorTable(1, &texTable00);
			slotRootParameter[1].InitAsDescriptorTable(1, &texTable01);
			slotRootParameter[2].InitAsShaderResourceView(1);
			slotRootParameter[3].InitAsUnorderedAccessView(1);
			slotRootParameter[4].InitAsConstantBufferView(0);
			slotRootParameter[5].InitAsConstantBufferView(1);

			CD3DX12_STATIC_SAMPLER_DESC samDesc(0,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP);
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
			objectBuffer = std::make_unique<JUploadBuffer<JOcclusionObjectConstants>>(J_UPLOAD_BUFFER_TYPE::COMMON);
			queryOutBuffer = std::make_unique<JUploadBuffer<float>>(J_UPLOAD_BUFFER_TYPE::UNORDERED_ACCEESS);
			queryResultBuffer = std::make_unique<JUploadBuffer<float>>(J_UPLOAD_BUFFER_TYPE::READ_BACK);
			depthMapInfoCB = std::make_unique<JUploadBuffer<JDepthMapInfoConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			occlusionPassCB = std::make_unique<JUploadBuffer<JOcclusionPassConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			 
			objectBuffer->Build(device, objectCapacity);
			queryOutBuffer->Build(device, objectCapacity);
			queryResultBuffer->Build(device, objectCapacity);
			depthMapInfoCB->Build(device, 1);
			occlusionPassCB->Build(device, 1);
		}
	}
}