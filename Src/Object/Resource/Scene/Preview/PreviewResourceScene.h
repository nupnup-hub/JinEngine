#pragma once
#include"PreviewScene.h" 
#include<vector>
#include<memory>

namespace JinEngine
{
	class JResourceObject; 
	class PreviewResourceScene : public PreviewScene
	{
	private: 
		JScene* scene; 
		JMaterial* textureMaterial;
	public:
		PreviewResourceScene(const std::string& previewSceneName, _In_ JResourceObject* resource, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag);
		~PreviewResourceScene();
		PreviewResourceScene(PreviewResourceScene&& rhs) = default;
		PreviewResourceScene& operator=(PreviewResourceScene&& rhs) = default;

		bool Initialze()noexcept; 
		void Clear()noexcept final; 
	protected:
		JScene* GetScene()noexcept final; ;
	private:
		bool MakeMeshPreviewScene()noexcept;
		bool MakeMaterialPreviewScene()noexcept;
		bool MakeUserTexturePreviewScene()noexcept;
		bool MakeEditorTexturePreviewScene(const J_EDITOR_TEXTURE editorTextureType)noexcept;
	};
}