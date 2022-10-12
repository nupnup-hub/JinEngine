#pragma once
#include"../../JEditorWindow.h"
#include"../../../../Utility/JVector.h"

namespace JinEngine
{
	class JCamera;
	class JScene;
	class JGameObject;

	namespace Editor
	{
		class JEditorCameraControl;
		class JSceneObserver final : public JEditorWindow
		{ 
		private:
			Core::JUserPtr<JScene> scene;
			Core::JUserPtr<JGameObject> cameraObj;
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;
		private:
			std::wstring editorCameraName;
			JVector3<float> lastCamPos{ 0,0,0 };
			JVector3<float> lastCamRot{ 0,0,0 };
		public:
			JSceneObserver(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType);
			~JSceneObserver();
			JSceneObserver(const JSceneObserver& rhs) = delete;
			JSceneObserver& operator=(const JSceneObserver& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize(Core::JUserPtr<JScene> newScene, const std::wstring& editorCameraName)noexcept;
			void UpdateWindow()final;
		public:
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
			void StoreEditorWindow(std::wofstream& stream)final;
			void LoadEditorWindow(std::wifstream& stream)final;
		};
	}
}