#pragma once
#include"JPreviewScene.h"  

namespace JinEngine
{
	class JResourceObject; 
	class JPreviewResourceScene : public JPreviewScene
	{
	public: 
		JPreviewResourceScene(_In_ JUserPtr<JResourceObject> resource, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewResourceScene();
		JPreviewResourceScene(JPreviewResourceScene&& rhs) = default;
		JPreviewResourceScene& operator=(JPreviewResourceScene && rhs) = default;
	public:
		bool Initialze()noexcept; 
	private:
		bool MakeMeshPreviewScene()noexcept;
		bool MakeMaterialPreviewScene()noexcept;
		bool MakeUserTexturePreviewScene()noexcept;
		bool MakeEditorTexturePreviewScene(const J_DEFAULT_TEXTURE editorTextureType)noexcept;
	};
}