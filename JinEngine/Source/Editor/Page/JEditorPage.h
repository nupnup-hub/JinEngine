#pragma once 
#include"JEditor.h" 
#include"JEditorPageEnum.h"  
#include"../Popup/JEditorPopupType.h"
#include"../Gui/JGuiType.h"
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
			struct WinodowMaximizeInfo
			{ 
			public:
				JEditorWindow* window = nullptr; 
				std::vector<GuiID> preTabItemID; 
				JVector2F prePos;
				JVector2F preSize;
				int destroyAfFrame = -1;	//reserve destroy when on previous size ev
			};
			struct State
			{
			public: 
				JVector2F nextPos;
				JVector2F nextSize; 
			public:
				bool hasSetNextPosReq = false;	
				bool hasSetNextSizeReq = false;	
			public:
				bool isPageOpen = false;
				bool isInputLock = false;
				bool isMaximize = false;
				bool isMinimize = false;
			};
		private:
			using OpenEditorWindowF = Core::JSFunctorType<void, JEditorPage&, const std::string>; 
			using CloseEditorWindowF = Core::JSFunctorType<void, JEditorPage&, const std::string>; 
			using PopupWndFuncTuple = std::tuple <J_EDITOR_POPUP_WINDOW_FUNC_TYPE, std::unique_ptr< Core::JBindHandleBase>>;
		private:
			J_EDITOR_PAGE_FLAG pageFlag;
			State state;
		private:
			std::vector<JEditorWindow*>windows;
			std::vector<JEditorWindow*>opendWindow;
			std::unordered_map<std::string, JEditorWindow*> windowMap;
			JEditorWindow* focusWindow = nullptr;   
		private:
			//popup only use by editor page
			std::vector<JEditorPopupWindow*>popupWindow;
			JEditorPopupWindow* opendPopupWindow = nullptr;
			JEditorPopupWindow* closeConfirmPopupWindow = nullptr; 
		private:
			std::unique_ptr<WinodowMaximizeInfo> maximizeInfo;
		private:
			std::unique_ptr<OpenEditorWindowF::Functor> openEditorWindowFunctor; 
			std::unique_ptr<CloseEditorWindowF::Functor> closeEditorWindowFunctor; 
		public:
			JEditorPage(const std::string name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_FLAG pageFlag);
			~JEditorPage();
			JEditorPage(const JEditorPage& rhs) = delete;
			JEditorPage& operator=(const JEditorPage& rhs) = delete;
		protected:
			void AddWindow(const std::vector<JEditorWindow*>& wnd)noexcept;
			void AddPopupWindow(const std::vector<JEditorPopupWindow*>& wnd)noexcept; 
		public:
			J_EDITOR_PAGE_FLAG GetPageFlag()const noexcept;
			bool GetPublicState(const J_EDITOR_PAGE_PUBLIC_STATE stateType)const noexcept;
			virtual J_EDITOR_PAGE_TYPE GetPageType()const noexcept = 0;
		protected:
			uint GetOpenWindowCount()const noexcept;
			JEditorWindow* GetOpenWindow(const uint index)const noexcept;
			JEditorPopupWindow* GetOpenPopupWindow()const noexcept;
			std::vector<JEditorWindow*> GetWindowVec()const noexcept;
			GuiID GetWindowID()const noexcept;
			GuiID GetDockSpaceID()const noexcept;
		public:
			void SetPageFlag(const J_EDITOR_PAGE_FLAG flag)noexcept;
			void SetNextPagePos(const JVector2F& pos)noexcept;
			void SetNextPageSize(const JVector2F& size)noexcept;
			void SetInputLock(const bool value)noexcept;
			void SetMaximize(const bool value)noexcept;
			void SetMinimize(const bool value);
			virtual void SetInitWindow() = 0;
		public:
			virtual bool IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj) noexcept = 0;
			bool HasWindow(const std::string& name)noexcept;
		protected:
			bool CanUpdate(JEditorWindow* wnd)const noexcept;
			bool CanClose()const noexcept;
			bool CanMaximize()const noexcept;
			bool CanMinimize()const noexcept;
			bool HasDockNodeSpace()const noexcept;
			bool HasMaximizedWindow()const noexcept; 
		public: 
			virtual void Initialize();
			virtual void Clear();
		protected:
			void EnterPage(J_GUI_WINDOW_FLAG_ guiWindowFlag)noexcept;
			void ClosePage()noexcept;
		public:
			virtual void UpdatePage() = 0;
			void UpdateOpenPopupWindow(const JVector2<float>& pagePos, const JVector2<float>& pageSize);
		protected:
			void UpdateOpenWindow(); 
			void UpdateDockSpace(J_GUI_DOCK_NODE_FLAG_ dockspaceFlag); 
		private:
			void UpdateMaximizeLife();
		public:
			void OpenWindow(const std::string& windowname)noexcept;
			void OpenWindow(JEditorWindow* window)noexcept;
			void CloseWindow(const std::string& windowname)noexcept;
			void CloseWindow(JEditorWindow* window)noexcept; 
			void ActivateWindow(JEditorWindow* window)noexcept;
			void DeActivateWindow(JEditorWindow* window)noexcept;
			void FocusWindow(JEditorWindow* window)noexcept;
			void FocusWindow(const std::string& windowName)noexcept;
			void UnFocusWindow(JEditorWindow* window)noexcept;
			void MaximizeWindow(JEditorWindow* window, const JVector2F& prePos, const JVector2F& preSize)noexcept;
			void PreviousSizeWindow(JEditorWindow* window, const bool useLazy)noexcept;
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
			CloseEditorWindowF::Functor* GetCloseEditorWindowFunctorPtr()noexcept; 
		protected:
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