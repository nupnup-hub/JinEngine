#pragma once
#include"../../JEditorWindow.h"    

namespace JinEngine
{  
	class JScene;
	class JCamera;
	class JTexture;
	namespace Editor
	{
		class JEditorCameraControl;
		class JSceneCameraList;
		class JSceneViewer final : public JEditorWindow
		{
		private:
			std::unique_ptr<JSceneCameraList> camList;
			//JUserPtr<JTexture> shiroBack;
		private: 
			JUserPtr<JScene> scene;
			JUserPtr<JCamera> selectedCam;
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
			void Initialize(JUserPtr<JScene> newScene);
			void UpdateWindow()final;
		private:
			void UpdateMouseWheel()final;
		private:
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
		private:
			void LoadEditorWindow(JFileIOTool& tool)final;
			void StoreEditorWindow(JFileIOTool& tool)final;
		};
	}
}
