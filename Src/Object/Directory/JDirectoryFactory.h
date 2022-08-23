#pragma once  
#include"../JObjectFlag.h"
#include"../../Core/Func/Callable/JCallable.h"
#include<string>

namespace JinEngine
{
	class JResourceManagerImpl;
	class JResourceIO;
	class JDirectory;

	namespace Editor
	{
		class JWindowDirectory;
	};

	class JDirectoryFactoryImpl 
	{
	private:
		friend class JDirectory;
		friend class JResourceManagerImpl;
		friend class JResourceIO;
		friend class Editor::JWindowDirectory;
	private:
		using DefaultPtr = JDirectory * (*)(JDirectory*);
		using InitPtr = JDirectory * (*)(const std::string&, const size_t, const J_OBJECT_FLAG, JDirectory*);
		//using CopytPtr = JDirectory * (*)(JDirectory*);
		using AddStoragePtr = JDirectory * (JResourceManagerImpl::*)(JDirectory&);

		using DefaultCallable = Core::JStaticCallable<JDirectory*, JDirectory*>;
		using InitCallable = Core::JStaticCallable<JDirectory*, const std::string&, const size_t, const J_OBJECT_FLAG, JDirectory*>;
	//	using CopyCallable = Core::JStaticCallable<JDirectory*, JDirectory*>;
		using AddStorageCallable = Core::JMemeberCallable<JResourceManagerImpl, JDirectory*, JDirectory&>;
	private:
		static AddStorageCallable* addStorage;
	private:
		static JDirectory* Create(JDirectory& parent);
		static JDirectory* CreateRoot(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag);
		static JDirectory* Create(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory& parent);
		static void Register(DefaultPtr defaultPtr, InitPtr initPtr);
		static void RegisterAddStroage(AddStoragePtr addPtr);
	}; 
	using JDFI = JDirectoryFactoryImpl;
}