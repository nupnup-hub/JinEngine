#pragma once 
#include"../../../Core/Singleton/JSingletonHolder.h"
#include"../../../Core/JDataType.h" 
#include"../../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	class JResourceObject;
	class JScene;
	class JFrameUpdateUserAccess;
	namespace Core
	{
		template <class T> class JCreateUsingNew;
	}

	class JSceneManagerPrivate;
	class JSceneManager
	{
	private:
		template <class T> friend class Core::JCreateUsingNew;
	private:
		friend class JSceneManagerPrivate;
		class JSceneManagerImpl;
	private:
		std::unique_ptr<JSceneManagerImpl> impl;
	public:
		uint GetActivatedSceneCount()const noexcept;
		JScene* GetFirstScene()noexcept;
	public:
		bool IsRegistered(JScene* scene)noexcept;
		bool IsFirstScene(const JScene* scene)const noexcept;
	public:
		bool RegisterObservationFrame(JScene* scene, const JFrameUpdateUserAccess& observationFrame);
	private:
		JSceneManager();
		~JSceneManager();
	};

	using _JSceneManager = Core::JSingletonHolder<JSceneManager>;
}