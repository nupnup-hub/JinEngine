#include"JFrameResource.h"
#include"../../JGraphicInfo.h"
#include"../../../Core/Exception/JExceptionMacro.h" 
#include"../../../Utility/JCommonUtility.h"

#include"JObjectConstants.h" 
#include"JAnimationConstants.h"
#include"JMaterialConstants.h"
#include"JPassConstants.h" 
#include"JCameraConstants.h"
#include"JLightConstants.h"
#include"JShadowMapConstants.h"
#include"JBoundingObjectConstants.h"
#include"JOcclusionConstants.h"

namespace JinEngine
{
	namespace Graphic
	{
		JFrameResource::EraseBlockInfo::EraseBlockInfo(const uint reqIndex, const uint dataIndex)
			:reqIndex(reqIndex), dataIndex(dataIndex)
		{}

		JFrameResource::JFrameResource(ID3D12Device* device, const JGraphicInfo& gInfo)
		{   
			for (uint i = 0; i < (uint)J_MAIN_THREAD_CMD_ORDER::COUNT; ++i)
			{
				ThrowIfFailedHr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAlloc[i].GetAddressOf())));
				ThrowIfFailedHr(device->CreateCommandList(0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					cmdListAlloc[i].Get(),
					nullptr,
					IID_PPV_ARGS(cmdList[i].GetAddressOf())));
				cmdList[i]->Close();
			}
			for (uint i = 0; i < Constants::gMaxFrameThread; ++i)
			{				 
				ThrowIfFailedHr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(sceneCmdListAlloc[i].GetAddressOf())));
				ThrowIfFailedHr(device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					sceneCmdListAlloc[i].Get(),
					nullptr,
					IID_PPV_ARGS(sceneCmdList[i].GetAddressOf())));
				sceneCmdList[i]->Close();

				ThrowIfFailedHr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(shadowCmdListAlloc[i].GetAddressOf())));
				ThrowIfFailedHr(device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					shadowCmdListAlloc[i].Get(),
					nullptr,
					IID_PPV_ARGS(shadowCmdList[i].GetAddressOf())));
				shadowCmdList[i]->Close();

				ThrowIfFailedHr(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(hzbOccDrawCmdListAlloc[i].GetAddressOf())));
				ThrowIfFailedHr(device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					hzbOccDrawCmdListAlloc[i].Get(),
					nullptr,
					IID_PPV_ARGS(hzbOccDrawCmdList[i].GetAddressOf())));
				hzbOccDrawCmdList[i]->Close();

				sceneCmdBatch[i] = sceneCmdList[i].Get();
				shadowCmdBatch[i] = shadowCmdList[i].Get();
				hzbOccDrawCmdBatch[i] = hzbOccDrawCmdList[i].Get();
			}
			  
			auto nameLam = [](const J_UPLOAD_FRAME_RESOURCE_TYPE type)
			{
				return JCUtil::StrToWstr(Core::GetName(type));
			};

			materialBuffer = std::make_unique<JUploadBuffer<JMaterialConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL), J_UPLOAD_BUFFER_TYPE::COMMON);
			lightBuffer = std::make_unique<JUploadBuffer<JLightConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT), J_UPLOAD_BUFFER_TYPE::COMMON);
			smLightBuffer = std::make_unique<JUploadBuffer<JShadowMapLightConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT), J_UPLOAD_BUFFER_TYPE::COMMON);
			hzbOccObjectBuffer = std::make_unique<JUploadBuffer<JHzbOccObjectConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT), J_UPLOAD_BUFFER_TYPE::COMMON);

			objectCB = std::make_unique<JUploadBuffer<JObjectConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			skinnedCB = std::make_unique<JUploadBuffer<JAnimationConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			passCB = std::make_unique<JUploadBuffer<JPassConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::PASS), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			cameraCB = std::make_unique<JUploadBuffer<JCameraConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			lightIndexCB = std::make_unique<JUploadBuffer<JLightIndexConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_INDEX), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			shadowMapCalCB = std::make_unique<JUploadBuffer<JShadowMapConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			bundingObjectCB = std::make_unique<JUploadBuffer<JBoundingObjectConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			hzbOccPassCB = std::make_unique<JUploadBuffer<JHzbOccPassConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_PASS), J_UPLOAD_BUFFER_TYPE::CONSTANT);
			 	 
			materialBuffer->Build(device, gInfo.minCapacity);
			lightBuffer->Build(device, gInfo.minCapacity);
			smLightBuffer->Build(device, gInfo.minCapacity);
			hzbOccObjectBuffer->Build(device, gInfo.minCapacity);

			objectCB->Build(device, gInfo.minCapacity);
			skinnedCB->Build(device, gInfo.minCapacity);
			passCB->Build(device, gInfo.minCapacity);
			cameraCB->Build(device, gInfo.minCapacity);
			lightIndexCB->Build(device, gInfo.minCapacity); 
			shadowMapCalCB->Build(device, gInfo.minCapacity);
			bundingObjectCB->Build(device, gInfo.minCapacity);		 
			hzbOccPassCB->Build(device, gInfo.minCapacity);
			 
			std::unordered_map<J_UPLOAD_FRAME_RESOURCE_TYPE, JUploadBufferBase*> uploadBufferMap = 
			{ 
				{J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, materialBuffer.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT, lightBuffer.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT, smLightBuffer.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, objectCB.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, skinnedCB.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::PASS, passCB.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, cameraCB.get()},  
				{J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT_INDEX, lightIndexCB.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP, shadowMapCalCB.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, bundingObjectCB.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_PASS, hzbOccPassCB.get()},
				{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, hzbOccObjectBuffer.get()}
			}; 
			for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				uploadBufferVec[i] = nullptr;

			for (const auto& data : uploadBufferMap)
				uploadBufferVec[(uint)data.first] = data.second;
		}
		JFrameResource::~JFrameResource()
		{ 
			for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
				uploadBufferVec[i] = nullptr;

			for (uint i = 0; i < (uint)J_MAIN_THREAD_CMD_ORDER::COUNT; ++i)
			{
				cmdListAlloc[i] = nullptr;
				cmdList[i] = nullptr;
			}

			for (uint i = 0; i < Constants::gMaxFrameThread; ++i)
			{
				sceneCmdListAlloc[i] = nullptr;
				sceneCmdList[i] = nullptr;
				shadowCmdListAlloc[i] = nullptr;
				shadowCmdList[i] = nullptr;

				hzbOccDrawCmdListAlloc[i] = nullptr;
				hzbOccDrawCmdList[i] = nullptr; 

				sceneCmdBatch[i] = nullptr;
				shadowCmdBatch[i] = nullptr;
				hzbOccDrawCmdBatch[i] = nullptr;
			} 
		}
		uint JFrameResource::GetElementCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept
		{
			return uploadBufferVec[(uint)type]->ElementCount();
		}
		ID3D12GraphicsCommandList* JFrameResource::GetCmd(const J_MAIN_THREAD_CMD_ORDER type)const noexcept
		{
			return cmdList[(uint)type].Get();
		}
		ID3D12GraphicsCommandList* JFrameResource::GetSceneCmd(const uint index)const noexcept
		{
			return sceneCmdList[index].Get();
		}
		ID3D12GraphicsCommandList* JFrameResource::GetShadowCmd(const uint index)const noexcept
		{
			return shadowCmdList[index].Get();
		}
		ID3D12GraphicsCommandList* JFrameResource::GetHzbDrawOccCmd(const uint index)const noexcept
		{
			return hzbOccDrawCmdList[index].Get();
		} 
		void JFrameResource::ReBuildFrameResource(ID3D12Device* device, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint newCount)
		{
			if (newCount == GetElementCount(type))
				return;

			//Caution
			//초기화시 기존 object들 setFrameDirty 호출 필요!
			uploadBufferVec[(uint)type]->Clear();
			uploadBufferVec[(uint)type]->Build(device, newCount);
		}
		void JFrameResource::ResetCmd(const J_MAIN_THREAD_CMD_ORDER type)
		{
			ThrowIfFailedHr(cmdListAlloc[(int)type]->Reset());
			ThrowIfFailedHr(cmdList[(int)type]->Reset(cmdListAlloc[(int)type].Get(), nullptr));
		}
		void JFrameResource::ResetCmd(const uint useThreadCount)
		{ 
			for (uint i = 0; i < (uint)J_MAIN_THREAD_CMD_ORDER::COUNT; ++i)
			{
				ThrowIfFailedHr(cmdListAlloc[i]->Reset());
				ThrowIfFailedHr(cmdList[i]->Reset(cmdListAlloc[i].Get(), nullptr));
			}
			 
			for (uint i = 0; i < useThreadCount; ++i)
			{
				ThrowIfFailedHr(sceneCmdListAlloc[i]->Reset());
				ThrowIfFailedHr(sceneCmdList[i]->Reset(sceneCmdListAlloc[i].Get(), nullptr));

				ThrowIfFailedHr(shadowCmdListAlloc[i]->Reset());
				ThrowIfFailedHr(shadowCmdList[i]->Reset(shadowCmdListAlloc[i].Get(), nullptr));

				ThrowIfFailedHr(hzbOccDrawCmdListAlloc[i]->Reset());
				ThrowIfFailedHr(hzbOccDrawCmdList[i]->Reset(hzbOccDrawCmdListAlloc[i].Get(), nullptr));
			}
		}
	}
}