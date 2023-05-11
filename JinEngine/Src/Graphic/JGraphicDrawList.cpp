#include"JGraphicDrawList.h"
#include"GraphicResource/JGraphicResourceInfo.h"
#include"../Object/Resource/Scene/JScene.h" 
#include"../Object/Resource/Scene/JScenePrivate.h" 
#include"../Object/Component/JComponent.h" 
#include"../Object/GameObject/JGameObject.h"
#include"../Object/JFrameUpdate.h" 
#include"../Object/JFrameUpdateUserAccess.h"

namespace JinEngine
{
	namespace Graphic
	{
		JShadowMapDrawRequestor::JShadowMapDrawRequestor(const JUserPtr<JComponent>& jLight, JGraphicResourceInfo* handle)
			:jLight(jLight), handle(handle)
		{}
		JShadowMapDrawRequestor::~JShadowMapDrawRequestor()
		{
			jLight = nullptr;
			handle = nullptr;
		}

		JSceneDrawRequestor::JSceneDrawRequestor(const JUserPtr<JComponent>& jCamera, JGraphicResourceInfo* handle)
			:jCamera(jCamera), handle(handle)
		{}
		JSceneDrawRequestor::~JSceneDrawRequestor()
		{
			jCamera = nullptr;
			handle = nullptr;
		}

		namespace
		{
			static std::vector<std::unique_ptr<JGraphicDrawTarget>> drawList;

			int GetIndex(const JUserPtr<JScene>& scene)noexcept
			{
				const size_t sceneGuid = scene->GetGuid();
				const int drawListCount = (int)drawList.size();
				for (int i = 0; i < drawListCount; ++i)
				{
					if (drawList[i]->scene->GetGuid() == sceneGuid)
						return i;
				}
				return -1;
			}
		}

		JGraphicDrawTarget::UpdateInfo::UpdateInfo(const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling)
			:updateFrequency(updateFrequency), isAllowOcclusionCulling(isAllowOcclusionCulling)
		{}
		JGraphicDrawTarget::UpdateInfo::~UpdateInfo()
		{ 
		}

		void JGraphicDrawTarget::UpdateInfo::UpdateStart()
		{
			objUpdateCount = aniUpdateCount = camUpdateCount = lightUpdateCount =
				hotObjUpdateCount = hotAniUpdateCount = hotCamUpdateCount = hotLitghtUpdateCount = 0;
			hasShadowUpdate = false;
			hasSceneUpdate = false;
			hasOcclusionUpdate = false;

			if (observationFrame != nullptr && observationFrame->IsFrameDirted())
				hasSceneUpdate = true;
		}
		void JGraphicDrawTarget::UpdateInfo::UpdateEnd()
		{
			const uint sceneUpdateCount = objUpdateCount + aniUpdateCount + lightUpdateCount + camUpdateCount;
			const uint hotUpdateCount = hotObjUpdateCount + hotAniUpdateCount + hotLitghtUpdateCount + hotCamUpdateCount;

			if (updateFrequency == J_GRAPHIC_DRAW_FREQUENCY::ALWAYS)
				hasSceneUpdate = true;
			else if (updateFrequency == J_GRAPHIC_DRAW_FREQUENCY::UPDATED && hotUpdateCount > 0)
				hasSceneUpdate = true;

			const uint shadowUpdateCount = hotObjUpdateCount + hotAniUpdateCount + hotLitghtUpdateCount;
			if (shadowUpdateCount > 0)
				hasShadowUpdate = true;

			if (isAllowOcclusionCulling)
			{
				const uint occlusionUpdateCount = hotObjUpdateCount + hotAniUpdateCount + hotCamUpdateCount;
				if (occlusionUpdateCount > 0)
					hasOcclusionUpdate = true;
			}
		}

		JGraphicDrawTarget::JGraphicDrawTarget(const JUserPtr<JScene>& scene, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling)
			: scene(scene)
		{
			updateInfo = std::make_unique<UpdateInfo>(updateFrequency, isAllowOcclusionCulling);
		}
		JGraphicDrawTarget::~JGraphicDrawTarget() {}
		bool JGraphicDrawList::AddDrawList(const JUserPtr<JScene>& scene, const J_GRAPHIC_DRAW_FREQUENCY updateFrequency, const bool isAllowOcclusionCulling)noexcept
		{
			if (scene == nullptr)
				return false;

			if (HasDrawList(scene))
				return false;

			std::unique_ptr<JGraphicDrawTarget> newTarget = std::make_unique<JGraphicDrawTarget>(scene, updateFrequency, isAllowOcclusionCulling);
			drawList.push_back(std::move(newTarget));
			return true;
		}
		bool JGraphicDrawList::PopDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			if (scene == nullptr)
				return false;

			int index = GetIndex(scene);
			if (index == -1)
				return false;

