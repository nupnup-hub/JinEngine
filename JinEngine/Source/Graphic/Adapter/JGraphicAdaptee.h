#pragma once
#include"../JGraphicConstants.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include<memory>

namespace JinEngine
{
	namespace Graphic
	{
		//Resource manage
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JCullingManager;
		class JFrameResource;

		//Draw object
		class JDepthMapDebug;
		class JDepthTest;
		class JShadowMap;
		class JSceneDraw;
		class JHardwareOccCulling;
		class JHZBOccCulling;
		class JOutline;
		class JImageProcessing;
		class JGraphicResourceInfo;

		class JGraphicAdaptee : public JGraphicDeviceUser
		{
		public:
			virtual std::unique_ptr<JGraphicDevice> CreateDevice() = 0;
			virtual std::unique_ptr<JGraphicResourceManager> CreateGraphicResourceManager() = 0;
			virtual std::unique_ptr<JCullingManager> CreateCullingManager() = 0;
			virtual void CreateFrameResource(_Out_ std::unique_ptr<JFrameResource>(&frame)[Constants::gNumFrameResources]) = 0;
		public:
			virtual std::unique_ptr<JDepthMapDebug> CreateDepthMapDebug() = 0;
			virtual std::unique_ptr<JDepthTest> CreateDepthTest() = 0;
			virtual std::unique_ptr<JShadowMap> CreateShadowMapDraw() = 0;
			virtual std::unique_ptr<JSceneDraw> CreateSceneDraw() = 0;
			virtual std::unique_ptr<JHardwareOccCulling> CreateHdOcc() = 0;
			virtual std::unique_ptr<JHZBOccCulling> CreateHzbOcc() = 0;
			virtual std::unique_ptr<JOutline> CreateOutlineDraw() = 0;
			virtual std::unique_ptr<JImageProcessing> CreateImageProcessing() = 0;
		public:
			//pre, post process(bind or set resource state...) --- main  thread
			virtual bool BeginDrawSceneSingleThread(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet) = 0;
			virtual bool EndDrawSceneSingeThread(const JGraphicDrawReferenceSet& drawRefSet) = 0;
		public:
			//pre, post process(bind or set resource state...) --- child  thread
			virtual bool SettingBeginFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet) = 0;
			virtual bool ExecuteBeginFrame(const JGraphicDrawReferenceSet& drawRefSet) = 0;
			virtual bool SettingMidFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMidFrameSet& dataSet) = 0;
			virtual bool ExecuteMidFrame(const JGraphicDrawReferenceSet& drawRefSet) = 0;
			virtual bool SettingEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond) = 0;
			virtual bool ExecuteEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond) = 0;
		public:
			//create task set and notify task done --- child thread
			virtual bool SettingDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet) = 0;
			virtual bool NotifyCompleteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex) = 0;
			virtual bool SettingDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadShadowMapTaskSet& dataSet) = 0;
			virtual bool NotifyCompleteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex) = 0;
			virtual bool SettingDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadSceneTaskSet& dataSet) = 0;
			virtual bool NotifyCompleteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex) = 0;
		public:
			//manage event handle --- main thread
			virtual bool ExecuteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet) = 0; 
			virtual bool ExecuteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet) = 0; 
			virtual bool ExecuteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet) = 0;
		public:
			//common
			virtual bool SettingBlurTask(const JGraphicDrawReferenceSet& drawRefSet,
				const ResourceHandle from,
				const ResourceHandle to,
				std::unique_ptr<JBlurDesc>&& desc,
				_Out_ std::unique_ptr<JGraphicBlurTaskSet>& dataSet) = 0;
			virtual bool SettingBlurTask(const JGraphicDrawReferenceSet& drawRefSet,
				const JUserPtr<JGraphicResourceInfo>& info,
				std::unique_ptr<JBlurDesc>&& desc,
				_Out_ std::unique_ptr<JGraphicBlurTaskSet>& dataSet) = 0;
			virtual bool SettingMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet,
				const JUserPtr<JGraphicResourceInfo>& srcInfo,
				const JUserPtr<JGraphicResourceInfo>& modInfo,
				std::unique_ptr<JDownSampleDesc>&& desc,
				_Out_ std::unique_ptr<JGraphicDownSampleTaskSet>& dataSet) = 0; 
		};
	}
}