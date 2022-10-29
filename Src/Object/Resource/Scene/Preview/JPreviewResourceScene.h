#pragma once
#include"JPreviewScene.h" 
#include<vector>
#include<memory>

namespace JinEngine
{
	class JResourceObject; 
	class JPreviewResourceScene : public JPreviewScene
	{
	private: 
		JScene* scene = nullptr;
		JMaterial* textureMaterial = nullptr;
	public: 
		JPreviewResourceScene(_In_ Core::JUserPtr<JResourceObject> resource, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewResourceScene();
		JPreviewResourceScene(JPreviewResourceScene&& rhs) = default;
		JPreviewResourceScene& operator=(JPreviewResourceScene && rhs) = default;
	public:
		bool Initialze()noexcept; 
		void Clear()noexcept final; 
	protected:
		JScene* GetScene()noexcept final;
	private:
		bool MakeMeshPreviewScene()noexcept;
		bool MakeMaterialPreviewScene()noexcept;
		bool MakeUserTexturePreviewScene()noexcept;
		bool MakeEditorTexturePreviewScene(const J_EDITOR_TEXTURE editorTextureType)noexcept;
	};
}