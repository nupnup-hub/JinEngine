#pragma once   
#include<vector>
#include<unordered_map>
#include<functional>
#include<memory> 
#include<string>
#include"Event/EditorEventType.h"
#include"Event/EditorEventStruct.h"
#include"../../Core/Event/JEventListener.h"
#include"../../Core/JDataType.h" 

namespace JinEngine
{
	class ImGuiManager; 
	class EditorUtility;
	class EditorMenuBar;
	class EditorWindowPage;
	class EditorWindow;
	class JCamera;  
	class JScene;

	class EditorManager : public Core::JEventListener<size_t, EDITOR_EVENT, EditorEventStruct*>
	{
	private:
		std::unique_ptr<ImGuiManager> imguiManager;

		std::vector<EditorWindow*> allEditorWindows;
		std::unordered_map<size_t, EditorWindow*> allEditorWindowsMap;
		std::vector<std::unique_ptr<EditorWindowPage>> editorPage;
		std::unordered_map<size_t, EditorWindowPage*> editorPageMap;
		std::vector<EditorWindowPage*> opendEditorPage;
		std::unique_ptr<EditorUtility> editorUtility;

		std::vector<std::unique_ptr<EditorEventStruct>> activationWindowQueue;
		std::vector<std::unique_ptr<EditorEventStruct>> openCloseWindowQueue;
		std::vector<std::unique_ptr<EditorEventStruct>> openClosePageQueue;

		const std::string editorPageDataFileName = "EditorData.txt"; 
		const size_t editorManagerGuid;
	public:
		EditorManager();
		~EditorManager();
		EditorManager(const EditorManager& rhs) = delete;
		EditorManager& operator=(const EditorManager& rhs) = delete;

		void SetEditorBackend();
		void OpenProjectSelector();
		void OpenProject(const std::string& mainSceneName);
		void Update();
		void Clear();
		void LoadPage(const std::string& sceneName);
		void StorePage(const std::string& sceneName);
	private: 
		void OpenAndClosePage();
		// OpenAndCloseWindow In OpendPage Vector
		void OpenAndCloseWindow();
		// ActivationWindow In OpendWindow Vector
		void ActivationWindow();
		std::string GetEditorDataPath(const std::string& sceneName)noexcept;
	private:
		virtual void OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)final;
	};


}
