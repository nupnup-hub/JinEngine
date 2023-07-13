#pragma once 
#include "../Pointer/JOwnerPtr.h"
#include"../Module/JModuleDesc.h"
#include<string>
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		class JModule;
		class JPluginInterface;
		class JPluginManagerInterface
		{
		public:
			virtual JUserPtr<JPluginInterface> GetPlugIn(const std::wstring& name)const = 0; 
			virtual std::vector<JUserPtr<JPluginInterface>> GetAllPlugin()const = 0;
		public:
			/**
			* @brief Load plugin using meta files while search sub direcory
			* @return loaded plugin count
			*/
			virtual uint LoadPlugin(const std::wstring& folderPath) = 0;
			virtual bool StorePlugin(JPluginInterface* p) = 0;
		public:
			JModule* TryAddModule(const std::wstring& path,
				const bool keppLoadState,
				JModuleDesc desc = JModuleDesc());
		};
	}
}