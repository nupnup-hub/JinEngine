#include"JGraphicAdapter.h"  
#include"../Culling/JCullingManager.h"
#include"../DepthMap/JDepthMapDebug.h"
#include"../DepthMap/JDepthTest.h"
#include"../Device/JGraphicDevice.h"
#include"../GraphicResource/JGraphicResourceManager.h" 
#include"../Outline/JOutline.h"
#include"../Culling/Occlusion/JHardwareOccCulling.h"
#include"../Culling/Occlusion/JHZBOccCulling.h"
#include"../Scene/JSceneDraw.h"
#include"../ShadowMap/JShadowMap.h" 
#include"../FrameResource/JFrameResource.h" 

namespace JinEngine::Graphic
{  
	void JGraphicAdapter::AddAdaptee(std::unique_ptr<JGraphicAdaptee>&& newAdaptee)
	{
		if (newAdaptee != nullptr)
		{
			const uint index = (uint)newAdaptee->GetDeviceType();
			adaptee[index] = std::move(newAdaptee);
		}
	}
	std::unique_ptr<JGraphicDevice> JGraphicAdapter::CreateDevice(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
 
		return GetAdaptee(deviceType)->CreateDevice();
	}
	std::unique_ptr<JGraphicResourceManager> JGraphicAdapter::CreateGraphicResourceManager(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateGraphicResourceManager();
	}
	std::unique_ptr<JCullingManager> JGraphicAdapter::CreateCullingManager(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateCullingManager();
	}
	bool JGraphicAdapter::CreateFrameResource(const J_GRAPHIC_DEVICE_TYPE deviceType, _Out_ std::unique_ptr<JFrameResource> (&frame)[Constants::gNumFrameResources])
	{
		if (!IsSameDevice(deviceType))
			return false;

		GetAdaptee(deviceType)->CreateFrameResource(frame);
		bool isSuccess = true;
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			isSuccess = isSuccess && frame[i] != nullptr;
		return isSuccess;
	}
	std::unique_ptr<JDepthMapDebug> JGraphicAdapter::CreateDepthMapDebug(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateDepthMapDebug();
	}
	std::unique_ptr<JDepthTest> JGraphicAdapter::CreateDepthTest(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateDepthTest();
	}
	std::unique_ptr<JShadowMap> JGraphicAdapter::CreateShadowMapDraw(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateShadowMapDraw();
	}
	std::unique_ptr<JSceneDraw> JGraphicAdapter::CreateSceneDraw(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateSceneDraw();
	}
	std::unique_ptr<JHardwareOccCulling> JGraphicAdapter::CreateHdOcc(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateHdOcc();
	}
	std::unique_ptr<JHZBOccCulling> JGraphicAdapter::CreateHzbOcc(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
 
		return GetAdaptee(deviceType)->CreateHzbOcc();
	}
	std::unique_ptr<JOutline> JGraphicAdapter::CreateOutlineDraw(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
		 
		return GetAdaptee(deviceType)->CreateOutlineDraw();
	}  
	void JGraphicAdapter::BeginDrawSceneSingleThread(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->BeginDrawSceneSingleThread(drawRefSet, dataSet);
	}
	void JGraphicAdapter::EndDrawSceneSingeThread(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->EndDrawSceneSingeThread(drawRefSet);
	}
	void JGraphicAdapter::SettingBeginFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->SettingBeginFrame(drawRefSet, dataSet);
	}
	void JGraphicAdapter::ExecuteBeginFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->ExecuteBeginFrame(drawRefSet);
	}
	void JGraphicAdapter::SettingMidFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMidFrameSet& dataSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->SettingMidFrame(drawRefSet, dataSet);
	}
	void JGraphicAdapter::ExecuteMidFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->ExecuteMidFrame(drawRefSet);
	}
	void JGraphicAdapter::SettingEndFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->SettingEndFrame(drawRefSet, cond);
	}
	void JGraphicAdapter::ExecuteEndFrame(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->ExecuteEndFrame(drawRefSet, cond);
	}
	void JGraphicAdapter::SettingDrawOccTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->SettingDrawOccTask(drawRefSet, threadIndex, dataSet);
	}
	void JGraphicAdapter::NotifyCompleteDrawOccTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->NotifyCompleteDrawOccTask(drawRefSet, threadIndex);
	}
	void JGraphicAdapter::SettingDrawShadowMapTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadShadowMapTaskSet& dataSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->SettingDrawShadowMapTask(drawRefSet, threadIndex, dataSet);
	}
	void JGraphicAdapter::NotifyCompleteDrawShadowMapTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->NotifyCompleteDrawShadowMapTask(drawRefSet, threadIndex);
	}
	void JGraphicAdapter::SettingDrawSceneTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadSceneTaskSet& dataSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->SettingDrawSceneTask(drawRefSet, threadIndex, dataSet);
	}
	void JGraphicAdapter::NotifyCompleteDrawSceneTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->NotifyCompleteDrawSceneTask(drawRefSet, threadIndex);
	}
	void JGraphicAdapter::ExecuteDrawOccTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->ExecuteDrawOccTask(drawRefSet);
	}
	void JGraphicAdapter::ExecuteDrawShadowMapTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->ExecuteDrawShadowMapTask(drawRefSet);
	}
	void JGraphicAdapter::ExecuteDrawSceneTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->ExecuteDrawSceneTask(drawRefSet);
	}
	JGraphicAdaptee* JGraphicAdapter::GetAdaptee(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		return adaptee[(uint)deviceType].get();
	}
	bool JGraphicAdapter::IsSameDevice(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		return adaptee[(uint)deviceType] != nullptr;
	}
}