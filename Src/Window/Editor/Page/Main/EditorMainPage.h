#pragma once  
#include"../EditorWindowPage.h"  

namespace JinEngine
{
	class LogViewer;
	class ObjectDetail;
	class ObjectExplorer;
	class MainSceneEditor;
	class SceneViewer;
	class WindowDirectory;
	class AnimationControllerEditor;
	class GraphicResourceWatcher;
	class StringConvertTest;

	class EditorMainPage : public EditorWindowPage
	{
	private:
		std::unique_ptr<WindowDirectory>windowDirectory;
		std::unique_ptr<ObjectExplorer>objectExplorer;
		std::unique_ptr<MainSceneEditor>sceneEditor;
		std::unique_ptr<ObjectDetail>objectDetail;
		std::unique_ptr<LogViewer>logViewer; 	
		std::unique_ptr<SceneViewer> sceneViewer;
		std::unique_ptr<AnimationControllerEditor> animationControllerEditor;
		std::unique_ptr<GraphicResourceWatcher> graphicResourceWatcher;
		std::unique_ptr<StringConvertTest> stringConvertTest;

	public:
		EditorMainPage();
		~EditorMainPage() = default;
		EditorMainPage(const EditorMainPage& rhs) = delete;
		EditorMainPage& operator=(const EditorMainPage& rhs) = delete;

		void Initialize(EditorUtility* editorUtility, std::vector<EditorWindow*>& allEditorWindows, bool hasImguiTxt)override; 
		void UpdatePage(EditorUtility* editorUtility)override;
	};
}
