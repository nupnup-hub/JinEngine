/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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
			hasUploadDataDirty = hasBindingDataDirty = false; 
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
		/*
		void JUpdateHelper::RegisterCallable(GetElementMultiCountT::Ptr getMultiCountPtr)
		{
			getElementMultiCount.push_back(std::make_unique<GetElementMultiCountT::Callable>(getMultiCountPtr));
		}
		*/ 
		void JUpdateHelper::WriteGraphicInfo(JGraphicInfo& info)const noexcept
		{
			info.frame.upObjCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].count;
			info.frame.upBoundingObjCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].count;
			info.frame.upHzbObjCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].count;
			info.frame.upScenePassCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS].count;
			info.frame.upAniCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].count;
			info.frame.upCameraCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].count;
			info.frame.upDLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].count;
			info.frame.upPLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].count;
			info.frame.upSLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].count;
			info.frame.upRLightCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT].count;
			info.frame.upCsmCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO].count;
			info.frame.upCubeShadowMapCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW].count;
			info.frame.upNormalShadowMapCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW].count;
			info.frame.upMaterialCount = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].count;

			info.frame.upObjCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT].capacity;
			info.frame.upBoundingObjCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT].capacity;
			info.frame.upHzbObjCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_OBJECT].capacity;
			info.frame.upScenePassCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SCENE_PASS].capacity;
			info.frame.upAniCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION].capacity;
			info.frame.upCameraCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA].capacity;
			info.frame.upDLightCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT].capacity;
			info.frame.upPLightCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT].capacity;
			info.frame.upSLightCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT].capacity;
			info.frame.upRLightCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT].capacity;
			info.frame.upMaterialCapacity = uData[(int)J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL].capacity;

			info.resource.binding2DTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].count;
			info.resource.bindingCubeMapCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].count;
			info.resource.bindingShadowTextureCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].count;
			info.resource.bindingShadowTextureArrayCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY].count;
			info.resource.bindingShadowTextureCubeCount = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE].count;

			info.resource.binding2DTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D].capacity;
			info.resource.bindingCubeMapCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE].capacity;
			info.resource.bindingShadowTextureCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP].capacity;
			info.resource.bindingShadowTextureArrayCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY].capacity;
			info.resource.bindingShadowTextureCubeCapacity = bData[(int)J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE].capacity;
		}
		
		void JGameObjectBuffer::ClearAlignedVecElement()
		{
			for (auto& data : aligned)
			{
				if(data.size() > 0)
					data.clear();
			}
		}

		JDrawHelper::JDrawHelper(const JGraphicInfo& info, const JGraphicOption& option, JGameObjectBuffer& objVec)
			:info(info), option(option), objVec(objVec)
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
				if (cullingInterface.IsCulled(J_CULLING_TARGET::RENDERITEM, rItemIndex))
					return true;
			}
			return false;
		} 
		JGraphicResourceUserInterface JDrawHelper::GetOccGResourceInterface()const noexcept
		{
			if (drawType == DRAW_TYPE::OCC)
			{
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam->GraphicResourceUserInterface();
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
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
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam->CullingUserInterface();
				else if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					return lit->CullingUserInterface();
				else
					return nullptr;
			}
			else if (drawType == DRAW_TYPE::FRUSTUM_CULLING)
			{
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam->CullingUserInterface();
				else if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
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
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam.Get();
				else if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					return lit.Get();
			}
			else if (drawType == DRAW_TYPE::FRUSTUM_CULLING)
			{
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					return cam.Get();
				else if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
					return lit.Get();
			}
			return nullptr;
		}
		JDrawHelper::DRAW_TYPE JDrawHelper::GetDrawType()const noexcept
		{
			return drawType;
		} 
		int JDrawHelper::GetSceneFrameIndex()const noexcept
		{
			return JFrameIndexAccess::GetSceneFrameIndex(scene.Get());
		}
		int JDrawHelper::GetCamFrameIndex(const uint frameLayerIndex)const noexcept
		{
			return JFrameIndexAccess::GetCamFrameIndex(cam.Get(), frameLayerIndex);
		} 
		int JDrawHelper::GetLitFrameIndex(const uint frameLayerIndex)const noexcept
		{
			return JFrameIndexAccess::GetLitFrameIndex(lit.Get(), frameLayerIndex);
		} 
		int JDrawHelper::GetLitShadowFrameIndex()const noexcept
		{
			return JFrameIndexAccess::GetLitShadowFrameIndex(lit.Get()); ;
		}
		const std::vector<JUserPtr<JGameObject>>& JDrawHelper::GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)const noexcept
		{
			return SceneCashInterface::GetGameObjectCashVec(scene, rLayer, meshType);
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
			allowMutilthreadDraw = option.rendering.allowMultiThread && value;
		}
		void JDrawHelper::SettingDrawShadowMap(const JWeakPtr<JLight>& lit)noexcept
		{
			JDrawHelper::lit = lit;
			drawType = DRAW_TYPE::SHADOW_MAP; 

			allowDrawShadowMap = lit->IsShadowActivated();
			allowDrawDebugMap = lit->AllowDisplayShadowMap();
			allowDrawOccDepthMap = lit->AllowDisplayOccCullingDepthMap();
		}
		void JDrawHelper::SettingDrawScene(const JWeakPtr<JCamera>& cam)noexcept
		{
			JDrawHelper::cam = cam;
			drawType = DRAW_TYPE::SCENE;

			allowDrawDebugMap = cam->AllowDisplayRenderResult();
			allowDrawDebugObject = cam->AllowDisplayDebugObject();
			allowFrustumCulling = cam->AllowFrustumCulling();
			allowHzbOcclusionCulling = cam->AllowHzbOcclusionCulling();
			allowHdOcclusionCulling = cam->AllowHdOcclusionCulling();
			allowDrawOccDepthMap = (allowHzbOcclusionCulling || allowHdOcclusionCulling) && cam->AllowDisplayOccCullingDepthMap();
			allowSsao = cam->AllowSsao() && option.CanUseSSAO();
			allowPostProcess = cam->AllowPostProcess();
			allowRtGi = cam->AllowRaytracingGI();
			allowTemporalProcess = allowRtGi;
		}
		void JDrawHelper::SettingFrustumCulling(const JWeakPtr<JComponent>& comp)noexcept
		{
			drawType = DRAW_TYPE::FRUSTUM_CULLING;
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				cam = Core::ConnectChildUserPtr<JCamera>(comp);
				cullingCompType = J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
			}
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			{
				lit = Core::ConnectChildUserPtr<JLight>(comp);
				cullingCompType = J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
			}
		}
		void JDrawHelper::SettingOccCulling(const JWeakPtr<JComponent>& comp)noexcept
		{
			Graphic::JCullingUserAccess* userAccess = nullptr;
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				cam = Core::ConnectChildUserPtr<JCamera>(comp);
				userAccess = cam.Get();
				//draw depth map + mipmap 
				cullingCompType = J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
			}
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			{
				lit = Core::ConnectChildUserPtr<JLight>(comp);
				userAccess = lit.Get();
				//draw depth map  
				cullingCompType = J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
			}
			drawType = DRAW_TYPE::OCC;

			if (userAccess != nullptr)
			{
				const bool isOcclusionActivated = option.culling.isOcclusionQueryActivated;
				allowFrustumCulling = userAccess->AllowFrustumCulling();
				if (isOcclusionActivated)
				{
					allowHzbOcclusionCulling = userAccess->AllowHzbOcclusionCulling();
					allowHdOcclusionCulling = userAccess->AllowHdOcclusionCulling();
					allowDrawOccDepthMap = (allowHzbOcclusionCulling || allowHdOcclusionCulling) && userAccess->AllowDisplayOccCullingDepthMap();
				}
			}
		}
		void JDrawHelper::SettingLightCulling(const JWeakPtr<JCamera>& cam)
		{
			JDrawHelper::cam = cam;
			drawType = DRAW_TYPE::LIT_CULLING;
			allowLightCulling = cam->AllowLightCulling() && option.culling.isLightCullingActivated;
			allowLightCullingDebug = cam->AllowDisplayLightCullingDebug() && option.debugging.allowDisplayLightCullingResult;
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
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
					isPerspective = !cam->IsOrthoCamera();
				else
					isPerspective = lit->GetLightType() == J_LIGHT_TYPE::POINT || lit->GetLightType() == J_LIGHT_TYPE::SPOT;
			}
			return isPerspective;
		}	
		void JDrawHelper::DispatchWorkIndex(const uint count, _Out_ uint& stIndex, _Out_ uint& edIndex)const noexcept
		{
			if (!CanDispatchWorkIndex())
			{
				stIndex = 0;
				edIndex = count;
				return;
			}

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
		JDrawHelper JDrawHelper::CreateFrustumCullingHelper(const JDrawHelper& ori, const JWeakPtr<JComponent>& comp)noexcept
		{
			JDrawHelper newHelper = ori;
			newHelper.SettingFrustumCulling(comp);
			return newHelper;
		}
		JDrawHelper JDrawHelper::CreateOccCullingHelper(const JDrawHelper& ori, const JWeakPtr<JComponent>& comp)noexcept
		{
			JDrawHelper newHelper = ori;
			newHelper.SettingOccCulling(comp);
			return newHelper;
		}
		JDrawHelper JDrawHelper::CreateLitCullingHelper(const JDrawHelper& ori, const JWeakPtr<JCamera>& cam)noexcept
		{
			JDrawHelper newHelper = ori;
			newHelper.SettingLightCulling(cam);
			return newHelper;
		}

		JDrawCondition::JDrawCondition(const JDrawHelper& helper,
			const bool newAllowAnimation,
			const bool newAllowCulling,
			const bool newAllowDebugOutline,
			const bool onlyDrawOccluder)
			:allowAnimation(allowAnimation),
			onlyDrawOccluder(onlyDrawOccluder)
		{
			allowAnimation = newAllowAnimation; 
			allowCulling = newAllowCulling; 
			allowOutline = newAllowDebugOutline && helper.allowDrawDebugObject && helper.option.debugging.allowOutline;
			allowAllCullingResult = helper.cam != nullptr && CamEditorSettingInterface::AllowAllCullingResult(helper.cam);
			if (allowAllCullingResult)
				allowCulling = newAllowCulling;
		}
		void JDrawCondition::SetRestrictRange(const uint st, const uint count)
		{
			drawSt = st;
			drawEd = st + count;
			restrictRange = true;
		}
		bool JDrawCondition::IsValidDrawingIndex(const uint drawIndex)const noexcept
		{
			return restrictRange ? (drawSt <= drawIndex && drawIndex < drawEd) : true;
		}
	}
}