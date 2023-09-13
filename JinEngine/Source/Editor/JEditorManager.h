#pragma once    
#include"Event/JEditorEventType.h" 
#include"Event/JEditorEventStruct.h"
#include"Page/JEditorPageEnum.h" 
#include"../Core/Event/JEventListener.h"
#include"../Core/JCoreEssential.h"  
#include"../Core/Utility/JCommonUtility.h"
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
	namespace Graphic
	{
		struct JGuiInitData;
		class JGuiBackendInterface;
	}
	class JMain;
	namespace Editor
	{   
		class JEditorPage;
		class JProjectCloseConfirm;
		class JGuiBehaviorAdapter;
		class JGuiBackendAdapter;

		class JEditorManager final: public Core::JEventListener<size_t, J_EDITOR_EVENT, JEditorEvStruct*>
		{
		private:
			friend class JMain;
		private:
			struct PageSizeTransformInfo
			{
			public:
				JEditorPage* page = nullptr; 
				JVector2F prePos;
				JVector2F preSize;
				int destroyAfFrame = -1;	//reserve destroy when on previous size ev
			public:
				bool isMaximize;
			};
		private:
			const size_t editorManagerGuid;
		private: 
			std::vector<std::unique_ptr<JEditorPage>> editorPage;
			std::unordered_map<J_EDITOR_PAGE_TYPE, JEditorPage*> editorPageMap;
			std::vector<JEditorPage*> opendEditorPage;  
			std::vector<std::unique_ptr<PageSizeTransformInfo>> pageSizeTInfo;
		private:
			JEditorPage* focusPage = nullptr;
		public:
			void Initialize(std::unique_ptr<JGuiBehaviorAdapter>&& adapter);
			void Clear();
		public:
			Graphic::JGuiBackendInterface* GetBackendInterface();
		private:
			std::wstring GetMetadataPath()const noexcept;
		private:
			bool CanUpdate(JEditorPage* page)const noexcept;
			bool HasMaximizePage()const noexcept;
		public:
			void OpenProjectSelector(std::unique_ptr<Graphic::JGuiInitData>&& initData);
			void OpenProject(std::unique_ptr<Graphic::JGuiInitData>&& initData);
		public:
			void Update(); 
		private: 
			void UpdatePageSizeTransformLife();
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
			void MaximizePage(JEditorMaximizePageEvStruct* evStruct);
			void MinimizePage(JEditorMinimizePageEvStruct* evStruct);
			void PreviousSizePage(JEditorPreviousSizePageEvStruct* evStruct);
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
			void MaximizeWindow(JEditorMaximizeWindowEvStruct* evStruct); 
			void PreviousSizeWindow(JEditorPreviousSizeWindowEvStruct* evStruct);
		private:
			void PressMainWindowCloseButton()noexcept;
		private:
			int FindPageSizeTInfo(const size_t guid);
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
