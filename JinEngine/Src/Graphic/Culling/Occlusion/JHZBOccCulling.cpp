#include"JHZBOccCulling.h" 
#include"../JCullingInfo.h"
#include"../JCullingInterface.h"
#include"../JCullingConstants.h"
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h"
#include"../../Upload/FrameResource/JFrameResource.h"
#include"../../Upload/FrameResource/JOcclusionConstants.h"
#include"../../Upload/FrameResource/JBoundingObjectConstants.h"
#include"../../../Core/Exception/JExceptionMacro.h" 
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Object/GameObject/JGameObject.h"  
#include"../../../Object/Component/Transform/JTransform.h"
#include"../../../Object/Resource/JResourceManager.h" 
#include"../../../Object/Resource/Shader/JShader.h"
#include"../../../Object/Resource/Shader/JShaderPrivate.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../Object/Resource/Material/JMaterial.h"
#include"../../../Utility/JMathHelper.h" 
#include"../../../Lib/DirectX/Tk/Src/d3dx12.h"

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
				JHzbOccDepthMapInfoConstants constants;
				constants.nowWidth = nowWidth;
				constants.nowHeight = nowHeight;
				constants.nowIndex = i;

				occDepthMapInfoCB->CopyData(i, constants);
				nowWidth /= 2;
				nowHeight /= 2;
			}
		}
		bool JHZBOccCulling::BuildOccBuffer(ID3D12Device* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo)
		{ 
			if (cullingInfo->GetCullingType() != J_CULLING_TYPE::OCCLUSION)
				return false;

			auto resultBuff = std::make_unique<JUploadBuffer<float>>(L"HzbOccResult", J_UPLOAD_BUFFER_TYPE::READ_BACK);
			resultBuff->Build(device, initCapacity);
			 
			//Debug
			//newUser->debugBuffer = std::make_unique<JHlslDebug<HZBDebugInfo>>(7);
			//newUser->debugBuffer->Build(device, objectCapacity);

			BYTE* resultPtr = resultBuff->Pointer();
			const size_t buffSize = resultBuff->ElementCount();
			cullingInfo->SetResultPtr(resultPtr, buffSize); 

			occResultBuffer.push_back(std::move(resultBuff));
			return true;
		}
		void JHZBOccCulling::ReBuildOccBuffer(ID3D12Device* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo)
		{
			for (const auto& data : cullingInfo)
			{
				uint arrayIndex = data->GetArrayIndex();
				occResultBuffer[arrayIndex]->Clear();
				occResultBuffer[arrayIndex]->Build(device, capacity);

				BYTE* resultPtr = occResultBuffer[arrayIndex]->Pointer();
				const size_t buffSize = occResultBuffer[arrayIndex]->ElementCount();

				data->SetResultPtr(resultPtr, buffSize);
			}
		}
		void JHZBOccCulling::DestroyOccBuffer(JCullingInfo* cullignInfo)
		{ 
			occResultBuffer.erase(occResultBuffer.begin() + cullignInfo->GetArrayIndex());
			cullignInfo->SetResultPtr(nullptr, 0);
		} 
		void JHZBOccCulling::Clear()
		{   
			mRootSignature.Reset();
			occQueryOutBuffer.reset();
			occResultBuffer.clear(); 
		}
		ID3D12RootSignature* JHZBOccCulling::GetRootSignature()const noexcept
		{
			return mRootSignature.Get();
		}
		void JHZBOccCulling::ReBuildOcclusionMapInfoConstants(ID3D12Device* device, const JGraphicInfo& info)
		{
			uint nowWidth = info.occlusionWidth;
			uint nowHeight = info.occlusionHeight;
			for (uint i = 0; i < info.occlusionMapCount; ++i)
			{
				JHzbOccDepthMapInfoConstants constants;
				constants.nowWidth = nowWidth;
				constants.nowHeight = nowHeight;
				constants.nowIndex = i;

				occDepthMapInfoCB->CopyData(i, constants);
				nowWidth /= 2;
				nowHeight /= 2;
			}

			//기본값으로 설정된 shader는 app이 실행될때 default shader에있는 정보를 참조해서 자동으로 로드됨 
			JShader* shader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER).Get();
			JShaderPrivate::CompileInterface::RecompileGraphicShader(shader);
		}
		void JHZBOccCulling::ReBuildObjectConstants(ID3D12Device* device, const uint objectCapacity)
		{
			occQueryOutBuffer->Clear();
			occQueryOutBuffer->Build(device, objectCapacity);

			JShader* shader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER).Get();
			JShaderPrivate::CompileInterface::RecompileComputeShader(shader);
		}
		void JHZBOccCulling::DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
			JFrameResource* currFrame,
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapSrvHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapUavHandle,
			const uint samplingCount,
			const uint srvDescriptorSize,
			const uint passCBIndex)
		{
			const uint depthMapInfoCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccDepthMapInfoConstants));
			commandList->SetComputeRootSignature(mRootSignature.Get());

			JShader* copyShader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_COPY_SHADER).Get();
			commandList->SetPipelineState(copyShader->GetComputePso());

			uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccPassConstants));
			D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = currFrame->hzbOccPassCB->Resource()->GetGPUVirtualAddress() + passCBIndex * passCBByteSize;

			commandList->SetComputeRootDescriptorTable(0, depthMapSrvHandle);
			commandList->SetComputeRootDescriptorTable(2, mipMapUavHandle);
			commandList->SetComputeRootConstantBufferView(5, occDepthMapInfoCB->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootConstantBufferView(6, passCBAddress);

			JVector3<uint> cgroupDim = copyShader->GetComputeGroupDim();
			commandList->Dispatch(cgroupDim.x, cgroupDim.y, cgroupDim.z);

			JShader* downSampleShader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_DOWNSAMPLING_SHADER).Get();
			commandList->SetPipelineState(downSampleShader->GetComputePso());

			const uint loopCount = samplingCount - 1;
			for (uint i = 0; i < loopCount; ++i)
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = mipMapSrvHandle;
				srcHandle.Offset(i, srvDescriptorSize);

				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = mipMapUavHandle;
				destHandle.Offset(i + 1, srvDescriptorSize);

				commandList->SetComputeRootDescriptorTable(1, srcHandle);
				commandList->SetComputeRootDescriptorTable(2, destHandle);

				D3D12_GPU_VIRTUAL_ADDRESS depthMapCBAddress = occDepthMapInfoCB->Resource()->GetGPUVirtualAddress() + i * depthMapInfoCBByteSize;
				commandList->SetComputeRootConstantBufferView(5, depthMapCBAddress);
				JVector3<uint> dgroupDim = downSampleShader->GetComputeGroupDim();
				commandList->Dispatch(dgroupDim.x, dgroupDim.y, dgroupDim.z);
			}
		}
		void JHZBOccCulling::OcclusionCulling(ID3D12GraphicsCommandList* commandList,
			JFrameResource* currFrame,
			CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle,
			const uint passCBIndex,
			const JCullingUserInterface& cullUser)
		{
			uint passCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JHzbOccPassConstants));
			D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = currFrame->hzbOccPassCB->Resource()->GetGPUVirtualAddress() + passCBIndex * passCBByteSize;
			 
			commandList->SetComputeRootDescriptorTable(1, mipMapStHandle);
			commandList->SetComputeRootShaderResourceView(3, currFrame->hzbOccObjectBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootUnorderedAccessView(4, occQueryOutBuffer->Resource()->GetGPUVirtualAddress());
			commandList->SetComputeRootConstantBufferView(6, passCBAddress);

			//Debug
			//occResultBuffer[cullingInfo->GetArrayIndex()]->debugBuffer->SetComputeShader(commandList);

			JShader* shader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_COMPUTE_SHADER::DEFUALT_HZB_OCCLUSION_SHADER).Get();
			commandList->SetPipelineState(shader->GetComputePso());

			JVector3<uint> groupDim = shader->GetComputeGroupDim();
			commandList->Dispatch(groupDim.x, groupDim.y, groupDim.z);

			CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(occQueryOutBuffer->Resource(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			commandList->ResourceBarrier(1, &rsBarrier);

			commandList->CopyResource(occResultBuffer[cullUser.GetArrayIndex(J_CULLING_TYPE::OCCLUSION)]->Resource(), occQueryOutBuffer->Resource());

			rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(occQueryOutBuffer->Resource(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandList->ResourceBarrier(1, &rsBarrier); 
		}
		void JHZBOccCulling::PrintDebug(const JUserPtr<JCullingInfo>& cullingInfo)
		{
			//Debug 
			/*
			static int streamC = 0;
			if (streamC < 2)
			{
				std::wofstream stream;
				stream.open(L"D:\\JinWooJung\\HZB_Occlusion_debug_bbox.txt", std::ios::app | std::ios::out);
				if (stream.is_open())
				{
					int count = 0;
					HZBDebugInfo* info = debugBuffer->Map(count);
					for (uint i = 0; i < count; ++i)
					{
						JFileIOHelper::StoreXMFloat3(stream, L"Center", info[i].center);
						JFileIOHelper::StoreXMFloat3(stream, L"Extents", info[i].extents);
						JFileIOHelper::StoreAtomicData(stream, L"CullingRes", info[i].cullingRes);
						JFileIOHelper::StoreAtomicData(stream, L"CenterDepth", info[i].centerDepth);
						JFileIOHelper::StoreAtomicData(stream, L"FinalCompareDepth", info[i].finalCompareDepth);

						JFileIOHelper::StoreAtomicData(stream, L"ThreadIndex", info[i].threadIndex);
						JFileIOHelper::StoreAtomicData(stream, L"QueryIdex", info[i].queryIndex);

						JFileIOHelper::InputSpace(stream, 1);

					}
					++streamC;
					stream.close();
				}
			}
			*/
		}
		void JHZBOccCulling::BuildRootSignature(ID3D12Device* d3dDevice, const uint occlusionDsvCapacity)
		{
			//Debug
			//static constexpr int slotCount = 8;
			static constexpr int slotCount = 7;

			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];

			CD3DX12_DESCRIPTOR_RANGE depthMapTable;
			depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
			CD3DX12_DESCRIPTOR_RANGE mipmapTable;
			mipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
			CD3DX12_DESCRIPTOR_RANGE lastMipmapTable;
			lastMipmapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

			// Create root CBV. 
			slotRootParameter[0].InitAsDescriptorTable(1, &depthMapTable);
			slotRootParameter[1].InitAsDescriptorTable(1, &mipmapTable);
			slotRootParameter[2].InitAsDescriptorTable(1, &lastMipmapTable);

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

			// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
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

			mRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("HZB RootSignature") - 1, "HZB RootSignature");
		}
		void JHZBOccCulling::BuildUploadBuffer(ID3D12Device* device, const uint objectCapacity, const uint occlusionMapCapacity)
		{
			occDepthMapInfoCB = std::make_unique<JUploadBuffer<JHzbOccDepthMapInfoConstants>>(L"HzbOccDepthMapInfo", J_UPLOAD_BUFFER_TYPE::CONSTANT);
			occQueryOutBuffer = std::make_unique<JUploadBuffer<float>>(L"HzbOccQueryOut", J_UPLOAD_BUFFER_TYPE::UNORDERED_ACCEESS);

			occDepthMapInfoCB->Build(device, occlusionMapCapacity);
			occQueryOutBuffer->Build(device, objectCapacity);
		}
	}
}