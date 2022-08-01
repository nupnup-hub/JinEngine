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

		std::string JApplicationVariable::engineExePath;
		std::string JApplicationVariable::enginePath;
		std::wstring JApplicationVariable::enginePathW;
		std::string JApplicationVariable::defaultResourcePath;
		std::string JApplicationVariable::shaderPath;
		std::string JApplicationVariable::activatedProjectPath;
		std::string JApplicationVariable::activatedProjectName;

		std::string JApplicationVariable::contentPath;
		std::string JApplicationVariable::projectSettingPath;
		std::string JApplicationVariable::libraryPath;
		std::string JApplicationVariable::shaderMetafilePath;
		std::string JApplicationVariable::projectDefaultResourcePath;
		std::string JApplicationVariable::projectEditorResoucePath;

		std::string JApplicationVariable::sceneFolderPath;
		std::string JApplicationVariable::scriptFolderPath;
		std::string JApplicationVariable::resourceFolderPath;
		std::vector<std::string> JApplicationVariable::folderPath;

		J_APPLICATION_STATE JApplicationVariable::applicationState = J_APPLICATION_STATE::PROJECT_SELECT;
		Core::J_LANGUAGE_TYPE JApplicationVariable::engineLanguage;

		JApplicationVariable::JApplicationVariable()
		{ 
			TCHAR programpath[_MAX_PATH];
			GetModuleFileName(NULL, programpath, _MAX_PATH);
			engineExePath = JCommonUtility::WstringToU8String(programpath);
			size_t index = engineExePath.find_last_of("\\");
			enginePath = engineExePath.substr(0, index);

			index = enginePath.find_last_of("\\");
			enginePath.erase(enginePath.begin() + index, enginePath.end());
			index = enginePath.find_last_of("\\");
			enginePath.erase(enginePath.begin() + index, enginePath.end());

			enginePathW = JCommonUtility::U8StringToWstring(enginePath);
			defaultResourcePath = enginePath + "\\EngineResource\\" + u8"BasicResource";
			shaderPath = enginePath + "\\EngineResource\\" + u8"JShader";

			setlocale(LC_ALL, "");
			std::string localeStr = setlocale(LC_ALL, NULL);
			if (localeStr == "Korean_Korea.949")
				engineLanguage = Core::J_LANGUAGE_TYPE::KOREAN;
			else
				engineLanguage = Core::J_LANGUAGE_TYPE::ENGLISH;
		}
		JApplicationVariable::~JApplicationVariable() {}
		std::string JApplicationVariable::GetEngineExePath()noexcept
		{
			return engineExePath;
		}
		std::string JApplicationVariable::GetEnginePath()noexcept
		{
			return enginePath;
		}
		std::wstring JApplicationVariable::GetEnginePathW()noexcept
		{
			return enginePathW;
		}
		std::string JApplicationVariable::GetDefaultResourcePath()noexcept
		{
			return defaultResourcePath;
		}
		std::string JApplicationVariable::GetShaderPath()noexcept
		{
			return shaderPath;
		}
		std::string JApplicationVariable::GetActivatedProjectPath()noexcept
		{
			return activatedProjectPath;
		}
		std::string JApplicationVariable::GetActivatedProjectName()noexcept
		{
			return activatedProjectName;
		}
		std::string JApplicationVariable::GetProjectPath()noexcept
		{
			return activatedProjectPath;
		}
		std::string JApplicationVariable::GetProjectContentPath()noexcept
		{
			return contentPath;
		}
		std::string JApplicationVariable::GetProjectSettingPath()noexcept
		{
			return projectSettingPath;
		}
		std::string JApplicationVariable::GetProjectLibraryPath()noexcept
		{
			return libraryPath;
		}
		std::string JApplicationVariable::GetProjectShaderMetafilePath()noexcept
		{
			return shaderMetafilePath;
		}
		std::string JApplicationVariable::GetProjectDefaultResourcePath()noexcept
		{
			return projectDefaultResourcePath;
		}
		std::string JApplicationVariable::GetProjectEditorResourcePath()noexcept
		{
			return projectEditorResoucePath;
		}
		std::string JApplicationVariable::GetProjectContentScenePath()noexcept
		{
			return sceneFolderPath;
		}
		std::string JApplicationVariable::GetProjectContentScriptPath()noexcept
		{
			return scriptFolderPath;
		}
		std::string JApplicationVariable::GetProjectContentResourcePath()noexcept
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
		bool JApplicationVariable::IsDefaultFolder(const std::string& path)noexcept
		{
			uint folderCount = (uint)folderPath.size();
			for (uint i = 0; i < folderCount; ++i)
			{
				if (path == folderPath[i])
					return true;
			}
			return false;
		}
		void JApplicationVariable::MakeFolderPath(const std::string& projectName, const std::string& projectPath)
		{
			activatedProjectName = projectName;
			activatedProjectPath = projectPath + "\\" + activatedProjectName;

			contentPath = activatedProjectPath + "\\" + "Content";
			projectSettingPath = activatedProjectPath + "\\" + "ProjectSetting";
			libraryPath = activatedProjectPath + "\\" + "Library";
			shaderMetafilePath = libraryPath + "\\" + "ShaderMetafile";
			projectDefaultResourcePath = projectSettingPath + "\\" + "DefaultResource";
			projectEditorResoucePath = projectSettingPath + "\\" + "Editor";

			sceneFolderPath = contentPath + "\\" + "JScene";
			scriptFolderPath = contentPath + "\\" + "Script";
			resourceFolderPath = contentPath + "\\" + "Resource";
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
						MessageBox(0, L"폴더생성실패", JCommonUtility::U8StringToWstring(folderPath[i]).c_str(), 0);
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
