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


#include"JSceneManager.h"
#include"JSceneManagerPrivate.h"
#include"JScene.h" 
#include"../JResourceObjectUserInterface.h"
#include"../../Component/JComponentType.h"
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphicDrawListInterface.h"  
#include"../../../Core/Guid/JGuidCreator.h" 

namespace JinEngine
{
	namespace
	{
		using SearchEqualScenePtr = bool(*)(const JUserPtr< JScene>&, const size_t);
	}

	class JSceneManager::JSceneManagerImpl : public JResourceObjectUserInterface, public Graphic::JGraphicDrawListSceneInterface
	{
	public:
		size_t implGuid = Core::MakeGuid();
	public:
		//opend Scene  
		//scene vector[0] is main scene  
		std::vector<JUserPtr<JScene>> activatedScene;
	public:
		JSceneManagerImpl() 
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
		JUserPtr<JScene> GetFirstScene() noexcept
		{
			return activatedScene.size() > 0 ? activatedScene[0] : nullptr;
		}
	public:
		bool IsRegistered(const size_t guid) noexcept
		{
			SearchEqualScenePtr equalScenePtr = [](const JUserPtr<JScene>& user, const size_t guid) { return user->GetGuid() == guid;};
			return JCUtil::GetIndex(activatedScene, equalScenePtr, guid) != -1;
		}
		bool IsFirstScene(const size_t guid)const noexcept
		{
			return activatedScene.size() > 0 ? guid == activatedScene[0]->GetGuid() : false;
		} 
	public:
		bool RegisterScene(const JUserPtr<JScene>& scene)noexcept
		{
			if (scene == nullptr)
				return false;

			if (!IsRegistered(scene->GetGuid()))
			{
				//has dependency
				//order 1. AddDrawList, 2. activate
				AddDrawList(scene);
				activatedScene.push_back(scene);
				return true;
			}
			else
				return false;
		}
		bool DeRegisterScene(const JUserPtr<JScene>& scene)noexcept
		{
			if (scene == nullptr)
				return false;

			SearchEqualScenePtr equalScenePtr = [](const JUserPtr<JScene>& user, const size_t guid) {return user->GetGuid() == guid; };
			int index = JCUtil::GetIndex(activatedScene, equalScenePtr, scene->GetGuid());
			if (index == -1)
				return false;

			activatedScene.erase(activatedScene.begin() + index);
			PopDrawList(scene);
			return true;
		}
		bool RegisterObservationFrame(const JUserPtr<JScene>& scene, const JUserPtr<Graphic::JFrameUpdateUserAccess>& observationFrame)
		{
			return AddObservationFrame(scene, observationFrame);
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == implGuid)
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE && jRobj->GetResourceType() == J_RESOURCE_TYPE::SCENE)
				DeRegisterScene(Core::GetUserPtr<JScene>(jRobj));
		}
	};
 
	uint JSceneManager::GetActivatedSceneCount()const noexcept
	{
		return impl->GetActivatedSceneCount();
	}
	JUserPtr<JScene> JSceneManager::GetFirstScene() noexcept
	{
		return impl->GetFirstScene();
	}
	bool JSceneManager::IsRegistered(JScene* scene) noexcept
	{
		return impl->IsRegistered(scene->GetGuid());
	}
	bool JSceneManager::IsFirstScene(JScene* scene)const noexcept
	{
		return impl->IsFirstScene(scene->GetGuid());
	}
	bool JSceneManager::RegisterObservationFrame(const JUserPtr<JScene>& scene, const JUserPtr<Graphic::JFrameUpdateUserAccess>& observationFrame)
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

	bool SceneAccess::RegisterScene(const JUserPtr<JScene>& scene)noexcept
	{
		return _JSceneManager::Instance().impl->RegisterScene(scene);
	}
	bool SceneAccess::DeRegisterScene(const JUserPtr<JScene>& scene)noexcept
	{
		return _JSceneManager::Instance().impl->DeRegisterScene(scene);
	} 
}