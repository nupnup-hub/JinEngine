#pragma once
#include<string>
#include<vector>
#include<deque>
#include"JApplicationState.h" 
#include"../Core/Platform/JLanguageType.h"
#include"../Core/Platform/JPlatformInfo.h"
#include"../Core/Func/Functor/JFunctor.h"
#include"../Core/File/JFileIOResult.h"

namespace JinEngine
{
	namespace Editor
	{
		class JProjectSelectorHub; 
		class JProjectMainPage;
	}
	namespace Application
	{
		class JApplication; 
		class JApplicationProject;

		class JApplicationVariable
		{
		private:
			friend class JApplication; 
			friend class JApplicationProject;
		public:
			using TimeVec = std::vector<std::tuple<std::string, float>>;
			using AppCommandBind = Core::JBindHandleBase;
			using StoreProjectF = Core::JMFunctorType<JApplication, void>;
			using LoadProjectF = Core::JMFunctorType<JApplication, void>;
		public:
			static std::string GetLatestVersion()noexcept;
			static int GetSubverionDigitRange()noexcept;
			static std::vector<std::string> GetAppVersion()noexcept;
		public:
			static std::wstring GetEngineExePath()noexcept;
			static std::wstring GetEnginePath()noexcept; 
			static std::wstring GetEngineResourcePath();
			static std::wstring GetEngineDefaultResourcePath()noexcept;
			static std::wstring GetEngineInfoPath()noexcept;
			static std::wstring GetEngineProjectListFilePath()noexcept; 
			static std::wstring GetShaderPath()noexcept;
		public:
			static std::wstring GetActivatedProjectPath()noexcept;
			static std::wstring GetActivatedProjectName()noexcept;
			static std::wstring GetProjectPath()noexcept;
			static std::wstring GetProjectContentPath()noexcept;
			static std::wstring GetProjectSettingPath()noexcept;
			static std::wstring GetProjectLibraryPath()noexcept;
			static std::wstring GetProjectShaderMetafilePath()noexcept;
			static std::wstring GetProjectDefaultResourcePath()noexcept;
			static std::wstring GetProjectEditorResourcePath()noexcept;
			static std::wstring GetProjectResourceCashPath()noexcept;
			static std::wstring GetProjectVersionFilePath()noexcept;
			static std::wstring GetProjectContentScenePath()noexcept;
			static std::wstring GetProjectContentScriptPath()noexcept;
			static std::wstring GetProjectContentResourcePath()noexcept;
		public:
			static J_APPLICATION_STATE GetApplicationState()noexcept;
			static Core::J_LANGUAGE_TYPE GetEngineLanguageType()noexcept;
			static bool IsDefaultFolder(const std::wstring& path)noexcept;
		public:
			static void AddTime(const std::string& str, float time)noexcept;
			static void UpdateTime(int index, float time)noexcept;
			static void ClearTime()noexcept;
			static TimeVec GetTime()noexcept;
		private:
			static void SetApplicationState(const J_APPLICATION_STATE newState)noexcept;
		private:
			static void Initialize(); 
			static bool MakeEngineFolder(); 
		private:
			static void RegisterFunctor(JApplication* app, StoreProjectF::Ptr storeF, LoadProjectF::Ptr loadF);
			static void ExecuteAppCommand();
		};

		class JApplicationProject
		{
		private:
			friend class JApplication;
			friend class Editor::JProjectMainPage;
			friend class Editor::JProjectSelectorHub; 
		public:
			class JProjectInfo
			{
			private:
				std::wstring name;
				std::wstring path;
				std::wstring version;
			public:
				JProjectInfo(const std::wstring& name, const std::wstring& path, std::wstring  version);
			public:
				std::wstring GetName()const noexcept;
				std::wstring GetPath()const noexcept;
				std::wstring GetVersion()const noexcept;
			};
		private:
			static uint GetProjectInfoCount()noexcept;
			static JProjectInfo* GetProjectInfo(uint index)noexcept;
		private:
			static void SetNextProjectInfo(std::unique_ptr<JProjectInfo>&& nextProjectInfo)noexcept;
		private:
			static void TryLoadOtherProject()noexcept;
			static void TryCloseProject()noexcept;
			static void CancelCloseProject()noexcept;
			static void ConfirmCloseProject()noexcept;
		private:
			static std::unique_ptr<JProjectInfo> MakeProjectInfo(const std::wstring& projectPath);
			static void MakeProjectFolderPath(const std::wstring& projectName, const std::wstring& projectPath);
			static bool MakeProjectFolder();
			static bool MakeProjectVersionFile(const std::string& pVersion);
			static bool StartNewProject();
			static bool Initialize();
		private:
			static bool IsValidVersion(const std::string& pVersion); 
			static bool CanStartProject()noexcept; 
			static bool CanEndProject()noexcept;
		private:
			static Core::J_FILE_IO_RESULT StoreProjectVersion(const std::string& pVersion);
			static Core::J_FILE_IO_RESULT LoadProejctVersion(_Out_ std::string& pVersion);
		private:
			static void StoreProject();
			static void LoadProject();
		private:
			static void StoreProjectList();
			static void LoadProjectList();
		};
	}
	using JApplicationVariable = Application::JApplicationVariable;
	using JApplicationProject = Application::JApplicationProject;
}