#pragma once
#include<vector>
#include"../JResourceUserInterface.h"
#include"../../../Core/Singleton/JSingletonHolder.h"
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	class JResourceObject;
	class JScene;
	class JGameObject;
	namespace Core
	{
		template <class T> class JCreateUsingNew;
	}

	class JSceneManagerImpl : public JResourceUserInterface
	{
	private:
		template <class T> friend class Core::JCreateUsingNew;
	private:
		std::vector<JScene*> opendScene; 
	public:
		bool TryOpenScene(JScene* scene)noexcept;
		bool TryCloseScene(JScene* scene)noexcept;
		bool IsOpen(JScene* scene)noexcept;
	public:
		uint GetOpendSceneCount()const noexcept; 
	private:
		int GetIndex(const size_t guid);
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		JSceneManagerImpl();
		~JSceneManagerImpl();
	};

	using JSceneManager = Core::JSingletonHolder<JSceneManagerImpl>;
}