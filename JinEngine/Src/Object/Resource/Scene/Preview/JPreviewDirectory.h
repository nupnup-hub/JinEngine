#pragma once
#include"JPreviewScene.h" 

namespace JinEngine
{
	class JDirectory;
	class JPreviewDirectory : public JPreviewScene
	{
	private:
		JScene* scene;
		JMaterial* textureMaterial;
	public:
		JPreviewDirectory(_In_ Core::JUserPtr<JDirectory> jDir, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewDirectory();
		JPreviewDirectory(JPreviewDirectory&& rhs) = default;
		JPreviewDirectory& operator=(JPreviewDirectory&& rhs) = default;
	public:
		bool Initialze()noexcept;
		void Clear()noexcept final;
	protected:
		JScene* GetScene()noexcept final; ;
	private:
		bool MakeJDirectoryPreviewScene()noexcept;
	};
}