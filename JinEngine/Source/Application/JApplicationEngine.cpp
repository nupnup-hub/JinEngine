#include"JApplicationEngine.h" 
#include"JApplicationEnginePrivate.h" 
#include"JApplication.h"
#include"../Core/JDataType.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Utility/JCommonUtility.h"
#include<Windows.h> 
#include <direct.h>	 
#include<fstream>  
#include<io.h>

namespace JinEngine
{
	namespace Application
	{
		namespace Private
		{
			//engine path
			static std::wstring engineExePath;
			static std::wstring engineProjPath;
			static std::wstring engineSlnPath;

			static J_APPLICATION_STATE applicationState;
			static Core::J_LANGUAGE_TYPE engineLanguage;
 
			static std::vector<std::string> versionList
			{
				"1.00",
			};
		}

		std::wstring JApplicationEngine::ExePath()noexcept
		{
			return Private::engineExePath;
		}
		std::wstring JApplicationEngine::SolutionPath()noexcept
		{
			return Private::engineSlnPath;
		}
		std::wstring JApplicationEngine::ProjectPath()noexcept
		{
			return Private::engineProjPath;
		}
		std::wstring JApplicationEngine::BinaryPath()noexcept
		{
			return Private::engineSlnPath + L"\\Binary";
		}
		std::wstring JApplicationEngine::DotNetBinaryPath()noexcept
		{
			return BinaryPath() + L"\\DotNet";
		} 
		std::wstring JApplicationEngine::ResourcePath()
		{
			return Private::engineProjPath + L"\\EngineResource";
		}
		std::wstring JApplicationEngine::InfoPath()noexcept
		{
			return Private::engineProjPath + L"\\EngineInfo";
		}
		std::wstring JApplicationEngine::DefaultResourcePath()noexcept
		{
			return ResourcePath() + L"\\DefaultResource";
		}
		std::wstring JApplicationEngine::ProjectLastRsPath()noexcept
		{
			return ResourcePath() + L"\\ProjectRenderResult";
		}
		std::wstring JApplicationEngine::ProjectListFilePath()noexcept
		{
			return InfoPath() + L"\\ProejctList.txt";
		}
		std::wstring JApplicationEngine::ShaderPath()noexcept
		{
			return ResourcePath() + L"\\Shader";
		}
		std::wstring JApplicationEngine::SouceCodePath()noexcept
		{
			return Private::engineProjPath + L"\\Source";
		}
		std::wstring JApplicationEngine::ThirdPartyPath()noexcept
		{
			return Private::engineProjPath + L"\\ThirdParty";
		}

		//versionList
		//0 ~ n .. Latest version ~ old version 
		std::string JApplicationEngine::GetLatestVersion()noexcept
		{
			return Private::versionList[0];
		}
		int JApplicationEngine::GetSubverionDigitRange()noexcept
		{
			return 2;
		}
		std::vector<std::string> JApplicationEngine::GetAppVersion()noexcept
		{
			return Private::versionList;
		}
		J_APPLICATION_STATE JApplicationEngine::GetApplicationState()noexcept
		{
			return Private::applicationState;
		}
		Core::J_LANGUAGE_TYPE JApplicationEngine::GetLanguageType()noexcept
		{
			return Private::engineLanguage;
		}
		std::vector<std::wstring> JApplicationEngine::GetDefaultFolderPath()noexcept
		{
			return std::vector<std::wstring>
			{
				Private::engineProjPath,
					ResourcePath(),
					DefaultResourcePath(),
					ProjectLastRsPath(),
					InfoPath(),
					ShaderPath()
			};
		}
		std::string JApplicationEngine::GetDriveAlpha()noexcept
		{ 
			return JCUtil::WstrToU8Str(Private::engineProjPath.substr(0, 1));
		}
		bool JApplicationEngine::IsDefaultFolder(const std::wstring& path)noexcept
		{
			auto defulatEFolderPathVec = GetDefaultFolderPath();
			for (const auto& data : defulatEFolderPathVec)
			{
				if (path == data)
					return true;
			}
			return false;
		}
		using AppAccess = JApplicationEnginePrivate::AppAccess; 
		 	 
		void AppAccess::Initialize()
		{
			TCHAR programpath[_MAX_PATH];
			GetModuleFileName(NULL, programpath, _MAX_PATH);
			Private::engineExePath = programpath;
			size_t index = Private::engineExePath.find_last_of(L"\\");
			Private::engineSlnPath = Private::engineExePath.substr(0, index);

			index = Private::engineSlnPath.find_last_of(L"\\");
			Private::engineSlnPath.erase(Private::engineSlnPath.begin() + index, Private::engineSlnPath.end());
			index = Private::engineSlnPath.find_last_of(L"\\");
			Private::engineSlnPath.erase(Private::engineSlnPath.begin() + index, Private::engineSlnPath.end());
			index = Private::engineSlnPath.find_last_of(L"\\");
			Private::engineSlnPath.erase(Private::engineSlnPath.begin() + index, Private::engineSlnPath.end());

			Private::engineProjPath = Private::engineSlnPath + L"\\JinEngine";
			setlocale(LC_ALL, "");
			std::string localeStr = setlocale(LC_ALL, NULL);
			if (localeStr == "Korean_Korea.949")
				Private::engineLanguage = Core::J_LANGUAGE_TYPE::KOREAN;
			else
				Private::engineLanguage = Core::J_LANGUAGE_TYPE::ENGLISH;

			MakeEngineFolder();
		}
		bool AppAccess::MakeEngineFolder()
		{
			auto defulatFolderPathVec = JApplicationEngine::GetDefaultFolderPath();
			for (const auto& data : defulatFolderPathVec)
			{
				if (_waccess(data.c_str(), 00) == -1)
				{
					if (_wmkdir(data.c_str()) == -1)
						return false;
				}
			}
			return true;
		}
		void AppAccess::SetApplicationState(const J_APPLICATION_STATE newState)noexcept
		{
			Private::applicationState = newState;
		}
	}
}
