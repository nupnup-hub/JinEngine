#pragma once    
#include"Event/JEditorEventType.h" 
#include"Event/JEditorEventStruct.h"
#include"Page/JEditorPageEnum.h"
#include"../Application/JApplicationCloseType.h"
#include"../Core/Event/JEventListener.h"
#include"../Core/JDataType.h"  
#include<vector>
#include<unordered_map>
#include<functional>
#include<memory> 
#include<string> 

namespace JinEngine
{ 
	namespace Application
	{
		class JApplication;
	}
	namespace Editor
	{   
		class JEditorPage;
		class JProjectCloseConfirm;

		class JEditorManager : public Core::JEventListener<size_t, J_EDITOR_EVENT, JEditorEvStruct*>
		{
		private:
			friend class Application::JApplication;
		private:
			const size_t editorManagerGuid; 
		private: 
			std::vector<std::unique_ptr<JEditorPage>> editorPage;
			std::unordered_map<J_EDITOR_PAGE_TYPE, JEditorPage*> editorPageMap;
			std::vector<JEditorPage*> opendEditorPage;  
		public:
			void Initialize();
			void Clear();
		public:
			void OpenProjectSelector(); 
			void OpenProject();  
		public:
			void Update(); 
		public:
			void LoadPage();
			void StorePage();
		private:
			void OpenPage(JEditorOpenPageEvStruct* evStruct);
			void ClosePage(JEditorClosePageEvStruct* evStruct); 
			void ActivatePage(JEditorActPageEvStruct* evStruct);
			void DeActivatePage(JEditorDeActPageEvStruct* evStruct);
			void FocusPage(JEditorFocusPageEvStruct* evStruct);
			void UnFocusPage(JEditorUnFocusPageEvStruct* evStruct);
		private:
			void OpenWindow(JEditorOpenWindowEvStruct* evStruct);
			void CloseWindow(JEditorCloseWindowEvStruct* evStruct);  
			void AcitvateWindow(JEditorActWindowEvStruct* evStruct);
			void DeActivateWindow(JEditorDeActWindowEvStruct* evStruct);
			void FocusWindow(JEditorFocusWindowEvStruct* evStruct);
			void UnFocusWindow(JEditorUnFocusWindowEvStruct* evStruct); 
		private:
			void OpenPopupWindow(JEditorOpenPopupWindowEvStruct* evStruct);
			void ClosePopupWindow(JEditorClosePopupWindowEvStruct* evStruct);
		private:
			std::wstring GetMetadataPath()const noexcept;  
		private:
			void PressMainWindowCloseButton()noexcept;
		private:
			virtual void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		private:
			JEditorManager();
			~JEditorManager();
			JEditorManager(const JEditorManager& rhs) = delete;
			JEditorManager& operator=(const JEditorManager& rhs) = delete;
		};
	}
}
