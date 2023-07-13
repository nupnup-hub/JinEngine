#pragma once
#include"../Singleton/JSingletonHolder.h"
#include"../Pointer/JOwnerPtr.h"
#include"../JDataType.h"
#include"JModuleMacro.h"
#include"JModuleDesc.h"
#include<string>
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		class JModule;
		class JModuleIntializer;
		template<typename T>class JCreateUsingNew;
		class CORE_API JModuleManager
		{ 
		private:
			template<typename T> friend class JCreateUsingNew;
			friend class JModuleManagerPrivate;
			class JModuleManagerImpl;
		private:
			std::unique_ptr<JModuleManagerImpl> impl;
		public:
			JUserPtr<JModule> GetModule(const std::wstring& path)const noexcept;
		public:
			bool HasModule(const std::wstring& path)const noexcept;
		public: 
			/**
			* @brief Load modules using meta files while search sub direcory  
			* @return loaded module count
			*/
			uint LoadModule(const std::wstring& folder);
			bool StoreModule(const JUserPtr<JModule>& m);
		public:
			/**
			* @brief If meta file not existing, try to create meta file and module
			* @return return valid pointer if add success or exist module
			*/
			JUserPtr<JModule> TryAddModule(const std::wstring& filePath, const bool keppLoadState);
		public:
			bool RegisterModule(const JModuleIntializer& init);
		private:
			JModuleManager();
			~JModuleManager();
		};
	}
	using _JModuleManager = Core::JSingletonHolder<Core::JModuleManager>;
}
 