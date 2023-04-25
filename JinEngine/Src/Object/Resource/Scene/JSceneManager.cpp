#include"JSceneManager.h"
#include"JSceneManagerPrivate.h"
#include"JScene.h" 
#include"../JResourceObjectUserInterface.h"
#include"../../Component/JComponentType.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphicDrawList.h"  
#include"../../../Core/Guid/GuidCreator.h" 

namespace JinEngine
{
	class JSceneManager::JSceneManagerImpl : public JResourceObjectUserInterface
	{
	public:
		size_t implGuid = Core::MakeGuid();
	public:
		//opend Scene  
		//scene vector[0] is main scene  
		std::vector<JScene*> activatedScene;
	public:
		JSceneManagerImpl()
			:JResourceObjectUserInterface(implGuid)
		{
			AddEventListener(*JResourceObject::EvInterface(), implGuid, J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		~JSceneManagerImpl()
		{
			RemoveListener(*JResourceObject::EvInterface(), implGuid);
		}
	public:
		uint GetActivatedSceneCount()const noexcept
		{
			return (uint)activatedScene.size();
		}
		JScene* GetFirstScene() noexcept
		{
			return activatedScene.size() > 0 ? activatedScene[0] : nullptr;
		}
	public:
		bool IsRegistered(JScene* scene) noexcept
		{
			return JCUtil::GetJIdenIndex(activatedScene, scene->GetGuid()) != -1;
		}
		bool IsFirstScene(const JScene* scene)const noexcept
		{
			return activatedScene.size() > 0 ? scene->GetGuid() == activatedScene[0]->GetGuid() : false;
		}
	public:
		void UpdateScene(JScene* scene, const J_COMPONENT_TYPE compType)
		{
			Graphic::JGraphicDrawList::UpdateScene(scene, compType);
		}
	public:
		bool RegisterScene(JScene* scene, bool isPreviewScene)noexcept
		{
			if (scene == nullptr)
				return false;

			if (!IsRegistered(scene))
			{
				//has dependency
				//order 1. AddDrawList, 2. activate
				if (isPreviewScene)
					Graphic::JGraphicDrawList::AddDrawList(scene, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED, false);
				else
					Graphic::JGraphicDrawList::AddDrawList(scene, Graphic::J_GRAPHIC_DRAW_FREQUENCY::ALWAYS, true);
				activatedScene.push_back(scene);
				return true;
			}
			else
				return false;
		}
		bool DeRegisterScene(JScene* scene)noexcept
		{
			if (scene == nullptr)
				return false;

			int index = JCUtil::GetJIdenIndex(activatedScene, scene->GetGuid());
			if (index == -1)
				return false;

			activatedScene.erase(activatedScene.begin() + index);
			Graphic::JGraphicDrawList::PopDrawList(scene);
			return true;
		}
		bool RegisterObservationFrame(JScene* scene, const JFrameUpdateUserAccess& observationFrame)
		{
			return Graphic::JGraphicDrawList::AddObservationFrame(scene, observationFrame);
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == implGuid)
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE && jRobj->GetResourceType() == J_RESOURCE_TYPE::SCENE)
				DeRegisterScene(static_cast<JScene*>(jRobj));
		}
	};
 
	uint JSceneManager::GetActivatedSceneCount()const noexcept
	{
		return impl->GetActivatedSceneCount();
	}
	JScene* JSceneManager::GetFirstScene() noexcept
	{
		return impl->GetFirstScene();
	}
	bool JSceneManager::IsRegistered(JScene* scene) noexcept
	{
		return impl->IsRegistered(scene);
	}
	bool JSceneManager::IsFirstScene(const JScene* scene)const noexcept
	{
		return impl->IsFirstScene(scene);
	}
	bool JSceneManager::RegisterObservationFrame(JScene* scene, const JFrameUpdateUserAccess& observationFrame)
	{
		return impl->RegisterObservationFrame(scene, observationFrame);
	}

	JSceneManager::JSceneManager()
		:impl(std::make_unique<JSceneManagerImpl>())
	{}
	JSceneManager::~JSceneManager()
	{
		impl.reset();
	}

	using SceneAccess = JSceneManagerPrivate::SceneAccess;

	bool SceneAccess::RegisterScene(JScene* scene, bool isPreviewScene)noexcept
	{
		return _JSceneManager::Instance().impl->RegisterScene(scene, isPreviewScene);
	}
	bool SceneAccess::DeRegisterScene(JScene* scene)noexcept
	{
		return _JSceneManager::Instance().impl->DeRegisterScene(scene);
	}
	void SceneAccess::UpdateScene(JScene* scene, const J_COMPONENT_TYPE compType)
	{
		_JSceneManager::Instance().impl->UpdateScene(scene, compType);
	}
}