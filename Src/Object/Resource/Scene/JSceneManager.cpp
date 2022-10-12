#include"JSceneManager.h"
#include"JScene.h"
#include"../JResourceManager.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphicDrawList.h"  

namespace JinEngine
{
	static size_t implGuid = Core::MakeGuid();

	bool JSceneManagerImpl::TryOpenScene(JScene* scene, bool isPreviewScene, IFrameDirty* observation)noexcept
	{
		if (scene == nullptr)
			return false;

		if (!IsOpen(scene) && JCUtil::GetJIdenIndex(opendScene, scene->GetGuid()) == -1)
		{
			//has dependency
			//order 1. AddDrawList, 2. activate
			if (isPreviewScene)
				Graphic::JGraphicDrawList::AddDrawList(scene, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			else
				Graphic::JGraphicDrawList::AddDrawList(scene, Graphic::J_GRAPHIC_DRAW_FREQUENCY::ALWAYS);
			CallOnResourceReference(scene);
			opendScene.push_back(scene);
			return true;
		}
		else
			return false;
	}
	bool JSceneManagerImpl::TryCloseScene(JScene* scene)noexcept
	{
		if (scene == nullptr)
			return false;

		int index = JCUtil::GetJIdenIndex(opendScene, scene->GetGuid());
		if (index == -1)
			return false;

		CallOffResourceReference(scene);
		if (CallGetResourceReferenceCount(*scene) == 0)
		{
			if (IsMainScene(scene))
				SetMainScene(nullptr);
			opendScene.erase(opendScene.begin() + index);
			Graphic::JGraphicDrawList::PopDrawList(scene);
			return true;
		}
		else
			return false;
	}
	bool JSceneManagerImpl::IsOpen(JScene* scene) noexcept
	{
		return CallGetResourceReferenceCount(*scene) != 0;
	}
	bool JSceneManagerImpl::IsMainScene(JScene* scene)const noexcept
	{
		return mainScene != nullptr ? scene->GetGuid() == mainScene->GetGuid() : false;
	}
	void JSceneManagerImpl::UpdateScene(JScene* scene, const J_COMPONENT_TYPE compType)
	{
		Graphic::JGraphicDrawList::UpdateScene(scene, compType);
	}
	uint JSceneManagerImpl::GetOpendSceneCount()const noexcept
	{
		return (uint)opendScene.size();
	}
	JScene* JSceneManagerImpl::GetMainScene() noexcept
	{
		return mainScene;
	}
	void JSceneManagerImpl::SetMainScene(JScene* scene)noexcept
	{
		if (scene == nullptr)
		{
			mainScene = scene;
			return;
		}

		int index = JCUtil::GetJIdenIndex(opendScene, scene->GetGuid());
		if (index != -1)
			mainScene = scene;
	}
	void JSceneManagerImpl::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == implGuid)
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE && jRobj->GetResourceType() == J_RESOURCE_TYPE::SCENE)
			TryCloseScene(static_cast<JScene*>(jRobj));
	}
	JSceneManagerImpl::JSceneManagerImpl()
	{
		AddEventListener(*JResourceManager::Instance().EvInterface(), implGuid, J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
	}
	JSceneManagerImpl::~JSceneManagerImpl()
	{
		RemoveListener(*JResourceManager::Instance().EvInterface(), implGuid);
	}
}