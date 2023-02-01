#pragma once
#include"JPreviewScene.h" 

namespace JinEngine
{
	class JDirectory;
	class JPreviewDirectory : public JPreviewScene
	{
	public:
		JPreviewDirectory(_In_ Core::JUserPtr<JDirectory> jDir, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag);
		~JPreviewDirectory();
		JPreviewDirectory(JPreviewDirectory&& rhs) = default;
		JPreviewDirectory& operator=(JPreviewDirectory&& rhs) = default;
	public:
		bool Initialze()noexcept;
	private:
		bool MakeJDirectoryPreviewScene()noexcept;
	};
}