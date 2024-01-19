#pragma once
#include"JGraphicAdaptee.h"
#include"../JGraphicConstants.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../../Core/JCoreEssential.h"
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
		class JGraphicDebug;
		class JDepthTest;
		class JShadowMap;
		class JSceneDraw;
		class JHardwareOccCulling;
		class JHZBOccCulling;
		class JLightCulling;
		class JOutline;
		class JImageProcessing;

		class JGraphicAdapter
		{
		private:
			std::unique_ptr<JGraphicAdaptee> adaptee[(uint)J_GRAPHIC_DEVICE_TYPE::COUNT]; 
		public:
			void AddAdaptee(std::unique_ptr<JGraphicAdaptee>&& newAdaptee);
		public:
			std::unique_ptr<JGraphicDevice> CreateDevice(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JGraphicResourceManager> CreateGraphicResourceManager(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JCullingManager> CreateCullingManager(const J_GRAPHIC_DEVICE_TYPE deviceType);
			bool CreateFrameResource(const J_GRAPHIC_DEVICE_TYPE deviceType, _Out_ std::unique_ptr<JFrameResource>(&frame)[Constants::gNumFrameResources]);
		public:
			std::unique_ptr<JGraphicDebug> CreateDebug(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JDepthTest> CreateDepthTest(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JShadowMap> CreateShadowMapDraw(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JSceneDraw> CreateSceneDraw(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JHardwareOccCulling> CreateHdOcc(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JHZBOccCulling> CreateHzbOcc(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JLightCulling> CreateLightCulling(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JOutline> CreateOutlineDraw(const J_GRAPHIC_DEVICE_TYPE deviceType);
			std::unique_ptr<JImageProcessing> CreateImageProcessing(const J_GRAPHIC_DEVICE_TYPE deviceType);
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
			//common 
			/**
			* @brief Generate custom mipmap in modInfo and copy to srcInfo
			*/ 
			bool SettingBlurTask(const J_GRAPHIC_DEVICE_TYPE deviceType,
				const JGraphicDrawReferenceSet& drawRefSet,
				const ResourceHandle from,
				const ResourceHandle to,
				std::unique_ptr<JBlurDesc>&& desc,
				_Out_ std::unique_ptr<JGraphicBlurComputeSet>& dataSet);
			bool SettingBlurTask(const J_GRAPHIC_DEVICE_TYPE deviceType,
				const JGraphicDrawReferenceSet& drawRefSet,
				const JUserPtr<JGraphicResourceInfo>& info,
				std::unique_ptr<JBlurDesc>&& desc,
				_Out_ std::unique_ptr<JGraphicBlurComputeSet>& dataSet);
			bool SettingMipmapGenerationTask(const J_GRAPHIC_DEVICE_TYPE deviceType, 
				const JGraphicDrawReferenceSet& drawRefSet,
				const JUserPtr<JGraphicResourceInfo>& srcInfo,
				const JUserPtr<JGraphicResourceInfo>& modInfo,
				std::unique_ptr<JDownSampleDesc>&& desc,
				_Out_ std::unique_ptr<JGraphicDownSampleComputeSet>& dataSet);
		private:
			JGraphicAdaptee* GetAdaptee(const J_GRAPHIC_DEVICE_TYPE deviceType);
		public:
			bool IsSameDevice(const J_GRAPHIC_DEVICE_TYPE deviceType);	 
		};
	}
}