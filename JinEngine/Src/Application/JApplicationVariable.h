#pragma once
#include<string>
#include<vector>
#include<deque>
#include"JApplicationState.h" 
#include"../Core/Platform/JLanguageType.h"
#include"../Core/Platform/JPlatformInfo.h"
#include"../Core/Func/Functor/JFunctor.h"
#include"../Core/File/JFileIOResult.h"
#include"../Core/Time/JRealTime.h"
#include"../Utility/JMacroUtility.h"
 
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
			static std::wstring GetEngineProjectLastRsPath()noexcept; 
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
			static std::wstring GetProjectCashPath()noexcept;
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
		private:
			using JTime = Core::JRealTime::JTime;
		public:
			class JProjectInfo
			{
			private:
				const size_t guid;
			private:
				std::wstring name;
				std::wstring path;
				std::wstring version;
				JTime createdTime;
				JTime lastUpdateTime;
			public:
				JProjectInfo(const size_t guid,
					const std::wstring& name,
					const std::wstring& path, 
					const std::wstring& version, 
					const JTime& createdTime,
					const JTime& lastUpdateTime);
			public:
				J_SIMPLE_GET(size_t, guid, Guid)
				J_SIMPLE_GET(std::wstring, name, Name)
				J_SIMPLE_GET(std::wstring, path, Path)
				J_SIMPLE_GET(std::wstring, version, Version)
				J_SIMPLE_GET(JTime, lastUpdateTime, LastUpdateTime)
				J_SIMPLE_GET(JTime, createdTime, CreatedTime) 
				std::unique_ptr<JProjectInfo> GetUnique()const noexcept;
				std::wstring lastRsPath()const noexcept;
			};
		public:
			static JProjectInfo* GetOpenProjectInfo()noexcept;
		private:
			static uint GetProjectInfoCount()noexcept;
			static JProjectInfo* GetProjectInfo(uint index)noexcept;
			static JProjectInfo* GetProjectInfo(const std::wstring& projectPath)noexcept; 
		private:
			static void SetNextProjectInfo(std::unique_ptr<JProjectInfo>&& nextProjectInfo)noexcept;
			static void SetProjectFolderPath(const std::wstring& projectName, const std::wstring& projectPath);
		private:
			//Load Project order
			//1. TryLoadOtherProject()
			//2. End window proccess
			//3. End Project order
			static void TryLoadOtherProject()noexcept;
			static void TryCloseProject()noexcept;
			static void CancelCloseProject()noexcept;
			//End Project order
			//1. TryCloseProject()
			//2.0 CancelCloseProject()	-> cancel close project
			//2.1 Call ConfirmCloseProject() -> close project
			//2.2 if set load trigger ->	restart new project
			static void ConfirmCloseProject()noexcept;
		private: 
			static std::unique_ptr<JProjectInfo> MakeProjectInfo(const std::wstring& projectPath)noexcept;
			static std::unique_ptr<JProjectInfo> MakeProjectInfo(const std::wstring& projectPath, const std::string& pVersion)noexcept;
			static bool MakeProjectFolder();
			static bool MakeProjectVersionFile(const std::string& pVersion);
			//Start project order
			//1. SetNextProjectInfo()
			//2. SetStartNewProjectTrigger()
			//3. Call Intiailize()
			static bool SetStartNewProjectTrigger();
			static bool Initialize();
		private:
			static bool IsValidVersion(const std::string& pVersion); 
			static bool IsValidPath(const std::wstring& projectPath)noexcept;
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