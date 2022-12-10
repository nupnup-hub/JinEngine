#include"JHZBOccCulling.h" 
#include"../JGraphicInfo.h"
#include"../JGraphicOption.h"
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
//#include"../../Core/File/JFileIOHelper.h"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace JinEngine
{
	namespace Graphic
	{
		void JHZBOccCulling::Initialize(ID3D12Device* d3dDevice, const JGraphicInfo& info)
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

				depthMapInfoCB->CopyData(i, constants);
				nowWidth /= 2;
				nowHeight /= 2;
			}
		}
		void JHZBOccCulling::Clear()
		{  
			if (queryResult != nullptr)
				queryResultBuffer->Resource()->Unmap(0, nullptr);

			objectBuffer.reset();
			queryOutBuffer.reset();
			queryResultBuffer.reset();
			depthMapInfoCB.reset();
			occlusionPassCB.reset();
			mRootSignature.Reset();
			queryResult = nullptr;

			//Debug
			//debugBuffer.reset();
		}
		ID3D12RootSignature* JHZBOccCulling::GetRootSignature()const noexcept
		{
			return mRootSignature.Get();
		}
		bool JHZBOccCulling::IsCulled(const uint objIndex)const noexcept
		{
			return queryResult != nullptr ? queryResult[objIndex] : false;
		}
		void JHZBOccCulling::UpdateOcclusionMapInfo(ID3D12Device* device, const JGraphicInfo& info)
		{
			uint nowWidth = info.occlusionWidth;
			uint nowHeight = info.occlusionHeight;
			for (uint i = 0; i < info.occlusionMapCount; ++i)
			{
				JDepthMapInfoConstants constants;
				constants.nowWidth = nowWidth;
				constants.nowHeight = nowHeight;
				constants.nowIndex = i;

				depthMapInfoCB->CopyData(i, constants);
				nowWidth /= 2;
				nowHeight /= 2;
			}

			JShader* shader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER);
			shader->CompileInterface()->RecompileGraphicShader();
		}
		void JHZBOccCulling::UpdateObjectCapacity(ID3D12Device* device, const uint objectCapacity)
		{
			objectBuffer->Clear();
			objectBuffer->Build(device, objectCapacity);

			queryOutBuffer->Clear();
			queryOutBuffer->Build(device, objectCapacity);

			queryResultBuffer->Clear();
			queryResultBuffer->Build(device, objectCapacity);

			//Debug
			//debugBuffer->Clear();
			//debugBuffer->Build(device, objectCapacity);

			JShader* shader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER);
			shader->CompileInterface()->RecompileGraphicShader();
		}
		void JHZBOccCulling::UpdateObject(JRenderItem* rItem, const uint submeshIndex, const uint buffIndex)
		{
			const DirectX::BoundingBox bbox = rItem->GetMesh()->GetBoundingBox();
			objectConstants.center = bbox.Center;
			objectConstants.extents = bbox.Extents;
			objectConstants.isValid = rItem->GetRenderLayer() == J_RENDER_LAYER::OPAQUE_OBJECT;
			objectConstants.queryResultIndex = buffIndex;
			XMStoreFloat4x4(&objectConstants.objWorld, XMMatrixTranspose(rItem->GetOwner()->GetTransform()->GetWorld()));

			objectBuffer->CopyData(buffIndex, objectConstants);
		}
		void JHZBOccCulling::UpdatePass(JScene* scene, const JGraphicInfo& info, const JGraphicOption& option, const uint queryCount, const uint cbIndex)
		{
			JCamera* mainCam = scene->GetMainCamera();
			JOcclusionPassConstants passConstatns;

			//static const BoundingBox drawBBox = JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE)->GetBoundingBox();
			static const BoundingBox drawBBox = JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE)->GetBoundingBox();
			XMStoreFloat4x4(&passConstatns.view, XMMatrixTranspose(mainCam->GetView()));
			XMStoreFloat4x4(&passConstatns.proj, XMMatrixTranspose(mainCam->GetProj()));
			const BoundingFrustum frustum = mainCam->GetBoundingFrustum();
			XMVECTOR planeV[6];
			frustum.GetPlanes(&planeV[0], &planeV[1], &planeV[2], &planeV[3], &planeV[4], &planeV[5]);
			for (uint i = 0; i < 6; ++i)
				XMStoreFloat4(&passConstatns.frustumPlane[i], planeV[i]);

			passConstatns.viewWidth = mainCam->GetViewWidth();
			passConstatns.viewHeight = mainCam->GetViewHeight();
			passConstatns.camNear = mainCam->GetNear();
			passConstatns.camFar = mainCam->GetFar();
			passConstatns.validQueryCount = queryCount;
			passConstatns.occMapCount = info.occlusionMapCount; 
			passConstatns.occIndexOffset = JMathHelper::Log2Int(info.occlusionMinSize);
			passConstatns.correctFailTrigger = (int)option.allowHZBCorrectFail;
			occlusionPassCB->CopyData(cbIndex, passConstatns);
		} 
		void JHZBOccCulling::DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapSrvHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapUavHandle,
			const uint samplingCount,
			const uint srvDescriptorSize)
		{
			const uint depthMapInfoCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JDepthMapInfoConstants));
			commandList->SetComputeRootSignature(mRootSignature.Get());

			JShader* copyShader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_COPY_SHADER);
			commandList->SetPipelineState(copyShader->GetComputePso());

			commandList->SetComputeRootDescriptorTable(0, depthMapSrvHandle);
			commandList->SetComputeRootDescriptorTable(1, mipMapUavHandle);
			commandList->SetComputeRootConstantBufferView(5, depthMapInfoCB->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootConstantBufferView(6, occlusionPassCB->Resource()->GetGPUVirtualAddress());

			JVector3<uint> cgroupDim = copyShader->GetComputeGroupDim();
			commandList->Dispatch(cgroupDim.x, cgroupDim.y, cgroupDim.z);

			JShader* downSampleShader = JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER);
			commandList->SetPipelineState(downSampleShader->GetComputePso());

			const uint loopCount = samplingCount - 1;
			for (uint i = 0; i < loopCount; ++i)
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = mipMapSrvHandle;
				srcHandle.Offset(i, srvDescriptorSize);

				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = mipMapUavHandle;
				destHandle.Offset(i + 1, srvDescriptorSize);

				commandList->SetComputeRootDescriptorTable(0, srcHandle);
				commandList->SetComputeRootDescriptorTable(1, destHandle);

				D3D12_GPU_VIRTUAL_ADDRESS depthMapCBAddress = depthMapInfoCB->Resource()->GetGPUVirtualAddress() + i * depthMapInfoCBByteSize;
				commandList->SetComputeRootConstantBufferView(5, depthMapCBAddress);
				JVector3<uint> dgroupDim = downSampleShader->GetComputeGroupDim();
				commandList->Dispatch(dgroupDim.x, dgroupDim.y, dgroupDim.z);
			}
		}
		void JHZBOccCulling::OcclusuinCulling(ID3D12GraphicsCommandList* commandList, CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle)
		{
			commandList->SetComputeRootDescriptorTable(2, mipMapStHandle);
			commandList->SetComputeRootShaderResourceView(3, objectBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootUnorderedAccessView(4, queryOutBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootConstantBufferView(6, occlusionPassCB->Resource()->GetGPUVirtualAddress());

			//Debug
			//debugBuffer->SetComputeShader(commandList);

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
			SetReadResultTrigger(true);
		}
		void JHZBOccCulling::ReadCullingResult()
		{
			if (!CanReadOcclusionResult())
				return;

			ThrowIfFailedHr(queryResultBuffer->Resource()->Map(0, nullptr, reinterpret_cast<void**>(&queryResult)));
			SetReadResultTrigger(false);

			//Debug
			/*
			static int streamC = 2;
			if (streamC < 1)
			{
				std::wofstream stream;
				stream.open(L"D:\\JinWooJung\\HZB_Occlusion_debug_bbox5.txt", std::ios::app | std::ios::out);
				if (stream.is_open())
				{
					int count = 0;
					HZBDebugInfo* info = debugBuffer->Map(count);
					for (uint i = 0; i < count; ++i)
					{
						JFileIOHelper::StoreXMFloat4x4(stream, L"ObjWorld", info[i].objWorld);

						JFileIOHelper::StoreAtomicData(stream, L"CenterDepth", info[i].centerDepth);
						JFileIOHelper::StoreAtomicData(stream, L"CopareDepth", info[i].copareDepth);

						JFileIOHelper::StoreAtomicData(stream, L"ThreadIndex", info[i].threadIndex);
						JFileIOHelper::StoreAtomicData(stream, L"QueryIdex", info[i].queryIndex);
						JFileIOHelper::InputSpace(stream, 1);

							JFileIOHelper::StoreXMFloat4x4(stream, L"ObjWorld", info[i].objWorld);

						JFileIOHelper::StoreXMFloat3(stream, L"Center", info[i].center);
						JFileIOHelper::StoreXMFloat3(stream, L"Extents", info[i].extents);

						JFileIOHelper::StoreXMFloat4(stream, L"PosCW", info[i].posCW);
						JFileIOHelper::StoreXMFloat4(stream, L"PosCV", info[i].posCV);
						JFileIOHelper::StoreXMFloat4(stream, L"PosEW", info[i].posEW);
						JFileIOHelper::StoreXMFloat4(stream, L"PosEV", info[i].posEV);

						JFileIOHelper::StoreXMFloat3(stream, L"nearPoint0", info[i].nearPoint0);
						JFileIOHelper::StoreXMFloat3(stream, L"nearPoint1", info[i].nearPoint1);
						JFileIOHelper::StoreXMFloat3(stream, L"nearPoint2", info[i].nearPoint2);
						JFileIOHelper::StoreXMFloat3(stream, L"nearPoint3", info[i].nearPoint3);
						JFileIOHelper::StoreXMFloat3(stream, L"nearPoint4", info[i].nearPoint4);
						JFileIOHelper::StoreXMFloat3(stream, L"nearPoint5", info[i].nearPoint5);

						JFileIOHelper::StoreXMFloat3(stream, L"nearPointW", info[i].nearPointW);
						JFileIOHelper::StoreXMFloat4(stream, L"nearPointH", info[i].nearPointH);
						JFileIOHelper::StoreXMFloat3(stream, L"nearPointC", info[i].nearPointC);

						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV00", info[i].bboxPointV0);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV01", info[i].bboxPointV1);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV02", info[i].bboxPointV2);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV03", info[i].bboxPointV3);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV04", info[i].bboxPointV4);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV05", info[i].bboxPointV5);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV06", info[i].bboxPointV6);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointV07", info[i].bboxPointV7);

						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH00", info[i].bboxPointH0);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH01", info[i].bboxPointH1);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH02", info[i].bboxPointH2);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH03", info[i].bboxPointH3);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH04", info[i].bboxPointH4);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH05", info[i].bboxPointH5);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH06", info[i].bboxPointH6);
						JFileIOHelper::StoreXMFloat4(stream, L"bboxPointH07", info[i].bboxPointH7);

						JFileIOHelper::StoreXMFloat2(stream, L"clipFrame0", info[i].clipFrame0);
						JFileIOHelper::StoreXMFloat2(stream, L"clipFrame1", info[i].clipFrame1);
						JFileIOHelper::StoreXMFloat2(stream, L"clipFrame2", info[i].clipFrame2);
						JFileIOHelper::StoreXMFloat2(stream, L"clipFrame3", info[i].clipFrame3);

						JFileIOHelper::StoreAtomicData(stream, L"Width", info[i].width);
						JFileIOHelper::StoreAtomicData(stream, L"Height", info[i].height);
						JFileIOHelper::StoreAtomicData(stream, L"Lod", info[i].lod);

						JFileIOHelper::StoreXMFloat3(stream, L"uvExtentsMax", info[i].uvExtentsMax);
						JFileIOHelper::StoreXMFloat3(stream, L"uvExtentsMin", info[i].uvExtentsMin);

						JFileIOHelper::StoreAtomicData(stream, L"CenterDepth", info[i].centerDepth);
						JFileIOHelper::StoreAtomicData(stream, L"CopareDepth", info[i].copareDepth);

						JFileIOHelper::StoreAtomicData(stream, L"ThreadIndex", info[i].threadIndex);
						JFileIOHelper::StoreAtomicData(stream, L"QueryIdex", info[i].queryIndex);

						JFileIOHelper::InputSpace(stream, 1);
						
					}
					++streamC;
					stream.close();
				}
			}*/
		}
		void JHZBOccCulling::BuildRootSignature(ID3D12Device* d3dDevice, const uint occlusionDsvCapacity)
		{
			//Debug
			//static constexpr int slotCount = 8;
			static constexpr int slotCount = 7;

			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

			CD3DX12_DESCRIPTOR_RANGE depthMapTable;
			depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			CD3DX12_DESCRIPTOR_RANGE lastMipmapTable;
			lastMipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
			CD3DX12_DESCRIPTOR_RANGE mipmapTable;
			mipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

			// Create root CBV. 
			slotRootParameter[0].InitAsDescriptorTable(1, &depthMapTable);
			slotRootParameter[1].InitAsDescriptorTable(1, &lastMipmapTable);
			slotRootParameter[2].InitAsDescriptorTable(1, &mipmapTable);

			slotRootParameter[3].InitAsShaderResourceView(2);
			slotRootParameter[4].InitAsUnorderedAccessView(1, 1);
			slotRootParameter[5].InitAsConstantBufferView(0);
			slotRootParameter[6].InitAsConstantBufferView(1);

			//Debug
			//slotRootParameter[7].InitAsUnorderedAccessView(2, 1);

			std::vector< CD3DX12_STATIC_SAMPLER_DESC> samDesc
			{
				CD3DX12_STATIC_SAMPLER_DESC(0,
				D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
				0.0f,                               // mipLODBias
				1,                                 // maxAnisotropy
				D3D12_COMPARISON_FUNC_LESS_EQUAL,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),

				CD3DX12_STATIC_SAMPLER_DESC(1,
				D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
				0.0f,                               // mipLODBias
				1,                                 // maxAnisotropy
				D3D12_COMPARISON_FUNC_NEVER,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
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
		void JHZBOccCulling::BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity, const uint occlusionCapacity)
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
			//debugBuffer = std::make_unique<JHlslDebug<HZBDebugInfo>>(7);
			//debugBuffer->Build(device, objectCapacity);
		}
	}
}