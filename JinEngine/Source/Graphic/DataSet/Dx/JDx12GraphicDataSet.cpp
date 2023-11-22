#include"JDx12GraphicDataSet.h"
#include"../../GraphicResource/JGraphicResourceManager.h"
#include"../../Culling/JCullingManager.h"
#include"../../FrameResource/JFrameResource.h"
#include"../../DepthMap/JDepthTest.h"
#include"../../DepthMap/JDepthMapDebug.h"
#include"../../Image/JImageProcessing.h"

namespace JinEngine::Graphic
{
	JDx12GraphicBindSet::JDx12GraphicBindSet(JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingManager,
		ID3D12GraphicsCommandList* cmdList)
		: JGraphicBindSet(currFrame, graphicResourceM, cullingManager), cmdList(cmdList)
	{
		if (currFrame == nullptr || graphicResourceM == nullptr || cullingManager != nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsAllSameDevice(currFrame, graphicResourceM, cullingManager));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicBindSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
 
	JDx12GraphicDepthMapDrawSet::JDx12GraphicDepthMapDrawSet(JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		ID3D12GraphicsCommandList* cmdList)
		:JGraphicDepthMapDrawSet(currFrame, graphicResourceM, cullingM), cmdList(cmdList)
	{
		if (currFrame == nullptr || graphicResourceM != nullptr || cullingM == nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(currFrame) && IsSameDevice(graphicResourceM) && IsSameDevice(cullingM));
	}
	JDx12GraphicDepthMapDrawSet::JDx12GraphicDepthMapDrawSet(const JDx12GraphicOccDrawSet* drawSet)
		:JGraphicDepthMapDrawSet(drawSet), cmdList(drawSet->cmdList)
	{}

	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDepthMapDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
 
	JDx12GraphicDepthMapDebugTaskSet::JDx12GraphicDepthMapDebugTaskSet(JGraphicDevice* gDevice, JGraphicResourceManager* graphicResourceM, ID3D12GraphicsCommandList* cmdList)
		:JGraphicDepthMapDebugTaskSet(gDevice, graphicResourceM), cmdList(cmdList), useHandle(false)
	{
		SetValid(gDevice != nullptr && graphicResourceM != nullptr && IsSameDevice(gDevice) && IsSameDevice(graphicResourceM));
	}
	JDx12GraphicDepthMapDebugTaskSet::JDx12GraphicDepthMapDebugTaskSet(JGraphicDevice* gDevice, 
		JGraphicResourceManager* graphicResourceM,
		const JVector2<uint> size,
		const float nearF,
		const float farF,
		const bool isPerspective,
		ID3D12GraphicsCommandList* cmdList,
		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle)
		:JGraphicDepthMapDebugTaskSet(gDevice, graphicResourceM, size, nearF, farF, isPerspective),
		cmdList(cmdList),
		srcHandle(srcHandle),
		destHandle(destHandle),
		useHandle(true)
	{
		SetValid(gDevice != nullptr && graphicResourceM != nullptr && IsSameDevice(gDevice) && IsSameDevice(graphicResourceM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDepthMapDebugTaskSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicSceneDrawSet::JDx12GraphicSceneDrawSet(JGraphicDevice* device,
		JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		ID3D12GraphicsCommandList* cmdList) :
		JGraphicSceneDrawSet(device, currFrame, graphicResourceM, cullingM), cmdList(cmdList)
	{
		if (device == nullptr || currFrame == nullptr || graphicResourceM == nullptr || cullingM == nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(device) && IsSameDevice(currFrame) && IsSameDevice(graphicResourceM) && IsSameDevice(cullingM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicSceneDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicShadowMapDrawSet::JDx12GraphicShadowMapDrawSet(JGraphicDevice* device,
		JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		JImageProcessing* image,
		ID3D12GraphicsCommandList* cmdList) :
		JGraphicShadowMapDrawSet(device, currFrame, graphicResourceM, cullingM, image), cmdList(cmdList)
	{
		if (device == nullptr || currFrame == nullptr || graphicResourceM == nullptr || cullingM == nullptr || image == nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(device) && IsSameDevice(currFrame) && IsSameDevice(graphicResourceM) && IsSameDevice(cullingM) && IsSameDevice(image));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicShadowMapDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicOccDrawSet::JDx12GraphicOccDrawSet(JGraphicDevice* device,
		JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		JDepthTest* depthTest,
		ID3D12GraphicsCommandList* cmdList) :
		JGraphicOccDrawSet(device, currFrame, graphicResourceM, cullingM, depthTest), cmdList(cmdList)
	{
		if (device == nullptr || currFrame == nullptr || graphicResourceM == nullptr ||
			cullingM == nullptr || cmdList == nullptr || depthTest == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(device) && IsSameDevice(currFrame) && IsSameDevice(graphicResourceM) && IsSameDevice(cullingM) && IsSameDevice(depthTest));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOccDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicOccDebugDrawSet::JDx12GraphicOccDebugDrawSet(JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM,
		JDepthMapDebug* depthDebug,
		ID3D12GraphicsCommandList* cmdList) :
		JGraphicOccDebugDrawSet(device, graphicResourceM, depthDebug), cmdList(cmdList)
	{
		if (device == nullptr || graphicResourceM == nullptr ||
			cmdList == nullptr || depthDebug == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(device) && IsSameDevice(graphicResourceM) && IsSameDevice(depthDebug));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOccDebugDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicHzbOccComputeSet::JDx12GraphicHzbOccComputeSet(JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		ID3D12GraphicsCommandList* cmdList)
		: JGraphicHzbOccComputeSet(currFrame, graphicResourceM, cullingM), cmdList(cmdList)
	{
		if (currFrame == nullptr || graphicResourceM == nullptr || cullingM == nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(currFrame) && IsSameDevice(graphicResourceM) && IsSameDevice(cullingM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicHzbOccComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicHdOccExtractSet::JDx12GraphicHdOccExtractSet(JCullingManager* cullingM, ID3D12GraphicsCommandList* cmdList)
		: JGraphicHdOccExtractSet(cullingM), cmdList(cmdList)
	{
		if (cullingM == nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(cullingM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicHdOccExtractSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
 
	JDx12GraphicOutlineDrawSet::JDx12GraphicOutlineDrawSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, ID3D12GraphicsCommandList* cmdList)
		:JGraphicOutlineDrawSet(device, graphicResourceM), cmdList(cmdList), useHandle(false)
	{
		SetValid(device != nullptr && graphicResourceM != nullptr && cmdList != nullptr && IsSameDevice(device) && IsSameDevice(graphicResourceM));
	}
	JDx12GraphicOutlineDrawSet::JDx12GraphicOutlineDrawSet(JGraphicDevice* device, 
		JGraphicResourceManager* graphicResourceM,
		ID3D12GraphicsCommandList* cmdList,
		CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle)
		: JGraphicOutlineDrawSet(device, graphicResourceM),
		cmdList(cmdList),
		depthMapHandle(depthMapHandle), 
		stencilMapHandle(stencilMapHandle),
		useHandle(true)
	{
		SetValid(device != nullptr && graphicResourceM != nullptr  && cmdList != nullptr && IsSameDevice(device) && IsSameDevice(graphicResourceM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOutlineDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicBlurApplySet::JDx12GraphicBlurApplySet(JGraphicDevice* device,
		std::unique_ptr<JBlurDesc>&& desc,
		ID3D12GraphicsCommandList* cmdList,
		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle)
		:JGraphicBlurTaskSet(device, std::move(desc)), cmdList(cmdList), srcHandle(srcHandle), destHandle(destHandle)
	{
		SetValid(device != nullptr && cmdList != nullptr && IsSameDevice(device));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicBlurApplySet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

			 
	JDx12GraphicDownSampleApplySet::JDx12GraphicDownSampleApplySet(JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM,
		std::unique_ptr<JDownSampleDesc>&& desc,
		std::vector<Core::JDataHandle>&& handle,
		ID3D12GraphicsCommandList* cmdList,
		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle)
		:JGraphicDownSampleTaskSet(device, graphicResourceM, std::move(desc), std::move(handle)),
		cmdList(cmdList), 
		srcHandle(srcHandle), 
		destHandle(destHandle)
	{
		SetValid(device != nullptr && graphicResourceM != nullptr && cmdList != nullptr && IsSameDevice(device) && IsSameDevice(graphicResourceM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDownSampleApplySet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicSsaoTaskSet::JDx12GraphicSsaoTaskSet(JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM, 
		JFrameResource* currFrame,
		ID3D12GraphicsCommandList* cmdList)
		:JGraphicSsaoTaskSet(device, graphicResourceM, currFrame), 
		cmdList(cmdList)
	{
		SetValid(device != nullptr && graphicResourceM != nullptr && currFrame != nullptr && cmdList != nullptr &&
			IsSameDevice(device)&& IsSameDevice(currFrame) && IsSameDevice(graphicResourceM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicSsaoTaskSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
}