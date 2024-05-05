#pragma once  
#include"../Module/JModuleMacro.h"
#include"../Pointer/JOwnerPtr.h"
#include"JPluginDesc.h" 

namespace JinEngine
{
	class JObject;
	namespace Core
	{
		class JModule;
		class JPluginInterface
		{
		public:
			virtual ~JPluginInterface() = default;
		public:
			virtual const std::wstring& GetName()const noexcept = 0;
			virtual const std::wstring& GetMetaFilePath()const noexcept = 0; 
			virtual JPluginDesc GetPluginDesc()const noexcept = 0; 
			virtual uint GetModuleCount()const noexcept = 0;
			virtual JUserPtr<JModule> GetModule(const uint index)const noexcept = 0; 
			virtual JUserPtr<JModule> GetModule(const std::wstring& name)const noexcept = 0;
			virtual std::vector<JUserPtr<JModule>> GetAllModule()const noexcept = 0;
		};
	}
}