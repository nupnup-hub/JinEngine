#pragma once 
#include"JModuleDesc.h"
#include"../Platform/JPlatformInfo.h"
#include"../../Utility/JMacroUtility.h" 
#include<string>

#ifdef OS_WINDOW
#define MODULE_HANDLE HMODULE
#include<windows.h>
#else
#define MODULE_HANDLE void*
#endif

namespace JinEngine
{
	namespace Core
	{
		class JModulePrivate;
		class JModuleManager;
		class JModule
		{ 
		private:
			friend class JModulePrivate;
			friend class JModuleManager;
		private:
			/** @brief It is same as meta file and library file */
			const std::wstring name;
			/** @brief Library path */
			const std::wstring path;  
		private:
			JModuleDesc desc;
		private:
			/** @brief It has  different value by OS*/
			MODULE_HANDLE moduleHandle;
		public:
			JModule(const std::wstring& name, const std::wstring& path, const JModuleDesc& desc);
		public:
			J_SIMPLE_GET(std::wstring, name, Name)
			J_SIMPLE_GET(std::wstring, path, Path) 
			J_SIMPLE_GET(JModuleDesc, desc, Desc)
		public:
			static std::wstring MetafileFormat()noexcept;
			std::wstring GetMetafilePath()const noexcept;
			void* GetFuncPtr(const std::string& name)const noexcept;
		public:
			/** @brief static library is always return true */
			bool IsLoad()const noexcept;
			/**
			* @brief load library 
			* @return return True if load success, false if not load or already loaded
			*/
			bool Load();
			/**
			* @brief load library
			* @return return True if unload success, false if not unload or didn't loaded
			*/
			bool UnLoad();   
		public:
			static MODULE_HANDLE TryLoadModule(const std::wstring& path);
		};
	}
}