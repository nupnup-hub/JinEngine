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
#include"JPluginManagerInterface.h"  
#include"JPluginDesc.h"
#include"../Singleton/JSingletonHolder.h" 

namespace JinEngine
{
	namespace Core
	{ 
		class JModule;
		template<typename T>class JCreateUsingNew;
		class JPluginManager final : public JPluginManagerInterface
		{
		private:
			template<typename T> friend class JCreateUsingNew;
			friend class JPluginManagerPrivate;
			class JPluginManagerImpl;
		private:
			std::unique_ptr<JPluginManagerImpl> impl; 
		public:
			JUserPtr<JPluginInterface> GetPlugIn(const std::wstring& path)const final; 
			std::vector<JUserPtr<JPluginInterface>> GetAllPlugin()const final;
		public: 
			/**
			* @brief Load plugin using meta files while search sub direcory
			* @return loaded plugin count
			*/
			uint LoadPlugin(const std::wstring& folderPath) final;
			bool StorePlugin(JPluginInterface* p)final;
		public: 
			/**
			* @brief If meta file not existing, try to create meta file and plugin
			* @return return valid pointer if add success or exist plugin
			*/
			JPluginInterface* TryAddPlugin(const std::wstring& name,
				const std::wstring& folderPath,
				const std::vector<JUserPtr<JModule>>& modVec,
				JPluginDesc desc = JPluginDesc());
		private:
			JPluginManager();
			~JPluginManager();
		};
	}
	using _JPluginManager = Core::JSingletonHolder<Core::JPluginManager>;
}