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


#include"JGraphicDataSet.h"

namespace JinEngine::Graphic
{ 
	JGraphicDeviceInitSet::JGraphicDeviceInitSet(JGraphicResourceManager* graphicResourceM)
		:graphicResourceM(graphicResourceM)
	{}

	JPostProcessEffectSet::JPostProcessEffectSet(JToneMapping* tm,
		JBloom* bloom,
		JAntialise* aa,
		JPostProcessHistogram* histogram,
		JPostProcessExposure* exposure,
		JConvertColor* convertColor)
		: tm(tm), bloom(bloom), aa(aa), histogram(histogram), exposure(exposure), convertColor(convertColor)
	{ 
		SetValid(tm != nullptr && bloom != nullptr && histogram != nullptr && exposure != nullptr && convertColor != nullptr);
	}

	JGraphicDrawReferenceSet::JGraphicDrawReferenceSet(const JGraphicInfo& info,
		const JGraphicOption& option,
		JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		JGpuAcceleratorManager* acceleratorM,
		JFrameResource* currFrame,
		JGraphicDebug* depthDebug,
		JDepthTest* depthTest,
		JBlur* blur,
		JDownSampling* downSample,
		JSsao* ssao, 
		JPostProcessEffectSet* postSet,
		JGraphicResourceShareData* shareData, 
		const int currFrameIndex,
		const int nextFrameIndex)
		: info(info), 
		option(option),
		device(device),
		graphicResourceM(graphicResourceM),
		cullingM(cullingM),
		acceleratorM(acceleratorM),
		currFrame(currFrame),
		depthDebug(depthDebug),
		depthTest(depthTest),
		blur(blur),
		downSample(downSample),
		ssao(ssao), 
		postSet(postSet),
		shareData(shareData), 
		currFrameIndex(currFrameIndex),
		nextFrameIndex(nextFrameIndex)
	{
		if (device != nullptr && graphicResourceM != nullptr && cullingM != nullptr && currFrame != nullptr &&
			depthDebug != nullptr && depthTest != nullptr && blur != nullptr && downSample != nullptr && ssao != nullptr && 
			postSet->IsValid())
			SetValid(true);
		else
			SetValid(false);
	}

	JGraphicShaderCompileSet::JGraphicShaderCompileSet(JGraphicDevice* device)
		:device(device)
	{}
 
	JGraphicBindSet::JGraphicBindSet(JCommandContext* context)
		: context(context)
	{}
	 
	JGraphicDepthMapDrawSet::JGraphicDepthMapDrawSet(JCommandContext* context)
		: context(context)
	{}
	JGraphicDepthMapDrawSet::JGraphicDepthMapDrawSet(const JGraphicOccDrawSet* drawSet)
		: context(drawSet->context)
	{}
 
	JGraphicDebugRsComputeSet::JGraphicDebugRsComputeSet(JCommandContext* context)
		: context(context)
	{}	 

	JGraphicSceneDrawSet::JGraphicSceneDrawSet(JCommandContext* context)
		: context(context)
	{}

	JGraphicShadowMapDrawSet::JGraphicShadowMapDrawSet(JCommandContext* context, JBlur* blur)
		: context(context), blur(blur)
	{}

	JGraphicOccDrawSet::JGraphicOccDrawSet(JCommandContext* context, JDepthTest* depthTest)
		: context(context), depthTest(depthTest)
	{}
	 
	JGraphicHzbOccComputeSet::JGraphicHzbOccComputeSet(JCommandContext* context)
		: context(context)
	{}

	JGraphicHdOccExtractSet::JGraphicHdOccExtractSet(JCommandContext* context)
		: context(context)
	{}
 
	JGraphicOutlineDrawSet::JGraphicOutlineDrawSet(JCommandContext* context)
		: context(context)
	{}

	JGraphicBlurComputeSet::JGraphicBlurComputeSet(JCommandContext* context, std::unique_ptr<JBlurDesc>&& desc)
		: context(context), desc(std::move(desc))
	{}

	JGraphicDownSampleComputeSet::JGraphicDownSampleComputeSet(JCommandContext* context,
		JBlur* blur,
		std::unique_ptr<JDownSampleDesc>&& desc,
		const std::vector<Core::JDataHandle>& handle)
		: context(context), blur(blur), desc(std::move(desc)), handle(handle)
	{}

	JGraphicSsaoComputeSet::JGraphicSsaoComputeSet(JCommandContext* context, JGraphicResourceShareData* shareData)
		: context(context), shareData(shareData)
	{}
	 
	JGraphicAAComputeSet::JGraphicAAComputeSet(JCommandContext* context, JGraphicResourceShareData* shareData)
		: context(context), shareData(shareData)
	{}

	JGraphicConvertColorComputeSet::JGraphicConvertColorComputeSet(JCommandContext* context, const JConvertColorDesc& desc)
		: context(context), desc(desc)
	{}

	JPostProcessComputeSet::JPostProcessComputeSet(JPostProcessEffectSet* ppSet, JCommandContext* context, JGraphicResourceManager* gm, JGraphicResourceShareData* shareData)
		: ppSet(ppSet), context(context), gm(gm), shareData(shareData)
	{}

	JGraphicLightCullingTaskSet::JGraphicLightCullingTaskSet(JCommandContext* context)
		: context(context)
	{}

	JGraphicLightCullingDebugDrawSet::JGraphicLightCullingDebugDrawSet(JCommandContext* context)
		: context(context)
	{}

	JGraphicRtAoComputeSet::JGraphicRtAoComputeSet(JCommandContext* context)
		: context(context)
	{}

	JGraphicRtGiComputeSet::JGraphicRtGiComputeSet(JCommandContext* context)
		: context(context)
	{}

	JGraphicRtDenoiseComputeSet::JGraphicRtDenoiseComputeSet(JCommandContext* context)
		: context(context)
	{}
	 
	JGraphicEndConditonSet::JGraphicEndConditonSet(const bool isSceneDrawn)
		:isSceneDrawn(isSceneDrawn)
	{}
 
	JGraphicSubClassShareData::JGraphicSubClassShareData(JFrameIndexAccess* frameIndexAccess, PushGraphicEventPtr pushGraphicEventPtr)
		: frameIndexAccess(frameIndexAccess), pushGraphicEventPtr(pushGraphicEventPtr)
	{}

	JGraphicInfoChangedSet::JGraphicInfoChangedSet(const JGraphicInfo& preInfo, const JGraphicInfo& newInfo)
		: preInfo(preInfo), newInfo(newInfo)
	{}
	JGraphicOptionChangedSet::JGraphicOptionChangedSet(const JGraphicOption& preOption, const JGraphicOption& newOption)
		: preOption(preOption), newOption(newOption)
	{}
}