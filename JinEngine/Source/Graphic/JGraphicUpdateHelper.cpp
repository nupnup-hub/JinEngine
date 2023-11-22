#include"JGraphicUpdateHelper.h"
#include"JGraphicInfo.h"
#include"JGraphicOption.h"
#include"JGraphicDrawList.h"
#include"Culling/JCullingInterface.h" 
#include"../Object/Component/JComponent.h"
#include"../Object/Component/Camera/JCamera.h"
#include"../Object/Component/Camera/JCameraPrivate.h"
#include"../Object/Component/Light/JLight.h"  
#include"../Object/Component/Light/JLightPrivate.h"  
#include"../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../Object/Component/Animator/JAnimatorPrivate.h"
#include"../Object/Resource/Scene/JScenePrivate.h"


namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
			using SceneFrameIndexInterface = JScenePrivate::FrameIndexInterface;
			using SceneCashInterface = JScenePrivate::CashInterface;
			using CamEditorSettingInterface = JCameraPrivate::EditorSettingInterface; 
			using CamFrameIndexInterface = JCameraPrivate::FrameIndexInterface;
			using RItemFrameIndexInterface = JRenderItemPrivate::FrameIndexInterface;
			using AniFrameIndexInterface = JAnimatorPrivate::FrameIndexInterface;
			using LitFrameIndexInterface = JLightPrivate::FrameIndexInterface;
		}
		namespace
		{
			static uint UDataCount()noexcept
			{
				return (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT;
			}
			static uint BDataCount()noexcept
			{
				return (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT;
			}
		}

		bool JUpdateHelper::BindingTextureData::HasCallable()const noexcept
		{
			return hasCallable;
		}

		void JUpdateHelper::BeginUpdatingDrawTarget()
		{
			const uint uCount = UDataCount();
			for (uint i = 0; i < uCount; ++i)
				uData[i].uploadCountPerTarget = 0;
		}
		void JUpdateHelper::EndUpdatingDrawTarget()
		{
			const uint uCount = UDataCount();
			for (uint i = 0; i < uCount; ++i)
				uData[i].uploadOffset += uData[i].uploadCountPerTarget;
		}
		void JUpdateHelper::Clear()
		{
			const uint uCount = UDataCount();
			for (uint i = 0; i < uCount; ++i)
			{
				uData[i].count = 0;
				uData[i].capacity = 0; 
				uData[i].uploadCountPerTarget = 0;
				uData[i].uploadOffset = 0;
				//uData[i].setDirty = false;
				if (uData[i].setDirty > 0)
					--uData[i].setDirty;
				uData[i].reAllocCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
			}

			const uint bCount = BDataCount();
			for (uint i = 0; i < bCount; ++i)
			{
				bData[i].count = 0;
				bData[i].capacity = 0;
				bData[i].reAllocCondition = J_UPLOAD_CAPACITY_CONDITION::KEEP;
			}
			hasRebuildCondition = false;
			hasRecompileShader = false;
		}
		void JUpdateHelper::RegisterCallable(J_UPLOAD_FRAME_RESOURCE_TYPE type, GetElementCountT::Ptr getCountPtr)
		{
			if (getCountPtr == nullptr)
				return;

			uData[(int)type].getElement = std::make_unique<GetElementCountT::Callable>(getCountPtr);
			uData[(int)type].useGetMultiCount = false;
			//uData[(int)type].getElementCapacityCallable = std::make_unique<GetElementCapacityT::Callable>(*getCapaPtr);
			//uData[(int)type].rebuildCallable = std::make_unique<RebuildT::Callable>(*rPtr);
		}
		void JUpdateHelper::RegisterCallable(J_GRAPHIC_RESOURCE_TYPE type, GetElementCountT::Ptr* getCountPtr, GetElementCapacityT::Ptr* getCapaPtr)
		{
			if (getCountPtr == nullptr || getCapaPtr == nullptr)
				return;

			bData[(int)type].getTextureCount = std::make_unique<GetElementCountT::Callable>(*getCountPtr); 
			bData[(int)type].getTextureCapacity = std::make_unique<GetElementCapacityT::Callable>(*getCapaPtr);
			bData[(int)type].hasCallable = true;
		}
		void JUpdateHelper::RegisterCallable(GetElementMultiCountT::Ptr getMultiCountPtr)
		{
			getElementMultiCount.push_back(std::make_unique<GetElementMultiCountT::Callable>(getMultiCountPtr));
		}
		void JUpdateHelper::RegisterListener(J_UPLOAD_FRAME_RESOURCE_TYPE type, std::unique_ptr<NotifyUpdateCapacityT::Callable>&& listner)
		{
			uData[(int)type].notifyUpdateCapacity.push_back(std::move(listner));
		}
		void JUpdateHelper::WriteGraphicInfo(JGraphicInfo& info)const noexcept
		{
			info.upObjCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].count;
			info.upEnginePassCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS].count;
			info.upScenePassCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS].count;
			info.upAniCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].count;
			info.upCameraCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].count;
			info.updLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].count;
			info.uppLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].count;
			info.upsLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].count;
			info.upCsmCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO].count;
			info.upCubeShadowMapCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW].count;
			info.upNormalShadowMapCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].count;
			info.upMaterialCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].count;
			info.upObjCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].capacity;
			info.upEnginePassCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ENGINE_PASS].capacity;
			info.upScenePassCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS].capacity;
			info.upAniCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].capacity;
			info.upCameraCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].capacity; 
			info.upMaterialCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].capacity;

			info.binding2DTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].count;
			info.bindingCubeMapCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].count;
			info.bindingShadowTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].count;
			info.bindingShadowTextureArrayCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY].count;
			info.bindingShadowTextureCubeCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE].count;

			info.binding2DTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].capacity;
			info.bindingCubeMapCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].capacity;
			info.bindingShadowTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].capacity;
			info.bindingShadowTextureArrayCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY].capacity;
			info.bindingShadowTextureCubeCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE].capacity;
		}
		void JUpdateHelper::NotifyUpdateFrameCapacity(JGraphic& grpahic)
		{
			for (uint i = 0; i < (uint)J_UPLOAD_FRAME_RESOURCE_TYPE::COUNT; ++i)
			{
				if (uData[i].reAllocCondition != J_UPLOAD_CAPACITY_CONDITION::KEEP)
				{
					const uint listenerCount = (uint)uData[i].notifyUpdateCapacity.size();
					for (uint j = 0; j < listenerCount; ++j)
						(*uData[i].notifyUpdateCapacity[j])(nullptr);
				}
			}
		}
		
		JDrawHelper::JDrawHelper(const JGraphicInfo& info, const JGraphicOption& option, const JAlignedObject& alignedObj)
			:info(info), option(option), alignedObj(alignedObj)
		{}
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
				if (cullingInterface.IsCulled(rItemIndex))
					return true;
			}
			return false;
		} 
		JGraphicResourceUserInterface JDrawHelper::GetOccGResourceInterface()const noexcept
		{
			if (drawType == DRAW_TYPE::OCC)
			{
				if (occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam->GraphicResourceUserInterface();
				if (occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					return lit->GraphicResourceUserInterface();
			}
			return JGraphicResourceUserInterface();
		}
		JCullingUserInterface JDrawHelper::GetCullInterface()const noexcept
		{
			if (drawType == DRAW_TYPE::SCENE)
				return cam->CullingUserInterface();
			else if (drawType == DRAW_TYPE::OCC)
			{
				if (occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam->CullingUserInterface();
				else if (occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					return lit->CullingUserInterface();
				else
					return nullptr;
			}
			else
				return lit->CullingUserInterface();
		}
		JCullingUserAccess* JDrawHelper::GetCullingUserAccess()const noexcept
		{
			if (drawType == DRAW_TYPE::SCENE)
				return cam.Get();
			else if (drawType == DRAW_TYPE::OCC)
			{
				if (occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam.Get();
				else if (occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					return lit.Get();
			}
			return nullptr;
		}
		JDrawHelper::DRAW_TYPE JDrawHelper::GetDrawType()const noexcept
		{
			return drawType;
		}
		int JDrawHelper::GetPassFrameIndex()const noexcept
		{
			return SceneFrameIndexInterface::GetPassFrameIndex(scene.Get());
		}
		int JDrawHelper::GetCamFrameIndex()const noexcept
		{
			return CamFrameIndexInterface::GetCamFrameIndex(cam.Get());
		}
		int JDrawHelper::GetCamDepthTestPassFrameIndex()const noexcept
		{
			return CamFrameIndexInterface::GetDepthTestPassFrameIndex(cam.Get());
		}
		int JDrawHelper::GetCamHzbOccComputeFrameIndex()const noexcept
		{
			return CamFrameIndexInterface::GetHzbOccComputeFrameIndex(cam.Get());
		} 
		int JDrawHelper::GetShadowMapDrawFrameIndex()const noexcept
		{ 
			JLightPrivate* lp = static_cast<JLightPrivate*>(&lit->PrivateInterface());
			return lp->GetFrameIndexInterface().GetShadowMapFrameIndex(lit.Get());
		}
		int JDrawHelper::GetLitDepthTestPassFrameIndex()const noexcept
		{
			JLightPrivate* lp = static_cast<JLightPrivate*>(&lit->PrivateInterface());
			return lp->GetFrameIndexInterface().GetDepthTestPassFrameIndex(lit.Get());
		}
		int JDrawHelper::GetLitHzbOccComputeFrameIndex()const noexcept
		{
			JLightPrivate* lp = static_cast<JLightPrivate*>(&lit->PrivateInterface());
			return lp->GetFrameIndexInterface().GetHzbOccComputeFrameIndex(lit.Get()); 
		}
		const std::vector<JUserPtr<JGameObject>>& JDrawHelper::GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)const noexcept
		{
			return SceneCashInterface::GetGameObjectCashVec(scene, rLayer, meshType);
		} 
		int JDrawHelper::GetObjectFrameIndex(JRenderItem* rItem)noexcept
		{
			return RItemFrameIndexInterface::GetObjectFrameIndex(rItem);
		}
		int JDrawHelper::GetBoundingFrameIndex(JRenderItem* rItem)noexcept
		{
			return RItemFrameIndexInterface::GetBoundingFrameIndex(rItem);
		}
		int JDrawHelper::GetAnimationFrameIndex(JAnimator* ani)noexcept
		{
			return AniFrameIndexInterface::GetFrameIndex(ani);
		}
		void JDrawHelper::SetDrawTarget(JGraphicDrawTarget* drawTarget)noexcept
		{
			JDrawHelper::drawTarget = drawTarget;
			JDrawHelper::scene = drawTarget->scene;
		}
		void JDrawHelper::SetTheadInfo(const uint threadCount, const uint threadIndex)noexcept
		{
			JDrawHelper::threadCount = threadCount;
			JDrawHelper::threadIndex = threadIndex;
		}
		void JDrawHelper::SetAllowMultithreadDraw(const bool value)noexcept
		{
			allowMutilthreadDraw = option.allowMultiThread && value;
		}
		void JDrawHelper::SettingOccCulling(const JWeakPtr<JComponent>& comp)noexcept
		{
			Graphic::JCullingUserAccess* userAccess = nullptr;
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				cam = Core::ConnectChildUserPtr<JCamera>(comp); 
				userAccess = cam.Get();
				//draw depth map + mipmap 
				occCompType = J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
			}
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			{
				lit = Core::ConnectChildUserPtr<JLight>(comp); 
				userAccess = lit.Get();
				//draw depth map  
				occCompType = J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
			}
			drawType = DRAW_TYPE::OCC;		
			 
			if (userAccess != nullptr)
			{
				allowFrustumCulling = userAccess->AllowFrustumCulling();
				allowOcclusionCulling = userAccess->AllowHzbOcclusionCulling() || userAccess->AllowHdOcclusionCulling();
				allowDrawOccDepthMap = userAccess->AllowDisplayOccCullingDepthMap();
			}
		}
		void JDrawHelper::SettingDrawShadowMap(const JWeakPtr<JLight>& lit)noexcept
		{
			JDrawHelper::lit = lit;
			drawType = DRAW_TYPE::SHADOW_MAP; 

			allowDrawDepthMap = lit->AllowDisplayShadowMap();
		}
		void JDrawHelper::SettingDrawScene(const JWeakPtr<JCamera>& cam)noexcept
		{
			JDrawHelper::cam = cam;
			drawType = DRAW_TYPE::SCENE;

			allowDrawDepthMap = cam->AllowDisplayDepthMap();
			allowDrawDebug = cam->AllowDisplayDebug();
			allowFrustumCulling = cam->AllowFrustumCulling();
			allowOcclusionCulling = (cam->AllowHzbOcclusionCulling() && option.isHZBOcclusionActivated) || 
				(cam->AllowHdOcclusionCulling() && option.isHDOcclusionAcitvated);
			allowDrawOccDepthMap = cam->AllowDisplayOccCullingDepthMap();
		}
		bool JDrawHelper::CanDrawShadowMap()const noexcept
		{
			return lit != nullptr && lit->IsShadowActivated();
		}
		bool JDrawHelper::CanOccCulling()const noexcept
		{
			const bool isOcclusionActivated = option.IsHDOccActivated() || option.IsHZBOccActivated();
			const bool canCullingStart = (cam != nullptr && (cam->AllowHzbOcclusionCulling() || cam->AllowHdOcclusionCulling())) ||
				lit != nullptr && lit->AllowHzbOcclusionCulling();

			return isOcclusionActivated && canCullingStart;
		}
		bool JDrawHelper::CanDispatchWorkIndex()const noexcept
		{
			return allowMutilthreadDraw && threadIndex != -1 && threadCount != -1;
		}
		bool JDrawHelper::UsePerspectiveProjection()const noexcept
		{ 
			bool isPerspective = true;
			if (drawType == DRAW_TYPE::SCENE)
			{
				if (cam != nullptr)
					isPerspective = !cam->IsOrthoCamera();
				else if (lit != nullptr)
					isPerspective = lit->GetLightType() == J_LIGHT_TYPE::POINT || lit->GetLightType() == J_LIGHT_TYPE::SPOT;
			}
			else if (drawType == DRAW_TYPE::SHADOW_MAP)
				isPerspective = lit->GetLightType() == J_LIGHT_TYPE::POINT || lit->GetLightType() == J_LIGHT_TYPE::SPOT;
			else if (drawType == DRAW_TYPE::OCC)
			{
				if (occCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					isPerspective = !cam->IsOrthoCamera();
				else
					isPerspective = lit->GetLightType() == J_LIGHT_TYPE::POINT || lit->GetLightType() == J_LIGHT_TYPE::SPOT;
			}
			return isPerspective;
		}	
		void JDrawHelper::DispatchWorkIndex(const uint count, _Out_ uint& stIndex, _Out_ uint& edIndex)const noexcept
		{
			if (count == 0)
			{
				stIndex = 0;
				edIndex = 0;
				return;
			}
			if (count < threadCount)
			{
				if (threadIndex < count)
				{
					stIndex = threadIndex;
					edIndex = threadIndex + 1;
				}
				else
				{
					stIndex = 0;
					edIndex = 0;
				}
			}
			else
			{
				const uint threadPer = count / threadCount;
				stIndex = threadPer * threadIndex;

				if (threadIndex == threadCount - 1)
					edIndex = count;
				else
					edIndex = threadPer * (threadIndex + 1);
			}
		}
		JDrawHelper JDrawHelper::CreateDrawSceneHelper(const JDrawHelper& ori, const JWeakPtr<JCamera>& cam)noexcept
		{ 
			JDrawHelper newHelper = ori;
			newHelper.SettingDrawScene(cam);
			return newHelper;
		}
		JDrawHelper JDrawHelper::CreateDrawShadowMapHelper(const JDrawHelper& ori, const JWeakPtr<JLight>& lit)noexcept
		{
			JDrawHelper newHelper = ori;
			newHelper.SettingDrawShadowMap(lit);
			return newHelper;
		}
		JDrawHelper JDrawHelper::CreateOccCullingHelper(const JDrawHelper& ori, const JWeakPtr<JComponent>& comp)noexcept
		{
			JDrawHelper newHelper = ori;
			newHelper.SettingOccCulling(comp);
			return newHelper;
		}

		JDrawCondition::JDrawCondition(const JDrawHelper& helper,
			const bool newAllowAnimation,
			const bool newAllowCulling,
			const bool newAllowDebugOutline)
			:allowAnimation(allowAnimation)
		{
			allowAnimation = newAllowAnimation; 
			allowCulling = newAllowCulling;
			allowHzbOcclusionCulling = newAllowCulling &&
				helper.allowOcclusionCulling &&
				helper.option.isOcclusionQueryActivated && helper.option.isHZBOcclusionActivated;
			allowHDOcclusionCulling = newAllowCulling &&
				helper.allowOcclusionCulling &&
				helper.option.isOcclusionQueryActivated && helper.option.isHDOcclusionAcitvated;
			allowDebugOutline = newAllowDebugOutline && helper.allowDrawDebug && helper.option.allowDebugOutline;
			allowAllCullingResult = helper.cam != nullptr && CamEditorSettingInterface::AllowAllCullingResult(helper.cam);
			if (allowAllCullingResult)
			{
				allowHzbOcclusionCulling = newAllowCulling;
				allowHDOcclusionCulling = newAllowCulling;
			}
		}
	}
}