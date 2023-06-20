#pragma once 
#include<d3d12.h>
#include<wrl/client.h>
#include"../../JGraphicConstants.h"
#include"../../JGraphicType.h"
#include"../../../Core/JDataType.h"
#include"../../../Core/Func/Callable/JCallable.h"
#include"../JUploadBuffer.h" 
#include"../JUploadType.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphic;
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

		struct JHzbOccObjectConstants;
		struct JHzbOccRequestorConstants; 

		struct JGraphicInfo;
		class JFrameResource
		{ 
		private:
			struct EraseBlockInfo
			{
			public:
				uint reqIndex = 0;
				uint dataIndex = 0;
			public:
				EraseBlockInfo(const uint reqIndex, const uint dataIndex);
			};
		private:
			friend class JGraphic; 
		private:
			// We cannot reset the allocator until the GPU is done processing the commands.
			// So each frame needs their own allocator.
			//used by main thread 
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc[(uint)J_MAIN_THREAD_CMD_ORDER::COUNT];
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList[(uint)J_MAIN_THREAD_CMD_ORDER::COUNT];
		private:
			//used by thread
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> sceneCmdListAlloc[Constants::gMaxFrameThread];
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> sceneCmdList[Constants::gMaxFrameThread];

			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> shadowCmdListAlloc[Constants::gMaxFrameThread];
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> shadowCmdList[Constants::gMaxFrameThread];

			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> hzbOccDrawCmdListAlloc[Constants::gMaxFrameThread];
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> hzbOccDrawCmdList[Constants::gMaxFrameThread]; 
		private:
			ID3D12CommandList* sceneCmdBatch[Constants::gMaxFrameThread];
			ID3D12CommandList* shadowCmdBatch[Constants::gMaxFrameThread];
			ID3D12CommandList* hzbOccDrawCmdBatch[Constants::gMaxFrameThread];
		private:
			// Fence value to mark commands up to this fence point.  This lets us
			// check if these frame resources are still in use by the GPU.
			uint64 fence = 0;
		public:
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
		public:
			//ShadowMap
			std::unique_ptr<JUploadBuffer<JShadowMapConstants>> shadowMapCalCB = nullptr;
		public:
			//Bounding
			std::unique_ptr<JUploadBuffer<JBoundingObjectConstants>> bundingObjectCB = nullptr;
		public:
			//HZB occ
			std::unique_ptr<JUploadBuffer<JHzbOccRequestorConstants>> hzbOccReqCB = nullptr;
			std::unique_ptr<JUploadBuffer<JHzbOccObjectConstants>> hzbOccObjectBuffer = nullptr;
		private:
			JUploadBufferBase* uploadBufferVec[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT];
		public:
			JFrameResource(ID3D12Device* device, const JGraphicInfo& gInfo);
			JFrameResource(const JFrameResource& rhs) = delete;
			JFrameResource& operator=(const JFrameResource& rhs) = delete;
			~JFrameResource();
		public:
			uint GetElementCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept;
			ID3D12GraphicsCommandList* GetCmd(const J_MAIN_THREAD_CMD_ORDER type)const noexcept;	//Main thread
			ID3D12GraphicsCommandList* GetSceneCmd(const uint index)const noexcept;		//thread
			ID3D12GraphicsCommandList* GetShadowCmd(const uint index)const noexcept;	//thread
			ID3D12GraphicsCommandList* GetHzbDrawOccCmd(const uint index)const noexcept;	//thread 
		public:
			void ReBuildFrameResource(ID3D12Device* device, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint newCount);
		public:
			void ResetCmd(const J_MAIN_THREAD_CMD_ORDER type);	//reset specific cmd
			void ResetCmd(const uint useThreadCount);
		};
	}
}