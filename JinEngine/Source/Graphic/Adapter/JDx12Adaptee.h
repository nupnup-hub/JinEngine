#pragma once
#include"JGraphicAdaptee.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12Adaptee final : public JGraphicAdaptee
		{
		public:
			std::unique_ptr<JGraphicDevice> CreateDevice() final;
			std::unique_ptr<JGraphicResourceManager> CreateGraphicResourceManager() final;
			std::unique_ptr<JCullingManager> CreateCullingManager() final;
			void CreateFrameResource(_Out_ std::unique_ptr<JFrameResource>(&frame)[Constants::gNumFrameResources]) final;
		public:
			std::unique_ptr<JDepthMapDebug> CreateDepthMapDebug() final;
			std::unique_ptr<JDepthTest> CreateDepthTest() final;
			std::unique_ptr<JShadowMap> CreateShadowMapDraw() final;
			std::unique_ptr<JSceneDraw> CreateSceneDraw() final;
			std::unique_ptr<JHardwareOccCulling> CreateHdOcc() final;
			std::unique_ptr<JHZBOccCulling> CreateHzbOcc() final;
			std::unique_ptr<JOutline> CreateOutlineDraw() final;
		public:
			//main thread(use single thread)
			bool BeginDrawSceneSingleThread(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet) final;
			bool EndDrawSceneSingeThread(const JGraphicDrawReferenceSet& drawRefSet) final;
		public:
			//child thread
			bool SettingBeginFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet) final;
			bool ExecuteBeginFrame(const JGraphicDrawReferenceSet& drawRefSet)final;
			bool SettingMidFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMidFrameSet& dataSet) final;
			bool ExecuteMidFrame(const JGraphicDrawReferenceSet& drawRefSet) final;
			bool SettingEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond) final;
			bool ExecuteEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond) final;
		public:
			//child thread
			bool SettingDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet) final;
			bool NotifyCompleteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)final;
			bool SettingDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadShadowMapTaskSet& dataSet) final;
			bool NotifyCompleteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)final;
			bool SettingDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadSceneTaskSet& dataSet) final;
			bool NotifyCompleteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)final;
		public:
			//main thread(use multi thread)
			bool ExecuteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet) final; 
			bool ExecuteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet) final; 
			bool ExecuteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet) final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
	}
}