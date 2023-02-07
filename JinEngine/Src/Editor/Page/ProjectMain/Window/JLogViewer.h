#pragma once
#include"../../JEditorWindow.h" 
 
namespace JinEngine
{ 
	namespace Editor
	{
		class JLogViewer final : public JEditorWindow
		{ 
		private:
			bool onSceneLog = false;
			bool onEditorLog = false;
		public:
			JLogViewer(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JLogViewer() = default;
			JLogViewer(const JLogViewer& rhs) = delete;
			JLogViewer& operator=(const JLogViewer& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
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
