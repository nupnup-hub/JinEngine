#pragma once 
#include"../JObjectFlag.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include"../../Core/Func/Callable/JCallable.h"
#include<fstream>

namespace JinEngine
{
	class JScene;
	class JGameObject;

	class JGameObjectFactoryImpl 
	{
	private:
		friend class JScene;
		friend class JGameObject;
	private:
		using DefaultPtr = JGameObject * (*)(JGameObject*);
		using InitPtr = JGameObject * (*)(const std::wstring&, const size_t, const J_OBJECT_FLAG, JGameObject*, JScene*);
		using LoadPtr = JGameObject * (*)(std::wifstream&, JGameObject*);
		using CopytPtr = JGameObject * (*)(JGameObject*, JGameObject*);
		using AddStoragePtr = JGameObject * (JScene::*)(JGameObject&);

		using DefaultCallable = Core::JStaticCallable<JGameObject*, JGameObject*>;
		using InitCallable = Core::JStaticCallable<JGameObject*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JGameObject*, JScene*>;
		using LoadCallable = Core::JStaticCallable<JGameObject*, std::wifstream&, JGameObject*>;
		using CopyCallable = Core::JStaticCallable<JGameObject*, JGameObject*, JGameObject*>;
		using AddStorageCallable = Core::JMemeberCallable<JScene, JGameObject*, JGameObject&>;
	private:
		static AddStorageCallable* addStorage;
	public:
		static JGameObject* Create(JGameObject& parent);
		static JGameObject* Create(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject& parent);
		static JGameObject* CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JScene& ownerScene);
		static JGameObject* Copy(JGameObject& ori, JGameObject& parent);
	private:
		static JGameObject* Create(std::wifstream& stream, JGameObject* parent);
		static void Regist(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr);
		static void RegisterAddStroage(AddStoragePtr addPtr);
	};

	using JGFI = JGameObjectFactoryImpl;
}