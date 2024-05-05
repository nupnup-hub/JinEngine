#pragma once
#include"JGraphicAdaptee.h"
#include"../JGraphicConstants.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../DataSet/JGraphicIndirectDataSet.h"
#include"../../Core/JCoreEssential.h" 

namespace JinEngine
{
	namespace Graphic
	{   
		class JGraphicAdapter
		{
		private:
			std::unique_ptr<JGraphicAdaptee> adaptee[(uint)J_GRAPHIC_DEVICE_TYPE::COUNT]; 
		public:
			~JGraphicAdapter();
		public:
			void Initialize(JCommandContextManager* manager, const J_GRAPHIC_DEVICE_TYPE deviceType);
			void Clear(const J_GRAPHIC_DEVICE_TYPE deviceType);
		public:
			void AddAdaptee(std::unique_ptr<JGraphicAdaptee>&& newAdaptee);
		public:
			std::unique_ptr<JGraphicDevice> CreateDevice(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData);
			void CreateResourceManageSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JResourceManageSubclassSet& set);
			void CreateDrawSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JDrawingSubclassSet& set);
			void CreateCullingSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JCullingSubclassSet& set);
			void CreateImageProcessingSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JImageProcessingSubclassSet& set);
			void CreateRaytracingSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JRaytracingSubclassSet& set);
		public:
			std::unique_ptr<JGraphicInfoChangedSet> CreateInfoChangedSet(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicInfo& preInfo, const JGraphicDrawReferenceSet& drawRefSet);
			std::unique_ptr<JGraphicOptionChangedSet> CreateOptionChangedSet(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicOption& preOption, const JGraphicDrawReferenceSet& drawRefSet);
		public:
			//called after update wait
			void BeginUpdateStart(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
		public: 
			//pre, post process(bind or set resource state...) --- main  thread
			void BeginDrawSceneSingleThread(const J_GRAPHIC_DEVICE_TYPE deviceType,const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet);
			void EndDrawSceneSingeThread(const J_GRAPHIC_DEVICE_TYPE deviceType,const JGraphicDrawReferenceSet& drawRefSet);
		public:
			//pre, post process(bind or set resource state...) --- child  thread
			void SettingBeginFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet);
			void ExecuteBeginFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
			void SettingMidFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMidFrameSet& dataSet);
			void ExecuteMidFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
			void SettingEndFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond);
			void ExecuteEndFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond);
		public:
			//create task set and notify task done --- child thread
			void SettingDrawOccTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet);
			void NotifyCompleteDrawOccTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex);
			void SettingDrawShadowMapTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadShadowMapTaskSet& dataSet);
			void NotifyCompleteDrawShadowMapTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex);
			void SettingDrawSceneTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadSceneTaskSet& dataSet);
			void NotifyCompleteDrawSceneTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex);
		public:
			//manage event handle --- main thread
			void ExecuteDrawOccTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
			void ExecuteDrawSceneTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
			void ExecuteDrawShadowMapTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
		public:
			//common task
			//begin, mid, end... 등 frame step중에 호출하는 task가 아닌
			//임의의 타이밍에 호출가능한 task함수
			/**
			* @brief Generate custom mipmap in modInfo and copy to srcInfo
			*/  
			bool BeginBlurTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBlurTaskSettingSet& set);
			void EndBlurTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
			bool BeginMipmapGenerationTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMipmapGenerationSettingSet& set);
			void EndMipmapGenerationTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
			bool BeginConvertColorTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicConvetColorSettingSet& set);
			void EndConvertColorTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet);
		private:
			JGraphicAdaptee* GetAdaptee(const J_GRAPHIC_DEVICE_TYPE deviceType);
		public:
			bool IsSameDevice(const J_GRAPHIC_DEVICE_TYPE deviceType);	 
		};
	}
}