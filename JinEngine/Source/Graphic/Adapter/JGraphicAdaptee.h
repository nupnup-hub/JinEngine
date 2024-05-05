#pragma once
#include"../JGraphicConstants.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../DataSet/JGraphicIndirectDataSet.h" 
#include"../../Core/Pointer/JOwnerPtr.h" 

namespace JinEngine
{
	namespace Graphic
	{ 
		//Draw object
		class JGraphicResourceInfo;
		class JCommandContextManager;
		class JGraphicAdaptee : public JGraphicDeviceUser
		{
		public:
			virtual void Initialize(JCommandContextManager* manager) = 0;
			virtual void Clear() = 0;
		public:
			virtual std::unique_ptr<JGraphicDevice> CreateDevice(const JGraphicSubClassShareData& shareData) = 0;
			virtual void CreateResourceManageSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JResourceManageSubclassSet& set) = 0;
			virtual void CreateDrawSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JDrawingSubclassSet& set) = 0;
			virtual void CreateCullingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JCullingSubclassSet& set) = 0;
			virtual void CreateImageProcessingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JImageProcessingSubclassSet& set) = 0;
			virtual void CreateRaytracingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JRaytracingSubclassSet& set) = 0;
		public:
			virtual std::unique_ptr<JGraphicInfoChangedSet> CreateInfoChangedSet(const JGraphicInfo& preInfo, const JGraphicDrawReferenceSet& drawRefSet) = 0;
			virtual std::unique_ptr<JGraphicOptionChangedSet> CreateOptionChangedSet(const JGraphicOption& preOption, const JGraphicDrawReferenceSet& drawRefSet) = 0;
		public:
			virtual void BeginUpdateStart(const JGraphicDrawReferenceSet& drawRefSet) = 0;
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
			virtual bool BeginBlurTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBlurTaskSettingSet& set) = 0;
			virtual void EndBlurTask(const JGraphicDrawReferenceSet& drawRefSet) = 0;
			virtual bool BeginMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMipmapGenerationSettingSet& set) = 0;
			virtual void EndMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet) = 0; 
			virtual bool BeginConvertColorTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicConvetColorSettingSet& set) = 0;
			virtual void EndConvertColorTask(const JGraphicDrawReferenceSet& drawRefSet) = 0;
		};
	}
}