#pragma once
#include"JPluginManagerInterface.h"  
#include"JPluginDesc.h"
#include"../Singleton/JSingletonHolder.h"
#include<memory> 

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