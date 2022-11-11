#pragma once
#include<vector> 
#include"../JResourceUserInterface.h"
#include"../../../Core/Singleton/JSingletonHolder.h"
#include"../../../Core/JDataType.h"
#include"../../Component/JComponentType.h"

namespace JinEngine
{
	class JResourceObject;
	class JScene; 
	class IFrameDirty;
	namespace Core
	{
		template <class T> class JCreateUsingNew;
	}

	class JSceneManagerImpl : public JResourceUserInterface
	{
	private:
		template <class T> friend class Core::JCreateUsingNew;
	private:
		//opend Scene  
		//scene vector[0] is main scene  
		std::vector<JScene*> opendScene; 
		JScene* mainScene = nullptr;
	public:
		bool TryOpenScene(JScene* scene, bool isPreviewScene, IFrameDirty* observation = nullptr)noexcept;
		bool TryCloseScene(JScene* scene)noexcept;
		bool IsOpen(JScene* scene)noexcept;
		bool IsMainScene(const JScene* scene)const noexcept;
	public:
		void UpdateScene(JScene* scene, const J_COMPONENT_TYPE compType);
	public:
		uint GetOpendSceneCount()const noexcept; 
		JScene* GetMainScene()noexcept;
	public:
		//Set main scene 
		//if scene isn't opend return fail
		void SetMainScene(JScene* scene)noexcept;
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		JSceneManagerImpl();
		~JSceneManagerImpl();
	};

	using JSceneManager = Core::JSingletonHolder<JSceneManagerImpl>;
}