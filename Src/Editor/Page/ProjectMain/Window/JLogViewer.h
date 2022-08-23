#pragma once
#include"../../JEditorWindow.h" 
 
namespace JinEngine
{ 
	namespace Editor
	{
		class JLogViewer : public JEditorWindow
		{
		private:
			bool onSceneLog = false;
			bool onEditorLog = false;
		public:
			JLogViewer(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JLogViewer() = default;
			JLogViewer(const JLogViewer& rhs) = delete;
			JLogViewer& operator=(const JLogViewer& rhs) = delete;
		public:
			void Initialize()noexcept;
			void UpdateWindow()final;
		private:
			void BuildLogViewer();
		private:
			//¹Ì±¸Çö
			void SceneLogOnScreen();
			void EditorLogOnScreen();
		};
	}
}
