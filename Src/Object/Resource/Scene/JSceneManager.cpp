#include"JSceneManager.h"
#include"JScene.h"
#include"../JResourceManager.h"
#include"../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	static size_t implGuid = Core::MakeGuid();

	bool JSceneManagerImpl::TryOpenScene(JScene* scene)noexcept
	{
		if (!IsOpen(scene) && GetIndex(scene->GetGuid()) == -1)
		{
			OnResourceReference(*scene);
			opendScene.push_back(scene);
			return true;
		}
		else
			return false;
	}
	bool JSceneManagerImpl::TryCloseScene(JScene* scene)noexcept
	{
		int index = GetIndex(scene->GetGuid());
		if (index == -1)
			return false;

		OffResourceReference(*scene);
		if (GetResourceReferenceCount(*scene) == 0)
		{
			opendScene.erase(opendScene.begin() + index);
			return true;
		}
		else
			return false;
	}
	bool JSceneManagerImpl::IsOpen(JScene* scene)noexcept
	{
		return GetResourceReferenceCount(*scene) != 0;
	}
	uint JSceneManagerImpl::GetOpendSceneCount()const noexcept
	{
		return (uint)opendScene.size();
	}
	int JSceneManagerImpl::GetIndex(const size_t guid)noexcept
	{
		const uint sceneCount = (uint)opendScene.size();
		for (uint i = 0; i < sceneCount; ++i)
		{
			if (opendScene[i]->GetGuid() == guid)
				return i;
		}
		return -1;
	}
	void JSceneManagerImpl::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == implGuid)
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
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