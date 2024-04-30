#pragma once
#include"JGraphicAdaptee.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12Adaptee final : public JGraphicAdaptee
		{
		public:
			void Initialize(JCommandContextManager* manager) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			std::unique_ptr<JGraphicDevice> CreateDevice(const JGraphicSubClassShareData& shareData) final;
			void CreateResourceManageSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JResourceManageSubclassSet& set) final;
			void CreateDrawSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JDrawingSubclassSet& set) final;
			void CreateCullingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JCullingSubclassSet& set) final;
			void CreateImageProcessingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JImageProcessingSubclassSet& set) final;
			void CreateRaytracingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JRaytracingSubclassSet& set) final;
		public:		
			std::unique_ptr<JGraphicInfoChangedSet> CreateInfoChangedSet(const JGraphicInfo& preInfo, const JGraphicDrawReferenceSet& drawRefSet)final;
			std::unique_ptr<JGraphicOptionChangedSet> CreateOptionChangedSet(const JGraphicOption& preOption, const JGraphicDrawReferenceSet& drawRefSet)final;
		public:
			void BeginUpdateStart(const JGraphicDrawReferenceSet& drawRefSet) final;
		public:
			//pre, post process(bind or set resource state...) --- main  thread
			bool BeginDrawSceneSingleThread(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet) final;
			bool EndDrawSceneSingeThread(const JGraphicDrawReferenceSet& drawRefSet) final;
		public:
			//pre, post process(bind or set resource state...) --- child  thread
			bool SettingBeginFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet) final;
			bool ExecuteBeginFrame(const JGraphicDrawReferenceSet& drawRefSet)final;
			bool SettingMidFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMidFrameSet& dataSet) final;
			bool ExecuteMidFrame(const JGraphicDrawReferenceSet& drawRefSet) final;
			bool SettingEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond) final;
			bool ExecuteEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond) final;
		public:
			//create task set and notify task done --- child thread
			bool SettingDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet) final;
			bool NotifyCompleteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)final;
			bool SettingDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadShadowMapTaskSet& dataSet) final;
			bool NotifyCompleteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)final;
			bool SettingDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadSceneTaskSet& dataSet) final;
			bool NotifyCompleteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)final;
		public:
			//manage event handle --- main thread
			bool ExecuteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet) final; 
			bool ExecuteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet) final; 
			bool ExecuteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet) final;
		public:
			//use public command
			bool BeginBlurTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBlurTaskSettingSet& set)final;
			void EndBlurTask(const JGraphicDrawReferenceSet& drawRefSet)final;
			bool BeginMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMipmapGenerationSettingSet& set)final;
			void EndMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet)final; 
			bool BeginConvertColorTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicConvetColorSettingSet& set)final;
			void EndConvertColorTask(const JGraphicDrawReferenceSet& drawRefSet)final;
		};
	}
}