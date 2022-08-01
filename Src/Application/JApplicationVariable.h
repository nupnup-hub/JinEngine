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
			static std::string engineExePath;
			static std::string enginePath;
			static std::wstring enginePathW;
			static std::string defaultResourcePath;
			static std::string shaderPath;
			static std::string activatedProjectPath;
			static std::string activatedProjectName;

			static std::string contentPath;
			static std::string projectSettingPath;
			static std::string libraryPath;
			static std::string shaderMetafilePath;
			static std::string projectDefaultResourcePath;
			static std::string projectEditorResoucePath;

			static std::string sceneFolderPath;
			static std::string scriptFolderPath;
			static std::string resourceFolderPath;
			static std::vector<std::string> folderPath;

			static J_APPLICATION_STATE applicationState;
			static Core::J_LANGUAGE_TYPE engineLanguage;
		public:
			JApplicationVariable();
			~JApplicationVariable();
			JApplicationVariable(const JApplicationVariable& rhs) = delete;
			JApplicationVariable& operator=(const JApplicationVariable& rhs) = delete;

			static std::string GetEngineExePath()noexcept;
			static std::string GetEnginePath()noexcept;
			static std::wstring GetEnginePathW()noexcept;
			static std::string GetDefaultResourcePath()noexcept;
			static std::string GetShaderPath()noexcept;
			static std::string GetActivatedProjectPath()noexcept;
			static std::string GetActivatedProjectName()noexcept;
			static std::string GetProjectPath()noexcept;
			static std::string GetProjectContentPath()noexcept;
			static std::string GetProjectSettingPath()noexcept;
			static std::string GetProjectLibraryPath()noexcept;
			static std::string GetProjectShaderMetafilePath()noexcept;
			static std::string GetProjectDefaultResourcePath()noexcept;
			static std::string GetProjectEditorResourcePath()noexcept;
			static std::string GetProjectContentScenePath()noexcept;
			static std::string GetProjectContentScriptPath()noexcept;
			static std::string GetProjectContentResourcePath()noexcept;
			static J_APPLICATION_STATE GetApplicationState()noexcept;
			static Core::J_LANGUAGE_TYPE GetEngineLanguageType()noexcept;
			static bool IsDefaultFolder(const std::string& path)noexcept; 
		private:
			void MakeFolderPath(const std::string& projectName, const std::string& projectPath);
			bool AccessProjectFolder();
			void StartNewProject();
		};
	}
	using JApplicationVariable = Application::JApplicationVariable;
}