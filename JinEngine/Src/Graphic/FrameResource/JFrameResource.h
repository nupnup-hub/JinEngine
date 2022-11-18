#pragma once 
#include<d3d12.h>
#include<wrl/client.h>
#include"../../Core/JDataType.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"../Upload/JUploadBuffer.h" 
#include"JFrameResourceType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JMaterialConstants;
		struct JLightConstants;
		struct JShadowMapLightConstants;

		struct JObjectConstants;
		struct JAnimationConstants;
		struct JPassConstants;
		struct JCameraConstants;  
		struct JLightIndexConstants;
		struct JShadowMapConstants;
		struct JBoundingObjectConstants;

		struct JGraphicInfo;
		struct JFrameResource
		{
		private:
			friend class JGraphicImpl;
		private:
			// We cannot reset the allocator until the GPU is done processing the commands.
			// So each frame needs their own allocator.
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;
			// Fence value to mark commands up to this fence point.  This lets us
			// check if these frame resources are still in use by the GPU.
			uint64 fence = 0;
		private:
			// We cannot update a cbuffer until the GPU is done processing the commands
			// that reference it.  So each frame needs their own cbuffers.
			std::unique_ptr<JUploadBuffer<JMaterialConstants>> materialBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<JLightConstants>> lightBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<JShadowMapLightConstants>> smLightBuffer = nullptr;

			std::unique_ptr<JUploadBuffer<JObjectConstants>> objectCB = nullptr;
			std::unique_ptr<JUploadBuffer<JAnimationConstants>> skinnedCB = nullptr;
			std::unique_ptr<JUploadBuffer<JPassConstants>> passCB = nullptr;
			std::unique_ptr<JUploadBuffer<JCameraConstants>> cameraCB = nullptr; 
			std::unique_ptr<JUploadBuffer<JLightIndexConstants>> lightIndexCB = nullptr;
			std::unique_ptr<JUploadBuffer<JShadowMapConstants>> shadowCalCB = nullptr;
			std::unique_ptr<JUploadBuffer<JBoundingObjectConstants>> bundingObjectCB = nullptr; 
		private:
			std::unordered_map<J_FRAME_RESOURCE_TYPE, JUploadBufferBase*> uploadBufferMap;
		public:
			JFrameResource(ID3D12Device* device, const JGraphicInfo& gInfo);
			JFrameResource(const JFrameResource& rhs) = delete;
			JFrameResource& operator=(const JFrameResource& rhs) = delete;
			~JFrameResource();
		public:
			void BuildFrameResource(ID3D12Device* device, const J_FRAME_RESOURCE_TYPE type, const uint newCount);
			uint GetElementCount(const J_FRAME_RESOURCE_TYPE type);
		};
	}
}