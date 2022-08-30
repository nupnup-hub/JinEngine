#pragma once
#include<string>
#include<vector>
#include"JApplicationState.h"
#include"../Core/Platform/JLanguageType.h"
#include"../Core/Platform/JPlatformInfo.h"

namespace JinEngine
{
	class ProjectSelector;
	namespace Application
	{
		class JApplication;
		class JApplicationVariable
		{
			friend class JApplication;
			friend class ProjectSelector;
		private:
			static std::wstring engineExePath;
			static std::wstring enginePath;
			static std::wstring enginePathW;
			static std::wstring defaultResourcePath;
			static std::wstring shaderPath;
			static std::wstring activatedProjectPath;
			static std::wstring activatedProjectName;

			static std::wstring contentPath;
			static std::wstring projectSettingPath;
			static std::wstring libraryPath;
			static std::wstring shaderMetafilePath;
			static std::wstring projectDefaultResourcePath;
			static std::wstring projectEditorResoucePath;

			static std::wstring sceneFolderPath;
			static std::wstring scriptFolderPath;
			static std::wstring resourceFolderPath;
			static std::vector<std::wstring> folderPath;

			static J_APPLICATION_STATE applicationState;
			static Core::J_LANGUAGE_TYPE engineLanguage;
		public:
			JApplicationVariable();
			~JApplicationVariable();
			JApplicationVariable(const JApplicationVariable& rhs) = delete;
			JApplicationVariable& operator=(const JApplicationVariable& rhs) = delete;

			static std::wstring GetEngineExePath()noexcept;
			static std::wstring GetEnginePath()noexcept; 
			static std::wstring GetDefaultResourcePath()noexcept;
			static std::wstring GetShaderPath()noexcept;
			static std::wstring GetActivatedProjectPath()noexcept;
			static std::wstring GetActivatedProjectName()noexcept;
			static std::wstring GetProjectPath()noexcept;
			static std::wstring GetProjectContentPath()noexcept;
			static std::wstring GetProjectSettingPath()noexcept;
			static std::wstring GetProjectLibraryPath()noexcept;
			static std::wstring GetProjectShaderMetafilePath()noexcept;
			static std::wstring GetProjectDefaultResourcePath()noexcept;
			static std::wstring GetProjectEditorResourcePath()noexcept;
			static std::wstring GetProjectContentScenePath()noexcept;
			static std::wstring GetProjectContentScriptPath()noexcept;
			static std::wstring GetProjectContentResourcePath()noexcept;
			static J_APPLICATION_STATE GetApplicationState()noexcept;
			static Core::J_LANGUAGE_TYPE GetEngineLanguageType()noexcept;
			static bool IsDefaultFolder(const std::wstring& path)noexcept;
		private:
			void MakeFolderPath(const std::wstring& projectName, const std::wstring& projectPath);
			bool AccessProjectFolder();
			void StartNewProject();
		};
	}
	using JApplicationVariable = Application::JApplicationVariable;
}