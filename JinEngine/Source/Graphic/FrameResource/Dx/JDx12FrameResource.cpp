#include"JDx12FrameResource.h"
#include"../../JGraphicInfo.h"
#include"../../../Core/Exception/JExceptionMacro.h" 
#include"../../../Core/Utility/JCommonUtility.h"

#include"../JObjectConstants.h" 
#include"../JAnimationConstants.h"
#include"../JMaterialConstants.h"
#include"../JPassConstants.h" 
#include"../JCameraConstants.h"
#include"../JLightConstants.h" 
#include"../JBoundingObjectConstants.h"
#include"../JOcclusionConstants.h"
#include"../JShadowMapConstants.h"
#include"../JDepthTestConstants.h"


namespace JinEngine::Graphic
{

	namespace
	{
		static std::wstring PrivateName(const J_MAIN_THREAD_ORDER orderType)
		{
			switch (orderType)
			{
			case JinEngine::Graphic::J_MAIN_THREAD_ORDER::BEGIN:
				return L"Begin";
			case JinEngine::Graphic::J_MAIN_THREAD_ORDER::MID:
				return L"Mid";
			case JinEngine::Graphic::J_MAIN_THREAD_ORDER::END:
				return L"End";
			default:
				return L"Error";
			}
		}
		static std::wstring PrivateName(const J_THREAD_TASK_TYPE taskType)
		{
			switch (taskType)
			{
			case JinEngine::Graphic::J_THREAD_TASK_TYPE::OCC:
				return L"Occ";
			case JinEngine::Graphic::J_THREAD_TASK_TYPE::SHADOW_MAP:
				return L"Shadow Map";
			case JinEngine::Graphic::J_THREAD_TASK_TYPE::SCENE:
				return L"Scene";
			default:
				return L"Error";
			}
		}
	}
	JDx12FrameResource::EraseBlockInfo::EraseBlockInfo(const uint reqIndex, const uint dataIndex)
		:reqIndex(reqIndex), dataIndex(dataIndex)
	{}
	 
	JDx12FrameResource::~JDx12FrameResource()
	{
		Clear();
	}
	void JDx12FrameResource::Intialize(JGraphicDevice* device, const JGraphicInfo& gInfo)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		ID3D12Device* d3dDevice = dx12Device->GetDevice();

