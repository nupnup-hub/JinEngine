#pragma once 
#include"Editor.h"
#include"../Event/EditorEventType.h"
#include"../Event/EditorEventStruct.h"  
#include"../../../Core/Event/JEventListener.h"

namespace JinEngine
{
	class JCamera; 
	class JScene;
	class EditorAttribute;
	class EditorWindowGroup; 
	class EditorWindow;
	class EditorMenuBar; 

	class EditorWindowPage : public Editor , public Core::JEventListener<size_t, EDITOR_EVENT, EditorEventStruct*>
	{ 
	protected:   
		std::unique_ptr<EditorMenuBar> editorMenuBar; 
		std::vector<EditorWindow*>windows;
		std::vector<EditorWindow*>opendWindow;
		EditorWindow* focusWindow;
		std::string dockSpaceName;
		bool pageOpen;
	public:  
		EditorWindowPage(std::unique_ptr<EditorAttribute> attribute);
		virtual ~EditorWindowPage();
		EditorWindowPage(const EditorWindowPage& rhs) = delete;
		EditorWindowPage& operator=(const EditorWindowPage& rhs) = delete;

		virtual void Initialize(EditorUtility* editorUtility, std::vector<EditorWindow*>& allEditorWindows, bool hasImguiTxt) = 0;
		bool Activate(EditorUtility* editorUtility) override;
		bool DeActivate(EditorUtility* editorUtility) override;
		virtual void UpdatePage(EditorUtility* editorUtility) = 0;
		virtual void StorePage(std::wofstream& stream);
		virtual void LoadPage(std::wifstream& stream, std::vector<EditorWindow*>& allEditorWindows, std::vector<EditorWindowPage*>& opendEditorPage, EditorUtility* editorUtility);
		 
		void EnterPage(const int windowFlag, const int dockspaceFlag, bool isMainPage)noexcept;
		void ClosePage(EditorUtility* editorUtility)noexcept;
		void OpenWindow(EditorWindow* window, EditorUtility* editorUtility)noexcept;
		void CloseWindow(EditorWindow* window, EditorUtility* editorUtility)noexcept;
		uint GetMemeberCount()const noexcept; 
	protected:	
		void UpdateWindowMenuBar(EditorUtility* editorUtility);
		/*Debug*/
		void PrintOpenWindowState();
	private: 
		EditorWindow* FindEditorWindow(const std::string& windowName)const noexcept;
		bool IsWindowInVector(EditorWindow* window, std::vector<EditorWindow*>::const_iterator st, const uint count)const noexcept;
		int FindWindowIndex(EditorWindow* window, std::vector<EditorWindow*>::const_iterator st, const uint count)const noexcept;
	private:
		virtual void OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)final;
	};
}