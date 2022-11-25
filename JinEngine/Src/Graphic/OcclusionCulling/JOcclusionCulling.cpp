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
			BuildUploadBuffer(d3dDevice, info.upObjCapacity, info.occlusionMapCapacity);

			uint nowWidth = info.occlusionWidth;
			uint nowHeight = info.occlusionHeight;
			for (uint i = 0; i < info.occlusionMapCount; ++i)
			{
				JDepthMapInfoConstants constants;
				constants.nowWidth = nowWidth;
				constants.nowHeight = nowHeight;
				constants.nowIndex = i;

				nowWidth /= 2;
				nowHeight /= 2;
				depthMapInfoCB->CopyData(i, constants);
			}
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
		void JOcclusionCulling::UpdateOcclusionMapInfo(ID3D12Device* device, const JGraphicInfo& info)
		{
			uint nowWidth = info.occlusionWidth;
			uint nowHeight = info.occlusionHeight;
			for (uint i = 0; i < info.occlusionMapCount; ++i)
			{
				JDepthMapInfoConstants constants;
				constants.nowWidth = nowWidth;
				constants.nowHeight = nowHeight;
				constants.nowIndex = i;

				nowWidth /= 2;
				nowHeight /= 2;
				depthMapInfoCB->CopyData(i, constants);
			}

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

			//Debug
			debugBuffer->Clear();
			debugBuffer->Build(device, objectCapacity);

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
		void JOcclusionCulling::DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapUavHandle,
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& depthResource,
			const uint samplingCount,
			const uint srvDescriptorSize)
		{
			commandList->SetComputeRootSignature(mRootSignature.Get());

			JShader* shader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER);
			commandList->SetPipelineState(shader->GetComputePso());

			const uint depthMapInfoCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JDepthMapInfoConstants));
			for (uint i = 0; i < samplingCount; ++i)
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle = depthMapSrvHandle;
				depthMapHandle.Offset(i, srvDescriptorSize);

				CD3DX12_GPU_DESCRIPTOR_HANDLE mipmapHandle = depthMapUavHandle;
				mipmapHandle.Offset(i, srvDescriptorSize);

				commandList->SetComputeRootDescriptorTable(0, depthMapHandle);
				commandList->SetComputeRootDescriptorTable(1, mipmapHandle);

				D3D12_GPU_VIRTUAL_ADDRESS depthMapCBAddress = depthMapInfoCB->Resource()->GetGPUVirtualAddress() + i * depthMapInfoCBByteSize;
				commandList->SetComputeRootConstantBufferView(5, depthMapCBAddress);
				JVector3<uint> groupDim = shader->GetComputeGroupDim();
				commandList->Dispatch(groupDim.x, groupDim.y, groupDim.z);
			}
		}
		void JOcclusionCulling::OcclusuinCulling(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle)
		{
			commandList->SetComputeRootDescriptorTable(2, depthMapSrvHandle);
			commandList->SetComputeRootShaderResourceView(3, objectBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootUnorderedAccessView(4, queryOutBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootConstantBufferView(6, occlusionPassCB->Resource()->GetGPUVirtualAddress());

			//Debug
			debugBuffer->SetComputeShader(commandList);

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
			//queryResult = nullptr;
			//queryResultBuffer->Resource()->Unmap(0, nullptr);
			isQueryUpdated = false;

			static int streamC = 0;
			if (streamC < 1)
			{
				std::wofstream stream;
				stream.open(L"D:\\JinWooJung\\HZB_Occlusion_debug.txt", std::ios::app | std::ios::out);
				if (stream.is_open())
				{
					std::vector<std::wstring> name
					{
						L"0 defualt sky",
						L"1 cube",
						L"2 cube(0)",
						L"3 cube(1)",
						L"4 cyilinder",
						L"5 cube(2)",
						L"5 cube(3)"
					};

					int count = 0;
					HZBDebugInfo* info = debugBuffer->Map(count);
					for (uint i = 0; i < name.size(); ++i)
					{
						JFileIOHelper::StoreJString(stream, L"Name:", name[i]);
						JFileIOHelper::StoreXMFloat4x4(stream, L"ObjWorld", info[i].objWorld);
						JFileIOHelper::StoreXMFloat4x4(stream, L"CamWorld", info[i].camWorld);
						JFileIOHelper::StoreXMFloat4(stream, L"PosW", info[i].posW);
						JFileIOHelper::StoreXMFloat4(stream, L"CamW", info[i].camW);
						JFileIOHelper::StoreXMFloat4(stream, L"PosC", info[i].posC);
						JFileIOHelper::StoreXMFloat4(stream, L"PosH", info[i].posH);
						JFileIOHelper::StoreXMFloat3(stream, L"PosS", info[i].posS);
						JFileIOHelper::StoreAtomicData(stream, L"Radius", info[i].radius);
						JFileIOHelper::StoreXMFloat4(stream, L"AdditionalPosW", info[i].additionalPosW);
						JFileIOHelper::StoreXMFloat2(stream, L"TexSize", info[i].texSize);
						JFileIOHelper::StoreAtomicData(stream, L"CenterDepth", info[i].centerDepth);
						JFileIOHelper::StoreAtomicData(stream, L"CopareDepth", info[i].copareDepth);
						JFileIOHelper::StoreAtomicData(stream, L"Lod", info[i].lod);
						JFileIOHelper::InputSpace(stream, 1);
					}
					++streamC;
					stream.close();
				}
			}
		}
		void JOcclusionCulling::BuildRootSignature(ID3D12Device* d3dDevice, const uint occlusionDsvCapacity)
		{
			static constexpr int slotCount = 8;
			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

			CD3DX12_DESCRIPTOR_RANGE depthMapTable;
			depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			CD3DX12_DESCRIPTOR_RANGE lastMipmapTable;
			lastMipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
			CD3DX12_DESCRIPTOR_RANGE mipmapTable;
			mipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, occlusionDsvCapacity, 1, 1);

			// Create root CBV. 
			slotRootParameter[0].InitAsDescriptorTable(1, &depthMapTable);
			slotRootParameter[1].InitAsDescriptorTable(1, &lastMipmapTable);
			slotRootParameter[2].InitAsDescriptorTable(1, &mipmapTable);

			slotRootParameter[3].InitAsShaderResourceView(2);
			slotRootParameter[4].InitAsUnorderedAccessView(1, 1);
			slotRootParameter[5].InitAsConstantBufferView(0);
			slotRootParameter[6].InitAsConstantBufferView(1);

			//Debug
			slotRootParameter[7].InitAsUnorderedAccessView(2, 1);

			std::vector< CD3DX12_STATIC_SAMPLER_DESC> samDesc
			{
				CD3DX12_STATIC_SAMPLER_DESC(0,
				D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
				0.0f,                               // mipLODBias
				16.0f,                                 // maxAnisotropy
				D3D12_COMPARISON_FUNC_LESS_EQUAL,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),

				CD3DX12_STATIC_SAMPLER_DESC(1,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
				0.0f,                               // mipLODBias
				16.0f,                                 // maxAnisotropy
				D3D12_COMPARISON_FUNC_LESS_EQUAL,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE)
			};
			 
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, (uint)samDesc.size(), samDesc.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
		void JOcclusionCulling::BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity, const uint occlusionCapacity)
		{
			objectBuffer = std::make_unique<JUploadBuffer<JOcclusionObjectConstants>>(J_UPLOAD_BUFFER_TYPE::COMMON);
			queryOutBuffer = std::make_unique<JUploadBuffer<float>>(J_UPLOAD_BUFFER_TYPE::UNORDERED_ACCEESS);
			queryResultBuffer = std::make_unique<JUploadBuffer<float>>(J_UPLOAD_BUFFER_TYPE::READ_BACK);
			depthMapInfoCB = std::make_unique<JUploadBuffer<JDepthMapInfoConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			occlusionPassCB = std::make_unique<JUploadBuffer<JOcclusionPassConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);

			objectBuffer->Build(device, objectCapacity);
			queryOutBuffer->Build(device, objectCapacity);
			queryResultBuffer->Build(device, objectCapacity);
			depthMapInfoCB->Build(device, occlusionCapacity);
			occlusionPassCB->Build(device, 1);

			//Debug
			debugBuffer = std::make_unique<JHlslDebug<HZBDebugInfo>>(7);
			debugBuffer->Build(device, objectCapacity);
		}
	}
}