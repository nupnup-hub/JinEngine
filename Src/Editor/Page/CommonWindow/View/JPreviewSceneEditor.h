#pragma once 
#include"JSceneEditor.h"

namespace JinEngine
{
	class JPreviewScene;
	namespace Editor
	{
		class JPreviewSceneEditor : public JSceneEditor
		{
		private:
			JPreviewScene* previewScene;
			uint cameraIndex;
		public:
			JPreviewSceneEditor(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JPreviewSceneEditor();
			JPreviewSceneEditor(const JPreviewSceneEditor& rhs) = delete;
			JPreviewSceneEditor& operator=(const JPreviewSceneEditor& rhs) = delete;

			void Initialize(JPreviewScene* previewScene, uint cameraIndex)noexcept;
			bool Activate(JEditorUtility* editorUtility) final;
			bool DeActivate(JEditorUtility* editorUtility) final;
			void UpdateWindow(JEditorUtility* editorUtility)override;
		};
	}
}