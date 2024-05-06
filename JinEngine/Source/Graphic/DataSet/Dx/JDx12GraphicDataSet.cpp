/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JDx12GraphicDataSet.h"
#include"../../GraphicResource/JGraphicResourceManager.h"
#include"../../GraphicResource/JGraphicResourceShareData.h"
#include"../../Culling/JCullingManager.h"
#include"../../FrameResource/JFrameResource.h"
#include"../../DepthMap/JDepthTest.h"
#include"../../Debug/JGraphicDebug.h"
#include"../../Image/JBlur.h"
#include"../../Image/JToneMapping.h"
#include"../../Image/JConvertColor.h"
#include"../../Command/JCommandContext.h"

namespace JinEngine::Graphic
{
	JDx12GraphicBindSet::JDx12GraphicBindSet(JCommandContext* context)
		: JGraphicBindSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicBindSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicDepthMapDrawSet::JDx12GraphicDepthMapDrawSet(JCommandContext* context)
		:JGraphicDepthMapDrawSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	JDx12GraphicDepthMapDrawSet::JDx12GraphicDepthMapDrawSet(const JDx12GraphicOccDrawSet* drawSet)
		:JGraphicDepthMapDrawSet(drawSet)
	{
		SetValid(IsAllSameDevice(context));
	}

	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDepthMapDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicDebugRsComputeSet::JDx12GraphicDebugRsComputeSet(JCommandContext* context)
		:JGraphicDebugRsComputeSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDebugRsComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	 
	JDx12GraphicSceneDrawSet::JDx12GraphicSceneDrawSet(JCommandContext* context) 
		:JGraphicSceneDrawSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicSceneDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicShadowMapDrawSet::JDx12GraphicShadowMapDrawSet(JCommandContext* context, JBlur* blur):
		JGraphicShadowMapDrawSet(context, blur)
	{
		SetValid(IsAllSameDevice(context, blur));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicShadowMapDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicOccDrawSet::JDx12GraphicOccDrawSet(JCommandContext* context, JDepthTest* depthTest)
		:JGraphicOccDrawSet(context, depthTest)
	{
		SetValid(IsAllSameDevice(context, depthTest));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOccDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
 
	JDx12GraphicHzbOccComputeSet::JDx12GraphicHzbOccComputeSet(JCommandContext* context)
		: JGraphicHzbOccComputeSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicHzbOccComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicHdOccExtractSet::JDx12GraphicHdOccExtractSet(JCommandContext* context)
		: JGraphicHdOccExtractSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicHdOccExtractSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicOutlineDrawSet::JDx12GraphicOutlineDrawSet(JCommandContext* context)
		:JGraphicOutlineDrawSet(context), useHandle(false)
	{
		SetValid(IsAllSameDevice(context));
	}
	JDx12GraphicOutlineDrawSet::JDx12GraphicOutlineDrawSet(JCommandContext* context,
		CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle)
		: JGraphicOutlineDrawSet(context),
		depthMapHandle(depthMapHandle),
		stencilMapHandle(stencilMapHandle),
		useHandle(true)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOutlineDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicBlurComputeSet::JDx12GraphicBlurComputeSet(JCommandContext* context,
		std::unique_ptr<JBlurDesc>&& desc,
		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle)
		:JGraphicBlurComputeSet(context, std::move(desc)),
		srcHandle(srcHandle), 
		destHandle(destHandle)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicBlurComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicDownSampleComputeSet::JDx12GraphicDownSampleComputeSet(JCommandContext* context,
		JBlur* blur,
		std::unique_ptr<JDownSampleDesc>&& desc,
		const std::vector<Core::JDataHandle>& handle, 
		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle)
		:JGraphicDownSampleComputeSet(context, blur, std::move(desc), handle),
		srcHandle(srcHandle),
		destHandle(destHandle)
	{
		SetValid(IsAllSameDevice(context, blur));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDownSampleComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicSsaoComputeSet::JDx12GraphicSsaoComputeSet(JCommandContext* context, JGraphicResourceShareData* shareData)
		:JGraphicSsaoComputeSet(context, shareData)
	{
		SetValid(IsAllSameDevice(context, shareData));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicSsaoComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicAAComputeSet::JDx12GraphicAAComputeSet(JCommandContext* context,JGraphicResourceShareData* shareData)
		:JGraphicAAComputeSet(context, shareData)
	{
		SetValid(IsAllSameDevice(context, shareData));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicAAComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicConvertColorComputeSet::JDx12GraphicConvertColorComputeSet(JCommandContext* context, const JConvertColorDesc& desc, JGraphicResourceInfo* srcInfo, JGraphicResourceInfo* destInfo)
		:JGraphicConvertColorComputeSet(context, desc), srcInfo(srcInfo), destInfo(destInfo)
	{
		SetValid(IsAllSameDevice(context) && srcInfo != nullptr && destInfo != nullptr);
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicConvertColorComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12PostProcessComputeSet::JDx12PostProcessComputeSet(JPostProcessEffectSet* ppSet, JCommandContext* context, JGraphicResourceManager* gm, JGraphicResourceShareData* shareData)
		:JPostProcessComputeSet(ppSet, context, gm, shareData)
	{
		SetValid(ppSet != nullptr && IsAllSameDevice(ppSet->tm, ppSet->convertColor, context, shareData));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12PostProcessComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicLightCullingTaskSet::JDx12GraphicLightCullingTaskSet(JCommandContext* context)
		:JGraphicLightCullingTaskSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicLightCullingTaskSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicLightCullingDebugDrawSet::JDx12GraphicLightCullingDebugDrawSet(JCommandContext* context)
		:JGraphicLightCullingDebugDrawSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicLightCullingDebugDrawSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicRtAoComputeSet::JDx12GraphicRtAoComputeSet(JCommandContext* context)
		:JGraphicRtAoComputeSet(context)
	{
		SetValid(IsAllSameDevice(context));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicRtAoComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicRtGiComputeSet::JDx12GraphicRtGiComputeSet(JCommandContext* context, JGraphicDevice* device)
		:JGraphicRtGiComputeSet(context), device(device)
	{
		SetValid(IsAllSameDevice(context, device));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicRtGiComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
 
	JDx12GraphicRtDenoiseComputeSet::JDx12GraphicRtDenoiseComputeSet(JCommandContext* context, JGraphicDevice* device, JGraphicResourceManager* gm, JGraphicResourceShareData* shareData)
		:JGraphicRtDenoiseComputeSet(context), device(device), gm(gm), shareData(shareData)
	{
		SetValid(IsAllSameDevice(context, device, gm, shareData));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicRtDenoiseComputeSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicInfoChangedSet::JDx12GraphicInfoChangedSet(const JGraphicInfo& preInfo,
		const JGraphicInfo& newInfo, 
		JGraphicDevice* device,
		JGraphicResourceManager* gm,
		JCullingManager* cm)
		: JGraphicInfoChangedSet(preInfo, newInfo), device(device), gm(gm), cm(cm)
	{
		SetValid(IsAllSameDevice(device, gm));
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicInfoChangedSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}

	JDx12GraphicOptionChangedSet::JDx12GraphicOptionChangedSet(const JGraphicOption& preOption,
		const JGraphicOption& newOption, 
		JGraphicDevice* device,
		JGraphicResourceManager* gm)
		: JGraphicOptionChangedSet(preOption, newOption), device(device), gm(gm)
	{
		SetValid(IsAllSameDevice(device, gm));
	} 
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicOptionChangedSet::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
}