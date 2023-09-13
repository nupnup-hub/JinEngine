#include"JPlugin.h" 
#include"JPluginPrivate.h"
#include"../File/JJSon.h"
#include"../Module/JModule.h" 
#include"../Module/JModuleManager.h" 
#include"../Utility/JCommonUtility.h" 
#include<iostream>
#include <psapi.h>
 
namespace JinEngine::Core
{
	JPlugin::JPlugin(const std::wstring& name, const std::wstring& path, const JPluginDesc& desc)
		:name(name), path(path), desc(desc)
	{}
	JPlugin::~JPlugin()
	{}

	const std::wstring& JPlugin::GetName()const noexcept
	{
		return name;
	}
	const std::wstring& JPlugin::GetMetaFilePath()const noexcept
	{
		return path;
	}
	JPluginDesc JPlugin::GetPluginDesc()const noexcept
	{
		return desc;
	} 
	uint JPlugin::GetModuleCount()const noexcept
	{
		return (uint)moduleVec.size();
	}
	JUserPtr<JModule> JPlugin::GetModule(const uint index)const noexcept
	{
		return moduleVec[index];
	}
	JUserPtr<JModule> JPlugin::GetModule(const std::wstring& name)const noexcept
	{
		for (const auto& data : moduleVec)
		{
			if (data->GetName() == name)
				return data;
		}
		return nullptr;
	}
	std::vector<JUserPtr<JModule>> JPlugin::GetAllModule()const noexcept
	{
		return moduleVec;
	}

	using IOInterface = JPluginPrivate::IOInterface;
 
	JOwnerPtr<JPluginInterface> IOInterface::LoadPlugin(const std::wstring& metafilePath)
	{
		JJSon json(metafilePath);
		if (!json.Load())
			return nullptr;

		std::wstring name = JCUtil::U8StrToWstr(json.value["Name"].asString());

		JPluginDesc desc;
		desc.version = json.value["Version"].asString();
		desc.minEngineVersion = json.value["MinEngineVersion"].asString();

		JOwnerPtr<JPlugin> newPlugin = JPtrUtil::MakeOwnerPtr<JPlugin>(name, metafilePath, desc);

		auto& modManager = _JModuleManager::Instance();
		const uint moduleCount = (uint)json.value["ModuleInfo"].size();
		for (uint i = 0; i < moduleCount; ++i)
		{
			const std::wstring path = JCUtil::U8StrToWstr(json.value["ModuleInfo"][i]["Path"].asString());
			auto mod = modManager.GetModule(path);
			if (mod != nullptr)
				newPlugin->moduleVec.push_back(mod);
		}
		newPlugin->moduleVec.shrink_to_fit();
		newPlugin->desc.isLoaded = true;
		return newPlugin;
	}
	bool IOInterface::StorePlugin(JPluginInterface* p)
	{
		if (p == nullptr)
			return false;

		auto desc = p->GetPluginDesc();
		if (!desc.isLoaded)
			return false;

		JJSon json(p->GetMetaFilePath());
		json.value["Name"] = JCUtil::WstrToU8Str(p->GetName());
		json.value["Version"] = desc.version;
		json.value["MinEngineVersion"] = desc.minEngineVersion;

		for (const auto& data : p->GetAllModule())
		{
			const std::string name = JCUtil::WstrToU8Str(data->GetName());
			json.value["ModuleInfo"][name]["Name"] = name;
			json.value["ModuleInfo"][name]["Path"] = JCUtil::WstrToU8Str(data->GetPath());
		}
		return json.Store();
	}
}

