#include"JModule.h"
#include"JModulePrivate.h"
#include"JModuleMacro.h"
#include"JModuleFuncDefenitions.h"  
#include"../File/JJSon.h"
#include"../../Utility/JCommonUtility.h"
#include <psapi.h>
#include<fstream>

namespace JinEngine::Core
{
	namespace
	{
		static std::vector<std::string> GetJSonLabelVec()noexcept
		{
			return std::vector<std::string>
			{
				"Name","Path","Version","ModuleType","LinkType","LoadPhase"
			};
		}
	}
	JModule::JModule(const std::wstring& name, const std::wstring& path, const JModuleDesc& desc)
		:name(name), path(path), desc(desc)
	{}
	std::wstring JModule::MetafileFormat()noexcept
	{
		return L".module";
	}
	std::wstring JModule::GetMetafilePath()const noexcept
	{
		return JCUtil::ChangeFileFormat(path, MetafileFormat());
	}
	void* JModule::GetFuncPtr(const std::string& name)const noexcept
	{
#ifdef OS_WINDOW
		return desc.isLoaded ? GetProcAddress(moduleHandle, name.c_str()) : nullptr;
#else
		return nullptr;
#endif
	}
	bool JModule::IsLoad()const noexcept
	{ 
		return desc.isLoaded;
	}
	bool JModule::Load()
	{
		if (desc.isLoaded)
			return false;

		std::wstring name;
		std::wstring folderPath;
		std::wstring format;
		std::wstring dllPath;

		JCUtil::DecomposeFilePath(path, folderPath, name, format);
		JCUtil::FindFirstFilePathByName(folderPath, name + L".dll", dllPath);
#ifdef OS_WINDOW
		moduleHandle = ::LoadLibraryW(dllPath.c_str());
#endif
		if (moduleHandle == NULL)
			return false;

		desc.isLoaded = true;
		auto func = (JInitModuleFunc)::GetProcAddress(moduleHandle, INIT_MODULE);
		func();
		return true;
	}
	bool JModule::UnLoad()
	{
		if (!desc.isLoaded || desc.linkType == J_MODULE_LINK_TYPE::STATIC)
			return false;

#ifdef OS_WINDOW
		auto func = (JFreeModuleFunc)::GetProcAddress(moduleHandle, FREE_MODULE);
#endif
		func();

		desc.isLoaded = false;
#ifdef OS_WINDOW
		const bool res = ::FreeLibrary(moduleHandle);
#endif
		if (res != 0)
			moduleHandle = NULL;
		return res;
	}
	MODULE_HANDLE JModule::TryLoadModule(const std::wstring& path)
	{
#ifdef OS_WINDOW
		return ::LoadLibraryW(path.c_str());
#else
		return nullptr;	//¹Ì±¸Çö
#endif	
	}

	using IOInterface = JModulePrivate::IOInterface;
	JOwnerPtr<JModule> IOInterface::LoadModule(const std::wstring& metafilePath)
	{
		JJSon json(metafilePath);
		if (!json.Load())
			return nullptr;

		std::wstring name;
		std::wstring path;

		const std::vector<std::string> contentsVec = GetJSonLabelVec();
		for(const auto& data: contentsVec)
		{
			if (json.value[data].isNull())	
				return nullptr;
		}
 
		JModuleDesc desc; 
		name = JCUtil::U8StrToWstr(json.value["Name"].asString());
		path = JCUtil::U8StrToWstr(json.value["Path"].asString());
		desc.version = json.value["Version"].asString();
		desc.moduleType = (J_MODULE_TYPE)json.value["ModuleType"].asInt();
		desc.linkType = (J_MODULE_LINK_TYPE)json.value["LinkType"].asInt();
		desc.loadPhase = (J_MODULE_LOAD_PHASE)json.value["LoadPhase"].asInt();

		return JPtrUtil::MakeOwnerPtr<JModule>(name, path, desc);
	}
	bool IOInterface::StoreModule(const JUserPtr<JModule>& m)
	{
		if (m == nullptr)
			return false;

		JJSon json(m->GetMetafilePath());
		json.value["Name"] = JCUtil::WstrToU8Str(m->name);
		json.value["Path"] = JCUtil::WstrToU8Str(m->path);
		json.value["Version"] = m->desc.version;
		json.value["ModuleType"] = (int)m->desc.moduleType;
		json.value["LinkType"] = (int)m->desc.linkType;
		json.value["LoadPhase"] = (int)m->desc.loadPhase;

		return json.Store();
	}
}