#include"JModule.h"
#include"JModulePrivate.h"
#include"JModuleMacro.h"
#include"JModuleFuncDefenitions.h"   
#include"../File/JFileIOHelper.h"
#include"../Utility/JCommonUtility.h"
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
	bool JModule::IsValidFormat(const std::wstring& format)noexcept
	{ 
		return format == L".dll" || format == L".DLL" || format == L".Dll";
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
		JFileIOTool tool;
		if (!tool.Begin(metafilePath, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return nullptr; 

		std::wstring name;
		std::wstring path;

		JModuleDesc desc; 
		bool isSuccess = true;
		isSuccess &= JFileIOHelper::LoadJString(tool, name, "Name") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::LoadJString(tool, path, "Path") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::LoadJString(tool, desc.version, "Version") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::LoadEnumData(tool, desc.moduleType, "ModuleType") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::LoadEnumData(tool, desc.linkType, "LinkType") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::LoadEnumData(tool, desc.loadPhase, "LoadPhase") == Core::J_FILE_IO_RESULT::SUCCESS;
		tool.Close();

		if (!isSuccess)
			return nullptr;
		return JModule::IsValidFormat(JCUtil::GetFileFormat(path)) ? JPtrUtil::MakeOwnerPtr<JModule>(name, path, desc) : nullptr;
	}
	bool IOInterface::StoreModule(const JUserPtr<JModule>& m)
	{
		JFileIOTool tool;
		if (m == nullptr || !tool.Begin(m->GetMetafilePath(), JFileIOTool::TYPE::JSON))
			return false;

		bool isSuccess = true;
		isSuccess &= JFileIOHelper::StoreJString(tool, JCUtil::WstrToU8Str(m->name), "Name") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::StoreJString(tool, JCUtil::WstrToU8Str(m->path), "Path") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::StoreJString(tool, m->desc.version, "Version") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::StoreEnumData(tool, m->desc.moduleType, "ModuleType") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::StoreEnumData(tool, m->desc.linkType, "LinkType") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess &= JFileIOHelper::StoreEnumData(tool, m->desc.loadPhase, "LoadPhase") == Core::J_FILE_IO_RESULT::SUCCESS;
		isSuccess ? tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA) : tool.Close();
		return isSuccess;
	}
}