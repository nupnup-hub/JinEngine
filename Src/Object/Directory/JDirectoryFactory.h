#pragma once  
#include"../JObjectFlag.h"
#include"../../Core/Func/Callable/JCallable.h"
#include<string>

namespace JinEngine
{
	class JResourceManager;
	class JResourceIO;
	class JDirectory;

	class JDirectoryFactoryImpl 
	{
	private:
		friend class JDirectory;
		friend class JResourceManager;
		friend class JResourceIO;
	private:
		using DefaultPtr = JDirectory * (*)(JDirectory*);
		using InitPtr = JDirectory * (*)(const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*);
		//using CopytPtr = JDirectory * (*)(JDirectory*);
		using AddStoragePtr = JDirectory * (JResourceManager::*)(JDirectory&);

		using DefaultCallable = Core::JStaticCallable<JDirectory*, JDirectory*>;
		using InitCallable = Core::JStaticCallable<JDirectory*, const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*>;
	//	using CopyCallable = Core::JStaticCallable<JDirectory*, JDirectory*>;
		using AddStorageCallable = Core::JMemeberCallable<JResourceManager, JDirectory*, JDirectory&>;
	private:
		static AddStorageCallable* addStorage;
	private:
		static JDirectory* Create(JDirectory& parent);
		static JDirectory* CreateRoot(const std::string& name, const size_t guid, const JOBJECT_FLAG flag);
		static JDirectory* Create(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory& parent);
		static void Regist(DefaultPtr defaultPtr, InitPtr initPtr);
		static void RegistAddStroage(AddStoragePtr addPtr);
	}; 
	using JDFI = JDirectoryFactoryImpl;
}