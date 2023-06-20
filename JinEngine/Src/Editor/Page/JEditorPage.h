#pragma once 
#include"JEditor.h" 
#include"JEditorPageEnum.h"  
#include"../Popup/JEditorPopupType.h"
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{ 
	namespace Core
	{
		class Core::JIdentifier;
	}
	namespace Editor
	{ 
		class JEditorAttribute; 
		class JEditorWindow;
		class JEditorPopupWindow;	
		class JEditorSimpleWindow;	//It isn't derived from JEditor .... used by testing or debugging

		class JEditorPage : public JEditor
		{ 
		protected:
			struct WindowInitInfo
			{
			public:
				std::string name; 
				bool isOpen;
				bool isLastAct;
			public:
				WindowInitInfo(const std::string name);
			public:
				std::string GetName()const noexcept;
				std::unique_ptr<JEditorAttribute> MakeAttribute()noexcept;
			};
		private:
			struct WinodwMaximizeInfo
			{ 
			public:
				JEditorWindow* window = nullptr; 
				std::vector<uint> preTabItemID;
				int destroyAfFrame = -1;	//reserve destroy when on previous size ev
			};
		private:
			using OpenEditorWindowF = Core::JSFunctorType<void, JEditorPage&, const std::string>;
			using OpenSimpleWindowF = Core::JSFunctorType<void, bool*>;
			using CloseEditorWindowF = Core::JSFunctorType<void, JEditorPage&, const std::string>;
			using CloseSimpleWindowF = Core::JSFunctorType<void, bool*>;
			using PopupWndFuncTuple = std::tuple <J_EDITOR_POPUP_WINDOW_FUNC_TYPE, std::unique_ptr< Core::JBindHandleBase>>;
		private:
			std::vector<JEditorWindow*>windows;
			std::vector<JEditorWindow*>opendWindow;
			JEditorWindow* focusWindow = nullptr;   
			J_EDITOR_PAGE_FLAG pageFlag;
			bool isPageOpen;
		private:
			//popup only use by editor page
			std::vector<JEditorPopupWindow*>popupWindow;
			JEditorPopupWindow* opendPopupWindow = nullptr;
			JEditorPopupWindow* closeConfirmPopupWindow = nullptr;
		private:
			std::vector<JEditorSimpleWindow*> simpleWindow;
		private:
			std::unique_ptr<WinodwMaximizeInfo> maximizeInfo;
		private:
			std::unique_ptr<OpenEditorWindowF::Functor> openEditorWindowFunctor;
			std::unique_ptr<OpenSimpleWindowF::Functor> openSimpleWindowFunctor;
			std::unique_ptr<CloseEditorWindowF::Functor> closeEditorWindowFunctor;
			std::unique_ptr<CloseSimpleWindowF::Functor> closeSimpleWindowFunctor;
		public:
			JEditorPage(const std::string name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_FLAG pageFlag);
			~JEditorPage();
			JEditorPage(const JEditorPage& rhs) = delete;
			JEditorPage& operator=(const JEditorPage& rhs) = delete;
		protected:
			void AddWindow(const std::vector<JEditorWindow*>& wnd)noexcept;
			void AddPopupWindow(const std::vector<JEditorPopupWindow*>& wnd)noexcept;
			void AddSimpleWindow(const std::vector<JEditorSimpleWindow*>& wnd)noexcept;
		public:
			J_EDITOR_PAGE_FLAG GetPageFlag()const noexcept; 
			void SetPageFlag(const J_EDITOR_PAGE_FLAG flag)noexcept;
		public:
			virtual J_EDITOR_PAGE_TYPE GetPageType()const noexcept = 0;
			virtual void SetInitWindow() = 0;
		protected:
			uint GetOpenWindowCount()const noexcept; 
			JEditorWindow* GetOpenWindow(const uint index)const noexcept; 
			JEditorPopupWindow* GetOpenPopupWindow()const noexcept;
			std::vector<JEditorWindow*> GetWindowVec()const noexcept;
		protected:
			bool CanUpdate(JEditorWindow* wnd)const noexcept;
			bool HasDockNodeSpace()const noexcept;
			bool HasMaximizedWindow()const noexcept; 
		public: 
			virtual void Initialize();
			virtual void Clear();
			virtual void UpdatePage() = 0;
			void EnterPage(int guiWindowFlag)noexcept;
			void ClosePage()noexcept;  
		protected:
			void UpdateOpenWindow();
			void UpdateOpenPopupWindow(const JVector2<float> pagePos, const JVector2<float> pageSize);
			void UpdateOpenSimpleWindow();
		public:
			virtual bool IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj) noexcept = 0;
		public:
			void OpenWindow(const std::string& windowname)noexcept;
			void OpenWindow(JEditorWindow* window)noexcept;
			void CloseWindow(const std::string& windowname)noexcept;
			void CloseWindow(JEditorWindow* window)noexcept; 
			void ActivateWindow(JEditorWindow* window)noexcept;
			void DeActivateWindow(JEditorWindow* window)noexcept;
			void FocusWindow(JEditorWindow* window)noexcept;
			void UnFocusWindow(JEditorWindow* window)noexcept;
			void MaximizeWindow(JEditorWindow* window)noexcept;
			void PreviousSizeWindow(JEditorWindow* window)noexcept;
		public:
			void OpenPopupWindow(const J_EDITOR_POPUP_WINDOW_TYPE popupType, 
				const std::string& desc = "", 
				std::vector<PopupWndFuncTuple>&& tupleVec = {});
			void OpenPopupWindow(JEditorPopupWindow* popupWindow,
				const std::string& desc = "", 
				std::vector<PopupWndFuncTuple>&& tupleVec = {});
			void ClosePopupWindow(const J_EDITOR_POPUP_WINDOW_TYPE popupType);
			void ClosePopupWindow(JEditorPopupWindow* popupWindow);
		protected:
			OpenEditorWindowF::Functor* GetOpenEditorWindowFunctorPtr()noexcept;
			OpenSimpleWindowF::Functor* GetOpenSimpleWindowFunctorPtr()noexcept;
			CloseEditorWindowF::Functor* GetCloseEditorWindowFunctorPtr()noexcept;
			CloseSimpleWindowF::Functor* GetCloseSimpleWindowFunctorPtr()noexcept;
		protected:
			void UpdateDockSpace(const int dockspaceFlag); 
			void PrintOpenWindowState();	//unuse
		protected:
			void DoSetOpen()noexcept override;
			void DoSetClose()noexcept override; 
			void DoActivate()noexcept override;
			void DoDeActivate()noexcept override; 
		private:
			JEditorWindow* FindEditorWindow(const std::string& windowName)const noexcept;
			JEditorPopupWindow* FindEditorPopupWindow(const J_EDITOR_POPUP_WINDOW_TYPE popupType)const noexcept;
			bool IsWindowInVector(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept;
			int FindWindowIndex(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept;
		public:
			virtual void StorePage(std::wofstream& stream);
			virtual void LoadPage(std::wifstream& stream);
		};
	}
}