#include"JGraphicUpdateHelper.h"
#include"JGraphicInfo.h"
#include"JGraphicOption.h"
#include"JGraphicDrawList.h"
#include"Culling/JCullingInterface.h"
#include"../Object/Component/JComponent.h"
#include"../Object/Component/Camera/JCamera.h"
#include"../Object/Component/Camera/JCameraPrivate.h"
#include"../Object/Component/Light/JLight.h" 

namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
			using CamEditorSettingInterface = JCameraPrivate::EditorSettingInterface;
			using CamFrameIndexInterface = JCameraPrivate::FrameIndexInterface;
		}

		bool JUpdateHelper::BindingTextureData::HasCallable()const noexcept
		{
			return hasCallable;
		}

		void JUpdateHelper::Clear()
		{
			const uint fVCount = (uint)uData.size();
			for (uint i = 0; i < fVCount; ++i)
			{
				uData[i].count = 0;
				uData[i].capacity = 0; 
				uData[i].offset = 0;
				//uData[i].setDirty = false;
				if (uData[i].setDirty > 0)
					--uData[i].setDirty;
				uData[i].rebuildCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
			}

			const uint bVCount = (uint)bData.size();
			for (uint i = 0; i < bVCount; ++i)
			{
				bData[i].count = 0;
				bData[i].capacity = 0;
				bData[i].recompileCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
			}
			hasRebuildCondition = false;
			hasRecompileShader = false;
		}
		void JUpdateHelper::RegisterCallable(J_UPLOAD_FRAME_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr)
		{
			if (getCountPtr == nullptr)
				return;

			uData[(int)type].getElement = std::make_unique<GetElementCountT::Callable>(*getCountPtr);
			//uData[(int)type].getElementCapacityCallable = std::make_unique<GetElementCapacityT::Callable>(*getCapaPtr);
			//uData[(int)type].rebuildCallable = std::make_unique<RebuildT::Callable>(*rPtr);
		}
		void JUpdateHelper::RegisterCallable(J_GRAPHIC_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr, GetElementCapacityT::Ptr* getCapaPtr, SetCapacityT::Ptr* sPtr)
		{
			if (getCountPtr == nullptr || getCapaPtr == nullptr || sPtr == nullptr)
				return;

			bData[(int)type].getTextureCount = std::make_unique<GetElementCountT::Callable>(*getCountPtr);
			bData[(int)type].getTextureCapacity = std::make_unique< GetElementCapacityT::Callable>(*getCapaPtr);
			bData[(int)type].setCapacity = std::make_unique<SetCapacityT::Callable>(*sPtr);
			bData[(int)type].hasCallable = true;
		}
		void JUpdateHelper::RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE type, std::unique_ptr<NotifyUpdateCapacityT::Callable>&& listner)
		{
			uData[(int)type].notifyUpdateCapacity.push_back(std::move(listner));
		}
		void JUpdateHelper::WriteGraphicInfo(JGraphicInfo& info)const noexcept
		{
			info.upObjCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].count;
			info.upPassCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::PASS].count;
			info.upAniCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].count;
			info.upCameraCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].count;
			info.upLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT].count;
			info.upSmLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT].count;
			info.upMaterialCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].count;

			info.upObjCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].capacity;
			info.upPassCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::PASS].capacity;
			info.upAniCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].capacity;
			info.upCameraCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].capacity;
			info.upLightCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT].capacity;
			info.upSmLightCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT].capacity;
			info.upMaterialCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].capacity;

			info.binding2DTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].count;
			info.bindingCubeMapCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].count;
			info.bindingShadowTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].count;

			info.binding2DTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].capacity;
			info.bindingCubeMapCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].capacity;
			info.bindingShadowTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].capacity;
		}
		void JUpdateHelper::NotifyUpdateFrameCapacity(JGraphic& grpahic)
		{
			for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
			{
				if (uData[i].rebuildCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
				{
					const uint listenerCount = (uint)uData[i].notifyUpdateCapacity.size();
					for (uint j = 0; j < listenerCount; ++j)
						(*uData[i].notifyUpdateCapacity[j])(nullptr);
				}
			}
		}
		
		bool JDrawHelper::RefelectOtherCamCullig(const uint rItemIndex)const noexcept
		{
			if (cam == nullptr)
				false;
			 
			const size_t guid = cam->GetGuid();
			for (const auto& data : drawTarget->sceneRequestor)
			{ 
				if(guid == data->jCamera->GetGuid())
					continue;

				auto cullingInterface = data->jCamera->CullingUserInterface(); 
				if (cullingInterface.UnsafeIsCulled(rItemIndex))
					return true;
			}
			return false;
		}
		void JDrawHelper::SettingOccCulling(const JUserPtr<JComponent>& comp)noexcept
		{
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				cam = Core::ConnectChildUserPtr<JCamera>(comp);
				cullUser = cam->CullingUserInterface();
				allowDrawOccMipMap = cam->AllowDisplayOccCullingDepthMap();
			}
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			{
				lit = Core::ConnectChildUserPtr<JLight>(comp);
				cullUser = lit->CullingUserInterface();
			}
		}
		void JDrawHelper::SettingDrawShadowMap(const JUserPtr<JLight>& lit)noexcept
		{
			JDrawHelper::lit = lit;
			cullUser = lit->CullingUserInterface();
		}
		void JDrawHelper::SettingDrawScene(const JUserPtr<JCamera>& cam)noexcept
		{
			JDrawHelper::cam = cam;
			cullUser = cam->CullingUserInterface();

			allowDrawDepthMap = cam->AllowDisplayDepthMap();
			allowDrawDebug = cam->AllowDisplayDebug();
			allowFrustumCulling = cam->AllowFrustumCulling();
			allowOcclusionCulling = cam->AllowHzbOcclusionCulling();
			allowDrawOccMipMap = cam->AllowDisplayOccCullingDepthMap();
		}

		JDrawCondition::JDrawCondition(const JGraphicOption& option,
			const JDrawHelper& helper,
			const bool newAllowAnimation,
			const bool newAllowCulling,
			const bool newAllowDebugOutline)
			:allowAnimation(allowAnimation), allowAllCullingResult(CamEditorSettingInterface::AllowAllCullingResult(helper.cam))
		{
			allowAnimation = newAllowAnimation; 
			allowCulling = newAllowCulling;
			allowHzbOcclusionCulling = newAllowCulling &&
				helper.allowOcclusionCulling &&
				option.isOcclusionQueryActivated && option.isHZBOcclusionActivated;
			allowHDOcclusionCulling = newAllowCulling &&
				helper.allowOcclusionCulling &&
				option.isOcclusionQueryActivated && option.isHDOcclusionAcitvated;
			allowDebugOutline = newAllowDebugOutline && helper.allowDrawDebug && option.allowDebugOutline;
		}
	}
}