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
#include"../Object/Resource/Scene/JScene.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{  
			using CamEditorSettingInterface = JCameraPrivate::EditorSettingInterface;  
		}
		namespace
		{
			static uint UDataCount()noexcept
			{
				return (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT;
			}
			static uint BDataCount()noexcept
			{
				return (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT;
			}
		}
		 

		void JUpdateHelper::Begin()
		{ 
		}
		void JUpdateHelper::End()
		{ 
		}
		void JUpdateHelper::Clear()
		{
			const uint uCount = UDataCount();
			for (uint i = 0; i < uCount; ++i)
			{
				uData[i].count = 0;
				uData[i].capacity = 0;    
				uData[i].reAllocCondition = J_GRAPHIC_CAPACITY_CONDITION::KEEP;
			}

			const uint bCount = BDataCount();
			for (uint i = 0; i < bCount; ++i)
			{
				bData[i].count = 0;
				bData[i].capacity = 0;
				bData[i].reAllocCondition = J_GRAPHIC_CAPACITY_CONDITION::KEEP;
			}
			hasUploadDataDirty = hasBindingDataDirty = false; 
		}  
		/*
		void JUpdateHelper::RegisterCallable(GetElementMultiCountT::Ptr getMultiCountPtr)
		{
			getElementMultiCount.push_back(std::make_unique<GetElementMultiCountT::Callable>(getMultiCountPtr));
		}
		*/ 
		void JUpdateHelper::WriteGraphicInfo(JGraphicInfo& info)const noexcept
		{
			for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
			{
				info.frame.count[i] = uData[i].count;
				info.frame.capacity[i] = uData[i].capacity;
			}
			for (uint i = 0; i < (uint)J_RESOURCE_TYPE::COUNT; ++i)
			{
				info.resource.count[i] = bData[i].count;
				info.resource.border[i] = bData[i].capacity;
			}
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

				auto cullingInterface = data->jCamera->ModuleManagedData()->GetCullingUserInterface(); 
				if (cullingInterface->IsCulled(J_CULLING_TARGET::RENDERITEM, rItemIndex))
					return true;
			}
			return false;
		} 
		JGraphicResourceInterface* JDrawHelper::GetResourceInterface()const noexcept
		{
			return static_cast<JGraphicResourceInterface*>(comp->ModuleManagedData()->GetGraphicResourceUserInterface());
		}
		JCullingInterface* JDrawHelper::GetCullInterface()const noexcept
		{
			return static_cast<JCullingInterface*>(comp->ModuleManagedData()->GetCullingUserInterface()); 
		} 
		JFrameUpdateInterface* JDrawHelper::GetFrameInterface()const noexcept
		{
			return static_cast<JFrameUpdateInterface*>(comp->ModuleManagedData()->GetFrameUpdateUserInterface());
		}
		JGpuAcceleratorInterface* JDrawHelper::GetGpuAcceleratorInterface()const noexcept
		{
			return static_cast<JGpuAcceleratorInterface*>(scene->ModuleManagedData()->GetGpuAcceleratorUserInterface());
		}
		JGraphicObjectDataSetBase* JDrawHelper::GetObjectDataSet()const noexcept
		{
			return static_cast<JGraphicObjectDataSetBase*>(comp->ModuleManagedData());
		}
		JDrawHelper::DRAW_TYPE JDrawHelper::GetDrawType()const noexcept
		{
			return drawType;
		} 
		int JDrawHelper::GetSceneFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			return scene->ModuleManagedData()->GetFrameUpdateUserInterface()->GetFrameIndex(type);
		}
		int JDrawHelper::GetCamFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			return cam->ModuleManagedData()->GetFrameUpdateUserInterface()->GetFrameIndex(type); 
		} 
		int JDrawHelper::GetLitFrameIndex(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
		{
			return lit->ModuleManagedData()->GetFrameUpdateUserInterface()->GetFrameIndex(type); 
		} 
		int JDrawHelper::GetLitShadowFrameIndex(const J_SHADOW_MAP_TYPE smType)const noexcept
		{
			return lit->ModuleManagedData()->GetFrameUpdateUserInterface()->GetFrameIndex(JLightType::SmToFrameR(smType));
		}
		const std::vector<JUserPtr<JGameObject>>& JDrawHelper::GetGameObjectCacheVec(const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)const noexcept
		{
			return scene->GetGameObjectCacheVec(rLayer, meshType);
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
			SetLight(lit); 
			drawType = DRAW_TYPE::SHADOW_MAP; 

			allowDrawShadowMap = lit->IsShadowActivated();
			allowDrawDebugMap = lit->AllowDisplayShadowMap();
			allowDrawOccDepthMap = lit->AllowDisplayOccCullingDepthMap();
		}
		void JDrawHelper::SettingDrawScene(const JWeakPtr<JCamera>& cam)noexcept
		{
			SetCamera(cam); 
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
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_CAMERA)
			{
				SetCamera(Core::ConnectChildUserPtr<JCamera>(comp)); 
				cullingCompType = J_COMPONENT_TYPE::ENGINE_CAMERA;
			}
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_LIGHT)
			{
				SetLight(Core::ConnectChildUserPtr<JLight>(comp)); 
				cullingCompType = J_COMPONENT_TYPE::ENGINE_LIGHT;
			}
		}
		void JDrawHelper::SettingOccCulling(const JWeakPtr<JComponent>& comp)noexcept
		{ 
			if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_CAMERA)
			{ 
				SetCamera(Core::ConnectChildUserPtr<JCamera>(comp)); 
				//draw depth map + mipmap 
				cullingCompType = J_COMPONENT_TYPE::ENGINE_CAMERA;
			}
			else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_LIGHT)
			{
				SetLight(Core::ConnectChildUserPtr<JLight>(comp)); 
				//draw depth map  
				cullingCompType = J_COMPONENT_TYPE::ENGINE_LIGHT;
			}
			drawType = DRAW_TYPE::OCC;

			if (comp->ModuleManagedData()->CanAccessCullingResource())
			{
				const bool isOcclusionActivated = option.culling.isOcclusionQueryActivated;
				if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_CAMERA)
				{
					allowFrustumCulling = cam->AllowFrustumCulling();
					allowHzbOcclusionCulling = isOcclusionActivated && cam->AllowHzbOcclusionCulling();
					allowHdOcclusionCulling = isOcclusionActivated && cam->AllowHdOcclusionCulling();
					allowDrawOccDepthMap = (allowHzbOcclusionCulling || allowHdOcclusionCulling) && cam->AllowDisplayOccCullingDepthMap();
				}
				else if (comp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_LIGHT)
				{
					allowFrustumCulling = lit->AllowFrustumCulling();
					allowHzbOcclusionCulling = isOcclusionActivated && lit->AllowHzbOcclusionCulling();
					allowHdOcclusionCulling = isOcclusionActivated && lit->AllowHdOcclusionCulling();
					allowDrawOccDepthMap = (allowHzbOcclusionCulling || allowHdOcclusionCulling) && lit->AllowDisplayOccCullingDepthMap();
				}
			}
		}
		void JDrawHelper::SettingLightCulling(const JWeakPtr<JCamera>& cam)
		{
			SetCamera(cam); 
			drawType = DRAW_TYPE::LIT_CULLING;
			allowLightCulling = cam->AllowLightCulling() && option.culling.isLightCullingActivated;
			allowLightCullingDebug = cam->AllowDisplayLightCullingDebug() && option.debugging.allowDisplayLightCullingResult;
		}			 
		void JDrawHelper::SetCamera(const JWeakPtr<JCamera>& newCam)noexcept
		{
			cam = newCam;
			comp = cam.Get();
		}
		void JDrawHelper::SetLight(const JWeakPtr<JLight>& newLit)noexcept
		{
			lit = newLit;
			comp = lit.Get();
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
				if (cullingCompType == J_COMPONENT_TYPE::ENGINE_CAMERA)
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
			allowAllCullingResult = helper.cam != nullptr && helper.cam->AllowReflectAllCullingResult();
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