#pragma once
#include<string> 
#include"JApplicationState.h"  
#include"../Core/Func/Functor/JFunctor.h"
#include"../Core/File/JFileIOResult.h"

namespace JinEngine
{
	namespace Editor
	{
		class JProjectSelectorHub;
		class JProjectMainPage;
	}

	//friend class Editor::JProjectMainPage;
	//friend class Editor::JProjectSelectorHub;
	class JMain;
	namespace Application
	{ 
		class JApplicationProjectInfo;
		class JApplicationProjectPrivate final
		{ 
		public:
			using LoadProjectF = Core::JFunctor<void>;
			using StoreProjectF = Core::JFunctor<void>;
			using SetAppStateF = Core::JFunctor<void, const J_APPLICATION_STATE>;
		public:
			class MainAccess
			{
			private:
				friend class JMain;
			private:
				static void RegisterFunctor(std::unique_ptr<LoadProjectF>&& loadF, 
					std::unique_ptr<StoreProjectF>&& storeF,
					std::unique_ptr<SetAppStateF>&& setStateF);
				static bool Initialize();
			private:
				//Load Project order
				//1. BeginLoadOtherProject()	//set trigger
				//2. End window proccess		// loop
				//3. End Project order
				static void BeginLoadOtherProject()noexcept;
				//End Project order
				//1. BeginCloseProject()	//set trigger
				//2. End window proccess		// loop
				//3.0 CancelCloseProject()	-> cancel close project
				//3.1.0 Call ConfirmCloseProject()
				//3.1.1 CloseProject()
				//3.2 if set load trigger ->	restart new project
				static void BeginCloseProject()noexcept;
			private:
				static void CloseProject()noexcept;
			private:
				static bool CanStartProject()noexcept;
				static bool CanEndProject()noexcept;
			};
			class LifeInterface
			{
			private:
				friend class JMain;
				friend class Editor::JProjectMainPage;
				friend class Editor::JProjectSelectorHub;
			private:
				static std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(const std::wstring& projectPath)noexcept;
				static std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(const std::wstring& projectPath, const std::string& pVersion)noexcept;
				static std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(JApplicationProjectInfo* info)noexcept;
			private:
				static void SetNextProjectInfo(std::unique_ptr<JApplicationProjectInfo>&& nextProjectInfo)noexcept;
			private:
				//Start project order
				//1. SetNextProjectInfo()
				//2. SetStartNewProjectTrigger()
				//3. Call Intiailize()
				static bool SetStartNewProjectTrigger();
			private:
				static void CancelCloseProject()noexcept;
				static void ConfirmCloseProject()noexcept;
			};
			class IOInterface
			{
			private:
				friend class JMain;
				friend class Editor::JProjectMainPage;
				friend class Editor::JProjectSelectorHub; 
			private:
				static Core::J_FILE_IO_RESULT StoreProjectVersion(const std::string& pVersion);
				static Core::J_FILE_IO_RESULT LoadProejctVersion(_Out_ std::string& pVersion);
			private:
				//Store & Load project resource
				static void StoreProject();
				static void LoadProject();
			private:
				//Store & Load project info
				static void StoreProjectList();
				static void LoadProjectList();
			};
		}; 
	}
	using JApplicationProjectPrivate = Application::JApplicationProjectPrivate;
}