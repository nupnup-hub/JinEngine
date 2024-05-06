/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once 
#include"../JApplicationType.h"  
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/File/JFileIOResult.h"

namespace JinEngine
{ 
	//friend class Editor::JProjectMainPage;
	//friend class Editor::JProjectSelectorHub;
	class JMain;
	namespace Application
	{ 
		class JApplicationProjectInfo;
		class JApplicationProjectPrivate final
		{ 
		public: 
			using SetAppStateF = Core::JFunctor<void, const J_APPLICATION_STATE>;
		public:
			class MainAccess
			{
			private:
				friend class JMain;
			private:
				static void RegisterFunctor(std::unique_ptr<SetAppStateF>&& setStateF);
				static bool Initialize();
			private:
				/** Load Other Project order
				* 1. BeginLoadOtherProject()	//set trigger
				* 2. End window proccess		// loop
				* 3. Begin End Project order
				*/
				static void BeginLoadOtherProject(std::unique_ptr<JApplicationProjectInfo>&& nextProjInfo)noexcept;
				/** End Project order
				* 1. BeginCloseProject()	//set trigger
				* 2. End window proccess		// loop
				* 3.0 CancelCloseProject()	-> cancel close project
				* 3.1.0 Call ConfirmCloseProject()
				* 3.1.1 CloseProject()
				* 3.2 if set load trigger ->	restart new project
				*/
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
			private:
				static std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(const std::wstring& projectPath)noexcept;
				static std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(const std::wstring& projectPath, const std::string& pVersion)noexcept;
				static std::unique_ptr<JApplicationProjectInfo> MakeProjectInfo(JApplicationProjectInfo* info)noexcept;
			private:
				static void SetNextProjectInfo(std::unique_ptr<JApplicationProjectInfo>&& nextProjectInfo)noexcept;
			private:
				/** Start project order
				* 1. SetNextProjectInfo()
				* 2. SetStartNewProjectTrigger()
				* 3. Call Intiailize()
				*/
				static bool SetStartNewProjectTrigger();
			private:
				static void CancelCloseProject()noexcept;
				static void ConfirmCloseProject()noexcept;
			private:
				/**
				* can destroy project in PROJECT_SELECT state
				* 추후에 다른 state에서도 삭제가능하게 추가필요.
				*/
				static void DestroyProject(const int projectIndex)noexcept;
			};
			class IOInterface
			{
			private:
				friend class JMain; 
			private:
				static Core::J_FILE_IO_RESULT StoreProjectVersion(const std::string& pVersion);
				static Core::J_FILE_IO_RESULT LoadProejctVersion(_Out_ std::string& pVersion);
			private:
				//Store & Load project info
				static void StoreProjectList();
				static void LoadProjectList();
			};
		}; 
	}
	using JApplicationProjectPrivate = Application::JApplicationProjectPrivate;
}