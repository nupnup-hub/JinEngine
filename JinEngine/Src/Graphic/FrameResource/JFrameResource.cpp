#include"JFrameResource.h"
#include"../JGraphicInfo.h"
#include"../../Core/Exception/JExceptionMacro.h"

#include"JObjectConstants.h" 
#include"JAnimationConstants.h"
#include"JMaterialConstants.h"
#include"JPassConstants.h" 
#include"JCameraConstants.h"
#include"JLightConstants.h"
#include"JShadowMapConstants.h"
#include"JBoundingObjectConstants.h"

namespace JinEngine
{
	namespace Graphic
	{
		JFrameResource::JFrameResource(ID3D12Device* device, const JGraphicInfo& gInfo)
		{  
			ThrowIfFailedHr(device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

			materialBuffer = std::make_unique<JUploadBuffer<JMaterialConstants>>(J_UPLOAD_BUFFER_TYPE::COMMON);
			lightBuffer = std::make_unique<JUploadBuffer<JLightConstants>>(J_UPLOAD_BUFFER_TYPE::COMMON);
			smLightBuffer = std::make_unique<JUploadBuffer<JShadowMapLightConstants>>(J_UPLOAD_BUFFER_TYPE::COMMON);
			 
			objectCB = std::make_unique<JUploadBuffer<JObjectConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			skinnedCB = std::make_unique<JUploadBuffer<JAnimationConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			passCB = std::make_unique<JUploadBuffer<JPassConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			cameraCB = std::make_unique<JUploadBuffer<JCameraConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			lightIndexCB = std::make_unique<JUploadBuffer<JLightIndexConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			shadowCalCB = std::make_unique<JUploadBuffer<JShadowMapConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			bundingObjectCB = std::make_unique<JUploadBuffer<JBoundingObjectConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			
			materialBuffer->Build(device, gInfo.minCapacity);
			lightBuffer->Build(device, gInfo.minCapacity);
			smLightBuffer->Build(device, gInfo.minCapacity);

			objectCB->Build(device, gInfo.minCapacity);
			skinnedCB->Build(device, gInfo.minCapacity);
			passCB->Build(device, gInfo.minCapacity);
			cameraCB->Build(device, gInfo.minCapacity);
			lightIndexCB->Build(device, gInfo.minCapacity);
			shadowCalCB->Build(device, gInfo.minCapacity);
			bundingObjectCB->Build(device, gInfo.minCapacity);

			uploadBufferMap = std::unordered_map< J_UPLOAD_RESOURCE_TYPE, JUploadBufferBase*>
			{ 
				{J_UPLOAD_RESOURCE_TYPE::MATERIAL, materialBuffer.get()}, {J_UPLOAD_RESOURCE_TYPE::LIGHT, lightBuffer.get()},
				{J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP_LIGHT, smLightBuffer.get()}, {J_UPLOAD_RESOURCE_TYPE::OBJECT, objectCB.get()},
				{J_UPLOAD_RESOURCE_TYPE::ANIMATION, skinnedCB.get()}, {J_UPLOAD_RESOURCE_TYPE::PASS, passCB.get()},
				{J_UPLOAD_RESOURCE_TYPE::CAMERA, cameraCB.get()},  {J_UPLOAD_RESOURCE_TYPE::LIGHT_INDEX, lightIndexCB.get()},
				{J_UPLOAD_RESOURCE_TYPE::SHADOW_MAP, shadowCalCB.get()},  {J_UPLOAD_RESOURCE_TYPE::BOUNDING_OBJECT, bundingObjectCB.get()}
			};
		}
		JFrameResource::~JFrameResource()
		{
			uploadBufferMap.clear();
		}
		void JFrameResource::BuildFrameResource(ID3D12Device* device, const J_UPLOAD_RESOURCE_TYPE type, const uint newCount)
		{
			if (newCount == GetElementCount(type))
				return;

			//Caution
			//초기화시 기존 object들 setFrameDirty 호출 필요!
			auto data = uploadBufferMap.find(type);
			data->second->Clear();
			data->second->Build(device, newCount);
		}
		uint JFrameResource::GetElementCount(const J_UPLOAD_RESOURCE_TYPE type)
		{
			return uploadBufferMap.find(type)->second->ElementCount();
		}
	}
}