#pragma once  
#include"../JObjectFlag.h"
#include"../../Core/Func/Callable/JCallable.h" 
#include<string>

namespace JinEngine
{
	class JResourceManagerImpl;
	class JResourceIO;
	class JDirectory;
	 
	namespace Core
	{
		struct JAssetFileLoadPathData;
	}
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
		using InitPtr = JDirectory * (*)(const std::wstring&, const size_t, const J_OBJECT_FLAG, JDirectory*);
		using LoadPtr = JDirectory * (*)(JDirectory*, const Core::JAssetFileLoadPathData&);
		using CopyPtr = JDirectory * (*)(JDirectory*, JDirectory*);
		using AddStoragePtr = JDirectory * (JResourceManagerImpl::*)(JDirectory&);

		using DefaultCallable = Core::JStaticCallable<JDirectory*, JDirectory*>;
		using InitCallable = Core::JStaticCallable<JDirectory*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JDirectory*>;
		using LoadCallable = Core::JStaticCallable<JDirectory*, JDirectory*, const Core::JAssetFileLoadPathData&>;
		using CopyCallable = Core::JStaticCallable<JDirectory*, JDirectory*, JDirectory*>;
	public:
		static JDirectory* Create(JDirectory& parent);
		static JDirectory* Create(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory& parent);
	private:
		static JDirectory* CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag);
		static JDirectory* Load(JDirectory& parent, const Core::JAssetFileLoadPathData& pathData);
		static JDirectory* LoadRoot(const Core::JAssetFileLoadPathData& pathData);
		static JDirectory* Copy(JDirectory& ori, JDirectory& parent);
		static void Register(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopyPtr copyPtr);
	}; 
	using JDFI = JDirectoryFactoryImpl;
}