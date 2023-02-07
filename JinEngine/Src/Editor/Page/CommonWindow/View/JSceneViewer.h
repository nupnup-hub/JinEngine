#pragma once
#include"../../JEditorWindow.h"    

namespace JinEngine
{  
	class JScene;
	namespace Editor
	{
		class JEditorCameraControl;
		class JSceneViewer final : public JEditorWindow
		{ 
		private: 
			Core::JUserPtr<JScene> scene;
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;
		public:
			JSceneViewer(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JSceneViewer();
			JSceneViewer(const JSceneViewer& rhs) = delete;
			JSceneViewer& operator=(const JSceneViewer& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept;
		public:
			void Initialize(Core::JUserPtr<JScene> newScene);
			void UpdateWindow()final;
		};
	}
}