			drawList.erase(drawList.begin() + index);
			const uint drawListCount = (uint)drawList.size();
			for (uint i = index; i < drawListCount; ++i)
				JScenePrivate::CompFrameInterface::SetAllComponentFrameDirty(drawList[i]->scene);

			return true;
		}
		bool JGraphicDrawList::AddObservationFrame(const JUserPtr<JScene>& scene, const JFrameUpdateUserAccess& observationFrame)noexcept
		{
			int index = GetIndex(scene);
			if (index == -1)
				return false;
			drawList[index]->updateInfo->observationFrame = std::make_unique<JFrameUpdateUserAccess>(observationFrame);
			return true;
		}
		bool JGraphicDrawList::HasDrawList(const JUserPtr<JScene>& scene)noexcept
		{
			const size_t sceneGuid = scene->GetGuid();
			const uint drawListCount = (uint)drawList.size();
			for (uint i = 0; i < drawListCount; ++i)
			{
				if (drawList[i]->scene->GetGuid() == sceneGuid)
					return true;
			}
			return false;
		}
		void JGraphicDrawList::UpdateScene(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept
		{
			if (scene == nullptr)
				return;

			int index = GetIndex(scene);

			if (index == -1)
				return;

			const uint drawListCount = (uint)drawList.size();
			for (uint i = index + 1; i < drawListCount; ++i)
				JScenePrivate::CompFrameInterface::SetComponentFrameDirty(drawList[i]->scene, cType);
		}
		void JGraphicDrawList::AddDrawShadowRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jLight, JGraphicResourceInfo* handle)noexcept
		{
			if (scene == nullptr || jLight == nullptr)
				return;

			int index = GetIndex(scene);

			if (index == -1)
				return;

			drawList[index]->shadowRequestor.emplace_back(std::make_unique<JShadowMapDrawRequestor>(jLight, handle));
			drawList[index]->updateInfo->hasShadowUpdate = true;
		}
		void JGraphicDrawList::AddDrawSceneRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jCamera, JGraphicResourceInfo* handle)noexcept
		{
			if (scene == nullptr || jCamera == nullptr)
				return;

			int index = GetIndex(scene);

			if (index == -1)
				return;

			drawList[index]->sceneRequestor.emplace_back(std::make_unique<JSceneDrawRequestor>(jCamera, handle));
			drawList[index]->updateInfo->hasSceneUpdate = true;
		}
		void JGraphicDrawList::PopDrawRequest(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& jComp)noexcept
		{
			if (scene == nullptr || jComp == nullptr)
				return;

			int index = GetIndex(scene);

			if (index == -1)
				return;

			if (jComp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
			{
				const uint reqCount = (uint)drawList[index]->sceneRequestor.size();
				for (uint i = 0; i < reqCount; ++i)
				{
					if (drawList[index]->sceneRequestor[i]->jCamera->GetGuid() == jComp->GetGuid())
					{
						drawList[index]->sceneRequestor.erase(drawList[index]->sceneRequestor.begin() + i);
						drawList[index]->updateInfo->hasSceneUpdate = true;
						break;
					}
				}
			}
			else if (jComp->GetComponentType() == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
			{
				const uint reqCount = (uint)drawList[index]->shadowRequestor.size();
				for (uint i = 0; i < reqCount; ++i)
				{
					if (drawList[index]->shadowRequestor[i]->jLight->GetGuid() == jComp->GetGuid())
					{
						drawList[index]->shadowRequestor.erase(drawList[index]->shadowRequestor.begin() + i);
						drawList[index]->updateInfo->hasSceneUpdate = true;
						drawList[index]->updateInfo->hasShadowUpdate = true;
						break;
					}
				}
			}

			if (drawList[index]->sceneRequestor.size() == 0)
			{
				//const JUserPtr<JScene>& scene = drawList[index]->scene;
				//scene->DeActivate();
			}
		}
		bool JGraphicDrawList::HasRequestor(const JUserPtr<JScene>& scene)noexcept
		{
			if (scene == nullptr)
				return false;

			int index = GetIndex(scene);

			if (index == -1)
				return false;

			return drawList[index]->sceneRequestor.size() + drawList[index]->shadowRequestor.size() > 0;
		}
		uint JGraphicDrawList::GetListCount()noexcept
		{
			return (uint)drawList.size();
		}
		JGraphicDrawTarget* JGraphicDrawList::GetDrawScene(const uint index)noexcept
		{
			return drawList[index].get();
		}
		/*
			std::vector<JGraphicDrawTarget*> JGraphicDrawList::GetAllDrawTarget()noexcept
		{

		}
		std::vector<JGraphicDrawTarget*> JGraphicDrawList::GetDrawableTarget()noexcept
		{

		}
		*/
	}
}