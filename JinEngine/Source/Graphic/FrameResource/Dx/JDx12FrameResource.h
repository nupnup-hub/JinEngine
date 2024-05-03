#pragma once
#include"../JFrameResource.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h" 
#include"../../Thread/JGraphicThreadType.h"
#include"../../../Core/Func/Callable/JCallable.h"
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12FrameResource final : public JFrameResource
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
			// We cannot reset the allocator until the GPU is done processing the commands.
			// So each frame needs their own allocator.
			//used by main thread 
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc[(uint)J_MAIN_THREAD_ORDER::COUNT];
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList[(uint)J_MAIN_THREAD_ORDER::COUNT];
		private:
			//used by thread
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> treadCmdListAlloc[(uint)J_THREAD_TASK_TYPE::COUNT][Constants::gMaxFrameThread];
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> treadCmdList[(uint)J_THREAD_TASK_TYPE::COUNT][Constants::gMaxFrameThread];
			ID3D12CommandList* threadCmdBatch[(uint)J_THREAD_TASK_TYPE::COUNT][Constants::gMaxFrameThread];
		private: 
			HANDLE threadTaskHandle[(uint)J_THREAD_TASK_TYPE::COUNT][Constants::gMaxFrameThread];
		private:
			// Fence value to mark commands up to this fence point.  This lets us
			// check if these frame resources are still in use by the GPU.
			GraphicFence fence = 0;
		private:
			// We cannot update a cbuffer until the GPU is done processing the commands
			// that reference it.  So each frame needs their own cbuffers.
			std::unique_ptr<JDx12GraphicBufferT<JMaterialConstants>> materialBuffer = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JDirectionalLightConstants>> dLightBuffer = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JCsmConstants>> csmBuffer = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JPointLightConstants>> pLightBuffer = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JSpotLightConstants>> sLightBuffer = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JRectLightConstants>> rLightBuffer = nullptr;	
			std::unique_ptr<JDx12GraphicBufferT<JObjectRefereneceInfoConstants>> objRefInfoBuffer = nullptr;
		private:
			std::unique_ptr<JDx12GraphicBufferT<JObjectConstants>> objectCB = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JAnimationConstants>> skinnedCB = nullptr; 
			std::unique_ptr<JDx12GraphicBufferT<JScenePassConstants>> sceneCB = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JDrawSceneCameraConstants>> cameraCB = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JShadowMapArrayDrawConstants>> smArrayDrawCB = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JShadowMapCubeDrawConstants>> smCubeDrawCB = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JShadowMapDrawConstants>> smDrawCB = nullptr;
		private:
			//Bounding
			std::unique_ptr<JDx12GraphicBufferT<JBoundingObjectConstants>> bundingObjectCB = nullptr;
		private:
			//depth test
			std::unique_ptr<JDx12GraphicBufferT<JDepthTestConstants>> depthTestCB = nullptr;
		private:
			//occ
			std::unique_ptr<JDx12GraphicBufferT<JHzbOccComputeConstants>> hzbOccReqCB = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JHzbOccObjectConstants>> hzbOccObjectBuffer = nullptr;
			std::unique_ptr<JDx12GraphicBufferT<JLightCullingCameraConstants>> litCullCB = nullptr;
		private:
			std::unique_ptr<JDx12GraphicBufferT<JSsaoConstants>> ssaoCB = nullptr; 
		private:
			JDx12GraphicBufferInterface* bufferVec[(uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT];
		private:
			int framePerThread = 0;
		public:
			JDx12FrameResource() = default;
			JDx12FrameResource(const JFrameResource& rhs) = delete;
			JDx12FrameResource& operator=(const JFrameResource& rhs) = delete;
			~JDx12FrameResource();
		public:
			void Intialize(JGraphicDevice* device) final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			JGraphicBufferBase* GetGraphicBufferBase(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept final;
			JDx12GraphicBufferInterface* GetDx12Buffer(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept;
			uint GetElementCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept final;
			GraphicFence GetFenceValue()const noexcept; 
			ID3D12GraphicsCommandList* GetCmd(const J_MAIN_THREAD_ORDER type)const noexcept;	//Main thread
			ID3D12GraphicsCommandList* GetCmd(const J_THREAD_TASK_TYPE taskType, const uint index)const noexcept;	//thread 
			ID3D12CommandList*const* GetBatchCmd(const J_THREAD_TASK_TYPE taskType)const noexcept;
			HANDLE GetHandle(const J_THREAD_TASK_TYPE taskType, const uint index)const noexcept;	//thread 
			const HANDLE* GetHandle(const J_THREAD_TASK_TYPE taskType)const noexcept;	//thread 
		public:
			void SetFenceValue(const GraphicFence value)noexcept;
		public:
			void ReBuild(JGraphicDevice* device, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint newCount)final;
		public:
			void ResetCmd(const J_MAIN_THREAD_ORDER type);	//reset specific cmd
			void ResetCmd(const uint useThreadCount);
		};
	}
}