		for (uint i = 0; i < (uint)J_MAIN_THREAD_ORDER::COUNT; ++i)
		{
			ThrowIfFailedHr(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAlloc[i].GetAddressOf())));
			ThrowIfFailedHr(d3dDevice->CreateCommandList(0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				cmdListAlloc[i].Get(),
				nullptr,
				IID_PPV_ARGS(cmdList[i].GetAddressOf())));
			cmdList[i]->Close();
			cmdListAlloc[i]->SetName((PrivateName((J_MAIN_THREAD_ORDER)i) + L" CmdAllocator").c_str());
			cmdList[i]->SetName((PrivateName((J_MAIN_THREAD_ORDER)i) +L" CmdList").c_str());
		}
		for (uint i = 0; i < (uint)J_THREAD_TASK_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < Constants::gMaxFrameThread; ++j)
			{
				ThrowIfFailedHr(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(treadCmdListAlloc[i][j].GetAddressOf())));
				ThrowIfFailedHr(d3dDevice->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					treadCmdListAlloc[i][j].Get(),
					nullptr,
					IID_PPV_ARGS(treadCmdList[i][j].GetAddressOf())));
				treadCmdList[i][j]->Close();
				treadCmdListAlloc[i][j]->SetName((PrivateName((J_THREAD_TASK_TYPE)i) + std::to_wstring(i) +L" CmdAllocator").c_str());
				treadCmdList[i][j]->SetName((PrivateName((J_THREAD_TASK_TYPE)i) + std::to_wstring(i) + L" CmdList").c_str());
				threadCmdBatch[i][j] = treadCmdList[i][j].Get();
			}
		}
		for (uint i = 0; i < (uint)J_THREAD_TASK_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < gInfo.frameThreadCount; ++j)
				threadTaskHandle[i][j] = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
		framePerThread = gInfo.frameThreadCount;
	 

		auto nameLam = [](const J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			return JCUtil::StrToWstr(Core::GetName(type));
		};

		materialBuffer = std::make_unique<JDx12GraphicBuffer<JMaterialConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL), J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		dLightBuffer = std::make_unique<JDx12GraphicBuffer<JDirectionalLightConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT), J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		csmBuffer = std::make_unique<JDx12GraphicBuffer<JCsmConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO), J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		pLightBuffer = std::make_unique<JDx12GraphicBuffer<JPointLightConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT), J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		sLightBuffer = std::make_unique<JDx12GraphicBuffer<JSpotLightConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT), J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		rLightBuffer = std::make_unique<JDx12GraphicBuffer<JRectLightConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT), J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
		hzbOccObjectBuffer = std::make_unique<JDx12GraphicBuffer<JHzbOccObjectConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT), J_GRAPHIC_BUFFER_TYPE::UPLOAD_BUFFER);
     
		objectCB = std::make_unique<JDx12GraphicBuffer<JObjectConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		skinnedCB = std::make_unique<JDx12GraphicBuffer<JAnimationConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		enginePassCB = std::make_unique<JDx12GraphicBuffer<JEnginePassConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		scenePassCB = std::make_unique<JDx12GraphicBuffer<JScenePassConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		cameraCB = std::make_unique<JDx12GraphicBuffer<JCameraConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		bundingObjectCB = std::make_unique<JDx12GraphicBuffer<JBoundingObjectConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		depthTestPassCB = std::make_unique<JDx12GraphicBuffer<JDepthTestPassConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		hzbOccReqCB = std::make_unique<JDx12GraphicBuffer<JHzbOccComputeConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		smArrayDrawCB = std::make_unique<JDx12GraphicBuffer<JShadowMapArrayDrawConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		smCubeDrawCB = std::make_unique<JDx12GraphicBuffer<JShadowMapCubeDrawConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		smDrawCB = std::make_unique<JDx12GraphicBuffer<JShadowMapDrawConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		ssaoCB = std::make_unique<JDx12GraphicBuffer<JSsaoConstants>>(nameLam(J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS), J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
		
		materialBuffer->Build(device, gInfo.minCapacity);
		dLightBuffer->Build(device, gInfo.minCapacity);
		csmBuffer->Build(device, gInfo.minCapacity);
		pLightBuffer->Build(device, gInfo.minCapacity);
		sLightBuffer->Build(device, gInfo.minCapacity);
		rLightBuffer->Build(device, gInfo.minCapacity);
		hzbOccObjectBuffer->Build(device, gInfo.minCapacity);

		objectCB->Build(device, gInfo.minCapacity);
		skinnedCB->Build(device, gInfo.minCapacity);
		enginePassCB->Build(device, gInfo.minCapacity);
		scenePassCB->Build(device, gInfo.minCapacity);
		cameraCB->Build(device, gInfo.minCapacity);
		bundingObjectCB->Build(device, gInfo.minCapacity);
		depthTestPassCB->Build(device, gInfo.minCapacity);
		hzbOccReqCB->Build(device, gInfo.minCapacity);
		smArrayDrawCB->Build(device, gInfo.minCapacity);
		smCubeDrawCB->Build(device, gInfo.minCapacity);
		smDrawCB->Build(device, gInfo.minCapacity);
		ssaoCB->Build(device, gInfo.minCapacity);

		std::unordered_map<J_UPLOAD_FRAME_RESOURCE_TYPE, JDx12GraphicBufferInterface*> uploadBufferMap =
		{
			{J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, materialBuffer.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT, dLightBuffer.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO, csmBuffer.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT, pLightBuffer.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT, sLightBuffer.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT, rLightBuffer.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, objectCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, skinnedCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS, enginePassCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS, scenePassCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, cameraCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, bundingObjectCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, depthTestPassCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, hzbOccReqCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT, hzbOccObjectBuffer.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW, smArrayDrawCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW, smCubeDrawCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, smDrawCB.get()},
			{J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, ssaoCB.get()}
		};
		for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
			bufferVec[i] = nullptr;

		for (const auto& data : uploadBufferMap)
			bufferVec[(uint)data.first] = data.second;
	}
	void JDx12FrameResource::Clear()
	{
		for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
			bufferVec[i] = nullptr;

		for (uint i = 0; i < (uint)J_MAIN_THREAD_ORDER::COUNT; ++i)
		{
			cmdListAlloc[i] = nullptr;
			cmdList[i] = nullptr;
		}

		for (uint i = 0; i < (uint)J_THREAD_TASK_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < Constants::gMaxFrameThread; ++j)
			{
				treadCmdListAlloc[i][j] = nullptr;
				treadCmdList[i][j] = nullptr;
				threadCmdBatch[i][j] = nullptr;
			}
		} 
		for (uint i = 0; i < (uint)J_THREAD_TASK_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < framePerThread; ++j)
				CloseHandle(threadTaskHandle[i][j]);
		}
		framePerThread = 0;
	}
	J_GRAPHIC_DEVICE_TYPE JDx12FrameResource::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	JGraphicBufferBase* JDx12FrameResource::GetGraphicBufferBase(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept
	{ 
		return bufferVec[(uint)type];
	}
	JDx12GraphicBufferInterface* JDx12FrameResource::GetDx12Buffer(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept
	{
		return bufferVec[(uint)type];
	}
	uint JDx12FrameResource::GetElementCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept
	{ 
		return bufferVec[(uint)type]->GetElementCount();
	}
	GraphicFence JDx12FrameResource::GetFenceValue()const noexcept
	{
		return fence;
	} 
	ID3D12GraphicsCommandList* JDx12FrameResource::GetCmd(const J_MAIN_THREAD_ORDER type)const noexcept
	{
		return cmdList[(uint)type].Get();
	}
	ID3D12GraphicsCommandList* JDx12FrameResource::GetCmd(const J_THREAD_TASK_TYPE taskType, const uint index)const noexcept
	{
		return treadCmdList[(uint)taskType][index].Get();
	}
	ID3D12CommandList* const* JDx12FrameResource::GetBatchCmd(const J_THREAD_TASK_TYPE taskType)const noexcept
	{
		return threadCmdBatch[(uint)taskType];
	}
	HANDLE JDx12FrameResource::GetHandle(const J_THREAD_TASK_TYPE taskType, const uint index)const noexcept
	{
		return threadTaskHandle[(uint)taskType][index];
	}
	const HANDLE* JDx12FrameResource::GetHandle(const J_THREAD_TASK_TYPE taskType)const noexcept
	{
		return threadTaskHandle[(uint)taskType];
	}
	void JDx12FrameResource::SetFenceValue(const GraphicFence value)noexcept
	{
		fence = value;
	}
	void JDx12FrameResource::ReBuild(JGraphicDevice* device, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint newCount)
	{
		if (newCount == GetElementCount(type))
			return;

		//Caution
		//초기화시 기존 object들 setFrameDirty 호출 필요! 
		bufferVec[(uint)type]->Build(device, newCount);
	}
	void JDx12FrameResource::ResetCmd(const J_MAIN_THREAD_ORDER type)
	{
		ThrowIfFailedHr(cmdListAlloc[(int)type]->Reset());
		ThrowIfFailedHr(cmdList[(int)type]->Reset(cmdListAlloc[(int)type].Get(), nullptr));
	}
	void JDx12FrameResource::ResetCmd(const uint useThreadCount)
	{
		for (uint i = 0; i < (uint)J_MAIN_THREAD_ORDER::COUNT; ++i)
		{
			ThrowIfFailedHr(cmdListAlloc[i]->Reset());
			ThrowIfFailedHr(cmdList[i]->Reset(cmdListAlloc[i].Get(), nullptr));
		}

		for (uint i = 0; i < (uint)J_THREAD_TASK_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < Constants::gMaxFrameThread; ++j)
			{
				ThrowIfFailedHr(treadCmdListAlloc[i][j]->Reset());
				ThrowIfFailedHr(treadCmdList[i][j]->Reset(treadCmdListAlloc[i][j].Get(), nullptr));
			}
		}
	}
}
