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
#include"../Singleton/JSingletonHolder.h"
#include"../Pointer/JOwnerPtr.h"
#include"../JCoreEssential.h"
#include"JModuleMacro.h"
#include"JModuleDesc.h" 

namespace JinEngine
{
	namespace Core
	{
		class JModule;
		class JModuleIntializer;
		template<typename T>class JCreateUsingNew;
		class JModuleManager
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
			JOwnerPtr<JModule> CreateModule(const JModuleIntializer& init);
		private:
			JModuleManager();
			~JModuleManager();
		};
	}
	using _JModuleManager = Core::JSingletonHolder<Core::JModuleManager>;
}
 