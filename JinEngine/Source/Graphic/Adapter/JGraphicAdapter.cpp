#include"JGraphicAdapter.h"   
#include"../Accelerator/JGpuAcceleratorManager.h"
#include"../Debug/JGraphicDebug.h"
#include"../DepthMap/JDepthTest.h"
#include"../Device/JGraphicDevice.h"
#include"../GraphicResource/JGraphicResourceManager.h" 
#include"../GraphicResource/JGraphicResourceShareData.h" 
#include"../Outline/JOutline.h"
#include"../Culling/JCullingManager.h"
#include"../Culling/Occlusion/JHardwareOccCulling.h"
#include"../Culling/Occlusion/JHZBOccCulling.h"
#include"../Culling/Light/JLightCulling.h"
#include"../Image/JBlur.h"
#include"../Image/JDownSampling.h"
#include"../Image/JSsao.h"
#include"../Image/JToneMapping.h"
#include"../Image/JBloom.h" 
#include"../Image/JAntialise.h" 
#include"../Image/JPostProcessHistogram.h" 
#include"../Image/JPostProcessExposure.h" 
#include"../Image/JConvertColor.h"
#include"../Raytracing/Occlusion/JRaytracingAmbientOcclusion.h" 
#include"../Scene/JSceneDraw.h"
#include"../ShadowMap/JShadowMap.h" 
#include"../FrameResource/JFrameResource.h" 

namespace JinEngine::Graphic
{ 
	JGraphicAdapter::~JGraphicAdapter()
	{
		for (uint i = 0; i < (uint)J_GRAPHIC_DEVICE_TYPE::COUNT; ++i)
			Clear((J_GRAPHIC_DEVICE_TYPE)i);
	}
	void JGraphicAdapter::Initialize(JCommandContextManager* manager, const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->Initialize(manager);
	}
	void JGraphicAdapter::Clear(const J_GRAPHIC_DEVICE_TYPE deviceType)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->Clear();
	}
	void JGraphicAdapter::AddAdaptee(std::unique_ptr<JGraphicAdaptee>&& newAdaptee)
	{
		if (newAdaptee != nullptr)
		{
			const uint index = (uint)newAdaptee->GetDeviceType();
			adaptee[index] = std::move(newAdaptee);
		}
	}
	std::unique_ptr<JGraphicDevice> JGraphicAdapter::CreateDevice(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;
 
		return GetAdaptee(deviceType)->CreateDevice(shareData);
	}
	void JGraphicAdapter::CreateResourceManageSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JResourceManageSubclassSet& set)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->CreateResourceManageSubclass(shareData, set);
	}
	void JGraphicAdapter::CreateDrawSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JDrawingSubclassSet& set)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->CreateDrawSubclass(shareData, set);
	}
	void JGraphicAdapter::CreateCullingSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JCullingSubclassSet& set)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->CreateCullingSubclass(shareData, set);
	}
	void JGraphicAdapter::CreateImageProcessingSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JImageProcessingSubclassSet& set)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->CreateImageProcessingSubclass(shareData, set);
	}
	void JGraphicAdapter::CreateRaytracingSubclass(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicSubClassShareData& shareData, _Inout_ JRaytracingSubclassSet& set)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->CreateRaytracingSubclass(shareData, set);
	}
	std::unique_ptr<JGraphicInfoChangedSet> JGraphicAdapter::CreateInfoChangedSet(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicInfo& preInfo, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;

		return GetAdaptee(deviceType)->CreateInfoChangedSet(preInfo, drawRefSet);
	}
	std::unique_ptr<JGraphicOptionChangedSet> JGraphicAdapter::CreateOptionChangedSet(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicOption& preOption, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return nullptr;

		return GetAdaptee(deviceType)->CreateOptionChangedSet(preOption, drawRefSet);
	}
	void JGraphicAdapter::BeginUpdateStart(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		return GetAdaptee(deviceType)->BeginUpdateStart(drawRefSet);
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
	bool JGraphicAdapter::BeginBlurTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBlurTaskSettingSet& set)
	{
		if (!IsSameDevice(deviceType))
			return false;

		return GetAdaptee(deviceType)->BeginBlurTask(drawRefSet, set);
	}
	void JGraphicAdapter::EndBlurTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->EndBlurTask(drawRefSet);
	}
	bool JGraphicAdapter::BeginMipmapGenerationTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMipmapGenerationSettingSet& set)
	{
		if (!IsSameDevice(deviceType))
			return false;

		return GetAdaptee(deviceType)->BeginMipmapGenerationTask(drawRefSet, set);
	}
	void JGraphicAdapter::EndMipmapGenerationTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->EndMipmapGenerationTask(drawRefSet);
	} 
	bool JGraphicAdapter::BeginConvertColorTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicConvetColorSettingSet& set)
	{
		if (!IsSameDevice(deviceType))
			return false;

		return GetAdaptee(deviceType)->BeginConvertColorTask(drawRefSet, set);
	}
	void JGraphicAdapter::EndConvertColorTask(const J_GRAPHIC_DEVICE_TYPE deviceType, const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsSameDevice(deviceType))
			return;

		GetAdaptee(deviceType)->EndConvertColorTask(drawRefSet);
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