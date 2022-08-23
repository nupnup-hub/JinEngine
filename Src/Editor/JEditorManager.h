#pragma once   
#include"Event/JEditorEventType.h" 
#include"Event/JEditorEventStruct.h"
#include"Page/JEditorPageType.h"
#include"../Core/Event/JEventListener.h"
#include"../Core/JDataType.h" 
#include"../Core/Func/Functor/JFunctor.h"
#include<vector>
#include<unordered_map>
#include<functional>
#include<memory> 
#include<string> 

namespace JinEngine
{
	class JCamera;  
	class JScene;
	namespace Application
	{
		class JApplication;
	}
	namespace Editor
	{ 
		class JEditorUtility;
		class JEditorMenuBar;
		class JEditorPage;
		class JEditorWindow;

		class JEditorManager : public Core::JEventListener<size_t, J_EDITOR_EVENT, JEditorEventStruct*>
		{
		private:
			friend class Application::JApplication;
		private: 
			std::vector<std::unique_ptr<JEditorPage>> editorPage;
			std::unordered_map<J_EDITOR_PAGE_TYPE, JEditorPage*> editorPageMap;
			std::vector<JEditorPage*> opendEditorPage; 
			const std::string editorPageDataFileName = "EditorData.txt";
			const size_t editorManagerGuid;
		public:
			void SetEditorBackend();
			void OpenProjectSelector();
			void OpenProject();
			void Update();
			void Clear();
			void LoadPage();
			void StorePage();
		private:
			void OpenPage(JEditorOpenPageEvStruct* evStruct);
			void ClosePage(JEditorClosePageEvStruct* evStruct);
			void FrontPage(JEditorFrontPageEvStruct* evStruct);
			void BackPage(JEditorBackPageEvStruct* evStruct);
			void ActivatePage(JEditorActPageEvStruct* evStruct);
			void DeActivatePage(JEditorDeActPageEvStruct* evStruct);
			void FocusPage(JEditorFocusPageEvStruct* evStruct);
			void UnFocusPage(JEditorUnFocusPageEvStruct* evStruct);
		private:
			void OpenWindow(JEditorOpenWindowEvStruct* evStruct);
			void CloseWindow(JEditorCloseWindowEvStruct* evStruct); 
			void FrontWindow(JEditorFrontWindowEvStruct* evStruct);
			void BackWindow(JEditorBackWindowEvStruct* evStruct);
			void AcitvateWindow(JEditorActWindowEvStruct* evStruct);
			void DeActivateWindow(JEditorDeActWindowEvStruct* evStruct);
			void FocusWindow(JEditorFocusWindowEvStruct* evStruct);
			void UnFocusWindow(JEditorUnFocusWindowEvStruct* evStruct);
		private:
			std::string GetMetadataPath()noexcept; 
		private:
			virtual void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)final;
		private:
			JEditorManager();
			~JEditorManager();
			JEditorManager(const JEditorManager& rhs) = delete;
			JEditorManager& operator=(const JEditorManager& rhs) = delete;
		};
	}
}
