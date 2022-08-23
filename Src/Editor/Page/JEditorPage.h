#pragma once 
#include"JEditor.h" 
#include"JEditorPageType.h"

namespace JinEngine
{
	class JCamera;
	class JScene;
	namespace Editor
	{
		class JEditorAttribute; 
		class JEditorWindow;
		class JEditorMenuBar;
		class JEditorPage : public JEditor
		{
		protected: 
			std::unique_ptr<JEditorMenuBar> editorMenuBar;
			std::vector<JEditorWindow*>windows;
			std::vector<JEditorWindow*>opendWindow;
			JEditorWindow* focusWindow;
			std::string dockSpaceName;
			bool pageOpen;
		public:
			virtual void Initialize(bool hasImguiTxt) = 0;
			virtual void UpdatePage() = 0;
			void EnterPage(const int windowFlag, const int dockspaceFlag, bool isMainPage)noexcept;
			void ClosePage()noexcept;
		public:
			virtual J_EDITOR_PAGE_TYPE GetPageType()const noexcept = 0;
		public:
			virtual bool IsValidOpenRequest(JObject* selectedObj) noexcept = 0;
		public:
			void DoSetOpen()noexcept final;
			void DoOffOpen()noexcept final;
		public:
			void OpenWindow(const std::string& windowname)noexcept;
			void OpenWindow(JEditorWindow* window)noexcept;
			void CloseWindow(const std::string& windowname)noexcept;
			void CloseWindow(JEditorWindow* window)noexcept;
			void FrontWindow(JEditorWindow* window)noexcept;
			void BackWindow(JEditorWindow* window)noexcept;
			void ActivateWindow(JEditorWindow* window)noexcept;
			void DeActivateWindow(JEditorWindow* window)noexcept;
			void FocusWindow(JEditorWindow* window)noexcept;
			void UnFocusWindow(JEditorWindow* window)noexcept;
		protected:
			void UpdateWindowMenuBar();
			void PrintOpenWindowState();
		protected:
			void DoActivate() override;
			void DoDeActivate() override;
		private:
			JEditorWindow* FindEditorWindow(const std::string& windowName)const noexcept;
			bool IsWindowInVector(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept;
			int FindWindowIndex(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept;
		private:
			virtual void OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)final;
		public:
			virtual void StorePage(std::wofstream& stream);
			virtual void LoadPage(std::wifstream& stream);
		protected:
			JEditorPage(std::unique_ptr<JEditorAttribute> attribute, bool hasOpenInitObjType);
			~JEditorPage();
			JEditorPage(const JEditorPage& rhs) = delete;
			JEditorPage& operator=(const JEditorPage& rhs) = delete;
		};
	}
}