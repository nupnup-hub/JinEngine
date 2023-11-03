#include"JDx12GraphicDataSet.h"
#include"../../GraphicResource/JGraphicResourceManager.h"
#include"../../Culling/JCullingManager.h"
#include"../../FrameResource/JFrameResource.h"
#include"../../DepthMap/JDepthTest.h"
#include"../../DepthMap/JDepthMapDebug.h"
#include"../../Blur/JBlur.h"

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

	JDx12GraphicDepthMapDebugObjectSet::JDx12GraphicDepthMapDebugObjectSet(JGraphicResourceManager* graphicResourceM, ID3D12GraphicsCommandList* cmdList)
		:JGraphicDepthMapDebugObjectSet(graphicResourceM), cmdList(cmdList)
	{
		if (graphicResourceM != nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(graphicResourceM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDepthMapDebugObjectSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicDepthMapDebugHandleSet::JDx12GraphicDepthMapDebugHandleSet(const JVector2<uint> size,
		const float nearF,
		const float farF,
		const bool isPerspective,
		ID3D12GraphicsCommandList* cmdList,
		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle)
		:JGraphicDepthMapDebugHandleSet(size, nearF, farF, isPerspective), cmdList(cmdList), srcHandle(srcHandle), destHandle(destHandle)
	{
		SetValid(true);
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDepthMapDebugHandleSet::GetDeviceType()const noexcept
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
		JBlur* blur,
		ID3D12GraphicsCommandList* cmdList) :
		JGraphicShadowMapDrawSet(device, currFrame, graphicResourceM, cullingM, blur), cmdList(cmdList)
	{
		if (device == nullptr || currFrame == nullptr || graphicResourceM == nullptr || cullingM == nullptr || blur == nullptr || cmdList == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(device) && IsSameDevice(currFrame) && IsSameDevice(graphicResourceM) && IsSameDevice(cullingM) && IsSameDevice(blur));
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

	JDx12GraphicOutlineObjectSet::JDx12GraphicOutlineObjectSet(JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM,
		ID3D12GraphicsCommandList* cmdList)
		:JGraphicOutlineObjectSet(device, graphicResourceM), cmdList(cmdList)
	{
		if (device == nullptr || graphicResourceM == nullptr)
			SetValid(false);
		else
			SetValid(IsSameDevice(device) && IsSameDevice(graphicResourceM));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOutlineObjectSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicOutlineHandleSet::JDx12GraphicOutlineHandleSet(JGraphicResourceManager* graphicResourceM,
		ID3D12GraphicsCommandList* cmdList,
		CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle)
		:JGraphicOutlineHandleSet(graphicResourceM), cmdList(cmdList), depthMapHandle(depthMapHandle), stencilMapHandle(stencilMapHandle)
	{
		SetValid(cmdList != nullptr);
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOutlineHandleSet::GetDeviceType()const noexcept
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
		SetValid(cmdList != nullptr);
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicBlurApplySet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

}