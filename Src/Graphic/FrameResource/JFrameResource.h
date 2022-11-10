#pragma once 
#include<d3d12.h>
#include<wrl/client.h>
#include"../../Core/JDataType.h"
#include"JUploadBuffer.h" 

namespace JinEngine
{
	namespace Graphic
	{
		struct JMaterialConstants;
		struct JObjectConstants;
		struct JAnimationConstants;
		struct JPassConstants;
		struct JCameraConstants;
		struct JLightConstants;
		struct JSMLightConstants;
		struct JShadowMapConstants;
		struct JBoundingObjectConstants;

		struct JFrameResource
		{
			// We cannot reset the allocator until the GPU is done processing the commands.
			// So each frame needs their own allocator.
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;

			// We cannot update a cbuffer until the GPU is done processing the commands
			// that reference it.  So each frame needs their own cbuffers.
			std::unique_ptr<JUploadBuffer<JMaterialConstants>> materialBuffer = nullptr;
			std::unique_ptr<JUploadBuffer<JObjectConstants>> objectCB = nullptr;
			std::unique_ptr<JUploadBuffer<JAnimationConstants>> skinnedCB = nullptr;
			std::unique_ptr<JUploadBuffer<JPassConstants>> passCB = nullptr;
			std::unique_ptr<JUploadBuffer<JCameraConstants>> cameraCB = nullptr;
			std::unique_ptr<JUploadBuffer<JLightConstants>> lightCB = nullptr;
			std::unique_ptr<JUploadBuffer<JSMLightConstants>> smLightCB = nullptr;
			std::unique_ptr<JUploadBuffer<JShadowMapConstants>> shadowCalCB = nullptr;
			std::unique_ptr<JUploadBuffer<JBoundingObjectConstants>> bundingObjectCB = nullptr;
			// Fence value to mark commands up to this fence point.  This lets us
			// check if these frame resources are still in use by the GPU.
			uint64 fence = 0;
		public:
			JFrameResource(ID3D12Device* device,
				const uint initMateiralCount,
				const uint initRenderItemCount,
				const uint initAnimatorCount,
				const uint initSceneCount,
				const uint initCameraCount,
				const uint initLightCount);

			JFrameResource(const JFrameResource& rhs) = delete;
			JFrameResource& operator=(const JFrameResource& rhs) = delete;
			~JFrameResource();
		};
	}
}