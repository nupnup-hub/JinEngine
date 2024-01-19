#include"JGraphicDataSet.h"

namespace JinEngine::Graphic
{
	JGraphicBaseDataSet::JGraphicBaseDataSet(const JGraphicInfo& info, const JGraphicOption& option)
		:info(info), option(option)
	{}

	JGraphicDeviceInitSet::JGraphicDeviceInitSet(const JGraphicInfo& info, const JGraphicOption& option, JGraphicResourceManager* graphicResourceM)
		: info(info), option(option), graphicResourceM(graphicResourceM)
	{}

	JGraphicDrawReferenceSet::JGraphicDrawReferenceSet(const JGraphicInfo& info,
		const JGraphicOption& option,
		JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		JFrameResource* currFrame,
		JGraphicDebug* depthDebug,
		JDepthTest* depthTest,
		JImageProcessing* image,
		const int currFrameIndex,
		const int nextFrameIndex)
		: info(info), 
		option(option),
		device(device),
		graphicResourceM(graphicResourceM),
		cullingM(cullingM),
		currFrame(currFrame),
		depthDebug(depthDebug),
		depthTest(depthTest),
		image(image),
		currFrameIndex(currFrameIndex),
		nextFrameIndex(nextFrameIndex)
	{
		if (device != nullptr && graphicResourceM != nullptr && cullingM != nullptr && currFrame != nullptr &&
			depthDebug != nullptr && depthTest != nullptr && image != nullptr)
			SetValid(true);
		else
			SetValid(false);
	}

	JGraphicShaderCompileSet::JGraphicShaderCompileSet(JGraphicDevice* device, const JGraphicBaseDataSet& base)
		:device(device), base(base)
	{}
	JGraphicShaderCompileSet::JGraphicShaderCompileSet(JGraphicDevice* device, const JGraphicInfo& info, const JGraphicOption& option)
		: device(device), base(JGraphicBaseDataSet(info, option))
	{}

	JGraphicBindSet::JGraphicBindSet(JFrameResource* currFrame, JGraphicResourceManager* graphicResourceM, JCullingManager* cullingManager)
		:currFrame(currFrame), graphicResourceM(graphicResourceM), cullingManager(cullingManager)
	{}
	 
	JGraphicDepthMapDrawSet::JGraphicDepthMapDrawSet(JFrameResource* currFrame, JGraphicResourceManager* graphicResourceM, JCullingManager* cullingM)
		: currFrame(currFrame), graphicResourceM(graphicResourceM), cullingM(cullingM)
	{}
	JGraphicDepthMapDrawSet::JGraphicDepthMapDrawSet(const JGraphicOccDrawSet* drawSet)
		: currFrame(drawSet->currFrame), graphicResourceM(drawSet->graphicResourceM), cullingM(drawSet->cullingM)
	{}
 
	JGraphicDebugRsComputeSet::JGraphicDebugRsComputeSet(JGraphicDevice* gDevice, JGraphicResourceManager* graphicResourceM)
		: device(device), graphicResourceM(graphicResourceM)
	{}	 

	JGraphicSceneDrawSet::JGraphicSceneDrawSet(JGraphicDevice* device,
		JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM)
		:device(device), currFrame(currFrame), graphicResourceM(graphicResourceM), cullingM(cullingM)
	{}

	JGraphicShadowMapDrawSet::JGraphicShadowMapDrawSet(JGraphicDevice* device,
		JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		JImageProcessing* image)
		:device(device), currFrame(currFrame), graphicResourceM(graphicResourceM), cullingM(cullingM), image(image)
	{}

	JGraphicOccDrawSet::JGraphicOccDrawSet(JGraphicDevice* device,
		JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM,
		JDepthTest* depthTest)
		:device(device),
		currFrame(currFrame),
		graphicResourceM(graphicResourceM),
		cullingM(cullingM),
		depthTest(depthTest)
	{}
	 
	JGraphicHzbOccComputeSet::JGraphicHzbOccComputeSet(JFrameResource* currFrame,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingM)
		:currFrame(currFrame),
		graphicResourceM(graphicResourceM),
		cullingM(cullingM)
	{}

	JGraphicHdOccExtractSet::JGraphicHdOccExtractSet(JCullingManager* cullingM)
		:cullingM(cullingM)
	{}
 
	JGraphicOutlineDrawSet::JGraphicOutlineDrawSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM)
		: device(device), graphicResourceM(graphicResourceM)
	{}

	JGraphicBlurComputeSet::JGraphicBlurComputeSet(JGraphicDevice* device, std::unique_ptr<JBlurDesc>&& desc)
		: device(device), desc(std::move(desc))
	{}

	JGraphicDownSampleComputeSet::JGraphicDownSampleComputeSet(JGraphicDevice* device, 
		JGraphicResourceManager* graphicResourceM, 
		std::unique_ptr<JDownSampleDesc>&& desc,
		std::vector<Core::JDataHandle>&& handle)
		: device(device), graphicResourceM(graphicResourceM), desc(std::move(desc)), handle(std::move(handle))
	{}

	JGraphicSsaoComputeSet::JGraphicSsaoComputeSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, JFrameResource* currFrame)
		: device(device), graphicResourceM(graphicResourceM), currFrame(currFrame)
	{}

	JGraphicLightCullingTaskSet::JGraphicLightCullingTaskSet(JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM,
		JCullingManager* cullingManager,
		JFrameResource* currFrame)
		: device(device), graphicResourceM(graphicResourceM), cullingManager(cullingManager), currFrame(currFrame)
	{}

	JGraphicLightCullingDebugDrawSet::JGraphicLightCullingDebugDrawSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM)
		: device(device), graphicResourceM(graphicResourceM)
	{}

	JGraphicEndConditonSet::JGraphicEndConditonSet(const bool isSceneDrawn)
		:isSceneDrawn(isSceneDrawn)
	{}
}