#include"JGraphicDataSet.h"

namespace JinEngine::Graphic
{
	JGraphicBaseDataSet::JGraphicBaseDataSet(const JGraphicInfo& info, const JGraphicOption& option)
		:info(info), option(option)
	{}
	JGraphicDeviceInitSet::JGraphicDeviceInitSet(const JGraphicInfo& info, const JGraphicOption& option, JGraphicResourceManager* gResourceM)
		: info(info), option(option), gResourceM(gResourceM)
	{}
	JGraphicDrawReferenceSet::JGraphicDrawReferenceSet(const JGraphicInfo& info,
		const JGraphicOption& option,
		JGraphicDevice* device,
		JGraphicResourceManager* gResourceM,
		JCullingManager* cullingM,
		JFrameResource* currFrame,
		JDepthMapDebug* depthDebug,
		JDepthTest* depthTest,
		JBlur* blur)
		: info(info), 
		option(option),
		device(device),
		gResourceM(gResourceM),
		cullingM(cullingM),
		currFrame(currFrame),
		depthDebug(depthDebug),
		depthTest(depthTest),
		blur(blur)
	{
		if (device != nullptr && gResourceM != nullptr && cullingM != nullptr && currFrame != nullptr &&
			depthDebug != nullptr && depthTest != nullptr && blur != nullptr)
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
	{

	}

	JGraphicDepthMapDebugObjectSet::JGraphicDepthMapDebugObjectSet(JGraphicResourceManager* graphicResourceM)
		: graphicResourceM(graphicResourceM)
	{}	 

	JGraphicDepthMapDebugHandleSet::JGraphicDepthMapDebugHandleSet(const JVector2<uint> size, const float nearF, const float farF, const bool isPerspective)
		: size(size), nearF(nearF), farF(farF), isPerspective(isPerspective)
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
		JBlur* blur)
		:device(device), currFrame(currFrame), graphicResourceM(graphicResourceM), cullingM(cullingM), blur(blur)
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

	JGraphicOutlineObjectSet::JGraphicOutlineObjectSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM)
		: device(device), graphicResourceM(graphicResourceM)
	{}

	JGraphicOutlineHandleSet::JGraphicOutlineHandleSet(JGraphicResourceManager* graphicResourceM)
		: graphicResourceM(graphicResourceM)
	{}

	JGraphicBlurTaskSet::JGraphicBlurTaskSet(JGraphicDevice* device, std::unique_ptr<JBlurDesc>&& desc)
		: device(device), desc(std::move(desc))
	{}

	JGraphicEndConditonSet::JGraphicEndConditonSet(const bool isSceneDrawn)
		:isSceneDrawn(isSceneDrawn)
	{}
}