#pragma once 
#include"SceneEditor.h"

namespace JinEngine
{
	class PreviewScene;
	class PreviewSceneEditor : public SceneEditor
	{
	private:
		PreviewScene* previewScene;
		uint cameraIndex;
	public:
		PreviewSceneEditor(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~PreviewSceneEditor();
		PreviewSceneEditor(const PreviewSceneEditor& rhs) = delete;
		PreviewSceneEditor& operator=(const PreviewSceneEditor& rhs) = delete;

		void Initialize(PreviewScene* previewScene, uint cameraIndex)noexcept;
		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final;
		void UpdateWindow(EditorUtility* editorUtility)override;
	};
}