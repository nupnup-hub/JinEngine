#include"JApplicationVariable.h"
#include"../Utility/JCommonUtility.h"
#include"../Core/JDataType.h"
#include<Windows.h> 
#include <direct.h>	 
#include<fstream>  
#include<io.h>

namespace JinEngine
{
	namespace Application
	{
		std::wstring JApplicationVariable::engineExePath;
		std::wstring JApplicationVariable::enginePath;
		std::wstring JApplicationVariable::enginePathW;
		std::wstring JApplicationVariable::defaultResourcePath;
		std::wstring JApplicationVariable::shaderPath;
		std::wstring JApplicationVariable::activatedProjectPath;
		std::wstring JApplicationVariable::activatedProjectName;

		std::wstring JApplicationVariable::contentPath;
		std::wstring JApplicationVariable::projectSettingPath;
		std::wstring JApplicationVariable::libraryPath;
		std::wstring JApplicationVariable::shaderMetafilePath;
		std::wstring JApplicationVariable::projectDefaultResourcePath;
		std::wstring JApplicationVariable::projectEditorResoucePath;

		std::wstring JApplicationVariable::sceneFolderPath;
		std::wstring JApplicationVariable::scriptFolderPath;
		std::wstring JApplicationVariable::resourceFolderPath;
		std::vector<std::wstring> JApplicationVariable::folderPath;

		J_APPLICATION_STATE JApplicationVariable::applicationState = J_APPLICATION_STATE::PROJECT_SELECT;
		Core::J_LANGUAGE_TYPE JApplicationVariable::engineLanguage;

		JApplicationVariable::JApplicationVariable()
		{ 
			TCHAR programpath[_MAX_PATH];
			GetModuleFileName(NULL, programpath, _MAX_PATH);
			engineExePath = programpath;
			size_t index = engineExePath.find_last_of(L"\\");
			enginePath = engineExePath.substr(0, index);

			index = enginePath.find_last_of(L"\\");
			enginePath.erase(enginePath.begin() + index, enginePath.end());
			index = enginePath.find_last_of(L"\\");
			enginePath.erase(enginePath.begin() + index, enginePath.end());

			enginePathW = enginePath;
			defaultResourcePath = enginePath + L"\\EngineResource\\" + L"BasicResource";
			shaderPath = enginePath + L"\\EngineResource\\" + L"JShader";

			setlocale(LC_ALL, "");
			std::string localeStr = setlocale(LC_ALL, NULL);
			if (localeStr == "Korean_Korea.949")
				engineLanguage = Core::J_LANGUAGE_TYPE::KOREAN;
			else
				engineLanguage = Core::J_LANGUAGE_TYPE::ENGLISH;
		}
		JApplicationVariable::~JApplicationVariable() {}
		std::wstring JApplicationVariable::GetEngineExePath()noexcept
		{
			return engineExePath;
		}
		std::wstring JApplicationVariable::GetEnginePath()noexcept
		{
			return enginePath;
		}
		std::wstring JApplicationVariable::GetDefaultResourcePath()noexcept
		{
			return defaultResourcePath;
		}
		std::wstring JApplicationVariable::GetShaderPath()noexcept
		{
			return shaderPath;
		}
		std::wstring JApplicationVariable::GetActivatedProjectPath()noexcept
		{
			return activatedProjectPath;
		}
		std::wstring JApplicationVariable::GetActivatedProjectName()noexcept
		{
			return activatedProjectName;
		}
		std::wstring JApplicationVariable::GetProjectPath()noexcept
		{
			return activatedProjectPath;
		}
		std::wstring JApplicationVariable::GetProjectContentPath()noexcept
		{
			return contentPath;
		}
		std::wstring JApplicationVariable::GetProjectSettingPath()noexcept
		{
			return projectSettingPath;
		}
		std::wstring JApplicationVariable::GetProjectLibraryPath()noexcept
		{
			return libraryPath;
		}
		std::wstring JApplicationVariable::GetProjectShaderMetafilePath()noexcept
		{
			return shaderMetafilePath;
		}
		std::wstring JApplicationVariable::GetProjectDefaultResourcePath()noexcept
		{
			return projectDefaultResourcePath;
		}
		std::wstring JApplicationVariable::GetProjectEditorResourcePath()noexcept
		{
			return projectEditorResoucePath;
		}
		std::wstring JApplicationVariable::GetProjectContentScenePath()noexcept
		{
			return sceneFolderPath;
		}
		std::wstring JApplicationVariable::GetProjectContentScriptPath()noexcept
		{
			return scriptFolderPath;
		}
		std::wstring JApplicationVariable::GetProjectContentResourcePath()noexcept
		{
			return resourceFolderPath;
		}
		J_APPLICATION_STATE JApplicationVariable::GetApplicationState()noexcept
		{
			return applicationState;
		}
		Core::J_LANGUAGE_TYPE JApplicationVariable::GetEngineLanguageType()noexcept
		{
			return engineLanguage;
		}
		bool JApplicationVariable::IsDefaultFolder(const std::wstring& path)noexcept
		{
			uint folderCount = (uint)folderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (path == folderPath[i])
					return true;
			}
			return false;
		}
		void JApplicationVariable::MakeFolderPath(const std::wstring& projectName, const std::wstring& projectPath)
		{
			activatedProjectName = projectName;
			activatedProjectPath = projectPath + L"\\" + activatedProjectName;

			contentPath = activatedProjectPath + L"\\" + L"Content";
			projectSettingPath = activatedProjectPath + L"\\" + L"ProjectSetting";
			libraryPath = activatedProjectPath + L"\\" + L"Library";
			shaderMetafilePath = libraryPath + L"\\" + L"ShaderMetafile";
			projectDefaultResourcePath = projectSettingPath + L"\\" + L"DefaultResource";
			projectEditorResoucePath = projectSettingPath + L"\\" + L"Editor";

			sceneFolderPath = contentPath + L"\\" + L"JScene";
			scriptFolderPath = contentPath + L"\\" + L"Script";
			resourceFolderPath = contentPath + L"\\" + L"Resource";
		}
		bool JApplicationVariable::AccessProjectFolder()
		{
			uint folderCount = (uint)folderPath.size();
			if (folderCount == 0)
			{
				folderPath =
				{
					activatedProjectPath,
					contentPath,
					projectSettingPath,
					libraryPath,
					shaderMetafilePath,
					projectEditorResoucePath,
					sceneFolderPath,
					scriptFolderPath,
					resourceFolderPath,
				};
				folderCount = (uint)folderPath.size();
			}
			for (uint i = 0; i < folderCount; ++i)
			{
				if (_access(folderPath[i].c_str(), 00) == -1)
				{
					if (_mkdir(folderPath[i].c_str()) == -1)
					{
						MessageBox(0, L"폴더생성실패", JCommonUtility::U8StrToWstr(folderPath[i]).c_str(), 0);
						return false;
					}
				}
			}
			return true;
		}
		void JApplicationVariable::StartNewProject()
		{
			applicationState = J_APPLICATION_STATE::EDIT_GAME;
		}
	}
}
