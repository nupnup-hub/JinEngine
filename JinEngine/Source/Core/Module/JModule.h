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


#pragma once 
#include"JModuleDesc.h"
#include"../Platform/JPlatformInfo.h"
#include"../Utility/JMacroUtility.h"  

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
			static bool IsValidFormat(const std::wstring& format)noexcept;
		public:
			static MODULE_HANDLE TryLoadModule(const std::wstring& path);
		};
	}
}