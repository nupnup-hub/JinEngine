#include"JPlugin.h" 
#include"JPluginPrivate.h"
#include"../File/JFileIOHelper.h"
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
		JFileIOTool tool;
		if (!tool.Begin(metafilePath, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return nullptr;
		 
		std::wstring name;
		JPluginDesc desc;

		bool isSuccess = true;
		isSuccess &= JFileIOHelper::LoadJString(tool, name, "Name") == Core::J_FILE_IO_RESULT::SUCCESS; 
		isSuccess &= JFileIOHelper::LoadJString(tool, desc.version, "Version") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::LoadJString(tool, desc.minEngineVersion, "MinEngineVersion") == Core::J_FILE_IO_RESULT::SUCCESS;
		JOwnerPtr<JPlugin> newPlugin = JPtrUtil::MakeOwnerPtr<JPlugin>(name, metafilePath, desc);

		auto& modManager = _JModuleManager::Instance(); 
		tool.PushExistStack("ModuleData");

		const uint moduleCount = tool.GetCurrentMemberCount();
		for (uint i = 0; i < moduleCount; ++i)
		{
			std::wstring name;
			std::wstring path;
			tool.PushExistStack();
			isSuccess &= JFileIOHelper::StoreJString(tool, name, "Name") == Core::J_FILE_IO_RESULT::SUCCESS;
			isSuccess &= JFileIOHelper::StoreJString(tool, path, "Path") == Core::J_FILE_IO_RESULT::SUCCESS; 
			tool.PopStack();

			auto mod = modManager.GetModule(path);
			if (mod != nullptr)
				newPlugin->moduleVec.push_back(mod);
		}
		tool.PopStack();
		tool.Close();
		if (!isSuccess)
			return nullptr;

		newPlugin->moduleVec.shrink_to_fit();
		newPlugin->desc.isLoaded = true;
		return newPlugin;
	}
	bool IOInterface::StorePlugin(JPluginInterface* p)
	{
		if (p == nullptr)
			return false;

		JFileIOTool tool;
		auto desc = p->GetPluginDesc();
		if (!desc.isLoaded || !tool.Begin(p->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return false;

		bool isSuccess = true;
		isSuccess &= JFileIOHelper::StoreJString(tool, JCUtil::WstrToU8Str(p->GetName()), "Name") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::StoreJString(tool, desc.version, "Version") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::StoreJString(tool, desc.minEngineVersion, "MinEngineVersion") == Core::J_FILE_IO_RESULT::SUCCESS;
 
		tool.PushArrayOwner("ModuleData");
		for (const auto& data : p->GetAllModule())
		{ 
			tool.PushArrayMember();
			isSuccess &= JFileIOHelper::StoreJString(tool, JCUtil::WstrToU8Str(data->GetName()), "Name") == Core::J_FILE_IO_RESULT::SUCCESS;
			isSuccess &= JFileIOHelper::StoreJString(tool, JCUtil::WstrToU8Str(data->GetPath()), "Path") == Core::J_FILE_IO_RESULT::SUCCESS;
			tool.PopStack();
		}
		tool.PopStack();
		isSuccess ? tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA) : tool.Close();
		return isSuccess;
	}
}

