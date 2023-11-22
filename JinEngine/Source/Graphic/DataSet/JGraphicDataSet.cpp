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
		JDepthMapDebug* depthDebug,
		JDepthTest* depthTest,
		JImageProcessing* image)
		: info(info), 
		option(option),
		device(device),
		graphicResourceM(graphicResourceM),
		cullingM(cullingM),
		currFrame(currFrame),
		depthDebug(depthDebug),
		depthTest(depthTest),
		image(image)
	{
		if (device != nullptr && graphicResourceM != nullptr && cullingM != nullptr && currFrame != nullptr &&
			depthDebug != nullptr && depthTest != nullptr && image != nullptr)
			SetValid(true);
		else
			SetValid(false);
	}

	JGraphicBindSet::JGraphicBindSet(JFrameResource* currFrame, JGraphicResourceManager* graphicResourceM, JCullingManager* cullingManager)
		:currFrame(currFrame), graphicResourceM(graphicResourceM), cullingManager(cullingManager)
	{}
	 
	JGraphicDepthMapDrawSet::JGraphicDepthMapDrawSet(JFrameResource* currFrame, JGraphicResourceManager* graphicResourceM, JCullingManager* cullingM)
		: currFrame(currFrame), graphicResourceM(graphicResourceM), cullingM(cullingM)
	{}
	JGraphicDepthMapDrawSet::JGraphicDepthMapDrawSet(const JGraphicOccDrawSet* drawSet)
		: currFrame(drawSet->currFrame), graphicResourceM(drawSet->graphicResourceM), cullingM(drawSet->cullingM)
	{}
 
	JGraphicDepthMapDebugTaskSet::JGraphicDepthMapDebugTaskSet(JGraphicDevice* gDevice, JGraphicResourceManager* graphicResourceM)
		: gDevice(gDevice), graphicResourceM(graphicResourceM)
	{}
	JGraphicDepthMapDebugTaskSet::JGraphicDepthMapDebugTaskSet(JGraphicDevice* gDevice,
		JGraphicResourceManager* graphicResourceM,
		const JVector2<uint> size,
		const float nearF,
		const float farF,
		const bool isPerspective)
		: gDevice(gDevice), graphicResourceM(graphicResourceM), size(size), nearF(nearF), farF(farF), isPerspective(isPerspective)
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

	JGraphicOccDebugDrawSet::JGraphicOccDebugDrawSet(JGraphicDevice* device,
		JGraphicResourceManager* graphicResourceM,
		JDepthMapDebug* depthDebug)
		:device(device), graphicResourceM(graphicResourceM), depthDebug(depthDebug)
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

	JGraphicBlurTaskSet::JGraphicBlurTaskSet(JGraphicDevice* device, std::unique_ptr<JBlurDesc>&& desc)
		: device(device), desc(std::move(desc))
	{}

	JGraphicDownSampleTaskSet::JGraphicDownSampleTaskSet(JGraphicDevice* device, 
		JGraphicResourceManager* graphicResourceM, 
		std::unique_ptr<JDownSampleDesc>&& desc,
		std::vector<Core::JDataHandle>&& handle)
		: device(device), graphicResourceM(graphicResourceM), desc(std::move(desc)), handle(std::move(handle))
	{}

	JGraphicSsaoTaskSet::JGraphicSsaoTaskSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, JFrameResource* currFrame)
		: device(device), graphicResourceM(graphicResourceM), currFrame(currFrame)
	{}

	JGraphicEndConditonSet::JGraphicEndConditonSet(const bool isSceneDrawn)
		:isSceneDrawn(isSceneDrawn)
	{}
}