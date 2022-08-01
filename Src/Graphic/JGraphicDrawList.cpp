#include"JGraphicDrawList.h"
#include"../Object/Resource/Scene/JScene.h" 

namespace JinEngine
{
	namespace Graphic
	{
		std::vector<std::unique_ptr<JGraphicDrawTarget>> JGraphicDrawList::drawList;

		JGraphicDrawTargetListener::JGraphicDrawTargetListener(const J_GRAPHIC_DRAW_FREQUENCY drawFrequency)
			:drawFrequency(drawFrequency)
		{}
		JGraphicDrawTargetListener::~JGraphicDrawTargetListener() {}

		JGraphicDrawTarget::JGraphicDrawTarget(JScene* scene, const J_GRAPHIC_DRAW_FREQUENCY drawFrequency, const bool isMainScene)
			: scene(scene),
			sceneFrameDirty(scene),
			transformCount(scene->GetGameObjectCount()),
			renderItemCount(scene->GetReferenceCount()),
			animatorCount(scene->GetAnimatorCount()),
			cameraCount(scene->GetCameraCount()),
			lightCount(scene->GetLightCount()),
			shadowCount(scene->GetShadowCount()),
			isMainScene(isMainScene)
		{
			listenerInfo.push_back(std::make_unique<JGraphicDrawTargetListener>(drawFrequency));
		}
		JGraphicDrawTarget::~JGraphicDrawTarget() {}

		bool JGraphicDrawList::AddDrawList(JScene* scene, const J_GRAPHIC_DRAW_FREQUENCY drawFrequency, const bool isMainScene)noexcept
		{
			if (scene == nullptr)
				return false;

			if (HasDrawList(scene))
				return AddDrawListListener(scene, drawFrequency);

			std::unique_ptr<JGraphicDrawTarget> newTarget = std::make_unique<JGraphicDrawTarget>(scene, drawFrequency, isMainScene);
			if (isMainScene)
			{
				newTarget->sceneFrameDirty->SetAllComponentDirty();
				drawList.emplace_back(std::move(newTarget));
			}
			else
			{
				const uint drawListCount = (uint)drawList.size();
				for (uint i = 0; i < drawListCount; ++i)
				{
					if (drawList[i]->isMainScene)
					{
						newTarget->sceneFrameDirty->SetAllComponentDirty();
						drawList.insert(drawList.begin() + i, std::move(newTarget));
						UpdateDrawList(i);
						break;
					}
				}
			}

			return true;
		}
		bool JGraphicDrawList::PopDrawList(JScene* scene)noexcept
		{
			if (scene == nullptr)
				return false;

			int index = GetIndex(scene);

			if (index == -1)
				return false;

			drawList[index]->listenerInfo.pop_back();
			if (drawList[index]->listenerInfo.size() == 0)
			{
				drawList.erase(drawList.begin() + index);
				UpdateDrawList(index);
			}
			return true;
		}
		bool JGraphicDrawList::HasDrawList(JScene* scene)noexcept
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
		void JGraphicDrawList::UpdateScene(JScene* scene)noexcept
		{
			if (scene == nullptr)
				return;

			int index = GetIndex(scene);

			if (index == -1)
				return;

			const uint drawListCount = (uint)drawList.size();
			if (scene->GetRenderItemCount() != drawList[index]->renderItemCount)
			{
				for (uint i = index + 1; i < drawListCount; ++i)
				{
					drawList[i]->sceneFrameDirty->SetAllTransformDirty();
					drawList[i]->sceneFrameDirty->SetAllRenderItemDirty();
				}
				drawList[index]->transformCount = scene->GetRenderItemCount();
				drawList[index]->renderItemCount = scene->GetRenderItemCount();
			}
			if (scene->GetAnimatorCount() != drawList[index]->animatorCount)
			{
				for (uint i = index + 1; i < drawListCount; ++i)
					drawList[i]->sceneFrameDirty->SetAllAnimatorDirty();
				drawList[index]->animatorCount = scene->GetAnimatorCount();
			}
			if (scene->GetCameraCount() != drawList[index]->cameraCount)
			{
				for (uint i = index + 1; i < drawListCount; ++i)
					drawList[i]->sceneFrameDirty->SetAllCameraDirty();
				drawList[index]->cameraCount = scene->GetCameraCount();
			}
			if (scene->GetLightCount() != drawList[index]->lightCount)
			{
				for (uint i = index + 1; i < drawListCount; ++i)
					drawList[i]->sceneFrameDirty->SetAllLightDirty();
				drawList[index]->lightCount = scene->GetLightCount();
			}
			if (scene->GetShadowCount() != drawList[index]->shadowCount)
			{
				for (uint i = index + 1; i < drawListCount; ++i)
					drawList[i]->sceneFrameDirty->SetAllLightDirty();
				drawList[index]->shadowCount = scene->GetShadowCount();
			}
		}
		bool JGraphicDrawList::AddDrawListListener(JScene* scene, const J_GRAPHIC_DRAW_FREQUENCY drawFrequency)noexcept
		{
			int index = GetIndex(scene);

			if (index == -1)
				return false;

			drawList[index]->listenerInfo.push_back(std::make_unique<JGraphicDrawTargetListener>(drawFrequency));
			return true;
		}
		int JGraphicDrawList::GetIndex(JScene* scene)noexcept
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
		void JGraphicDrawList::UpdateDrawList(int index)noexcept
		{
			const uint drawListCount = (uint)drawList.size();
			for (uint i = index; i < drawListCount; ++i)
				drawList[i]->sceneFrameDirty->SetAllComponentDirty();
		}
	}
}