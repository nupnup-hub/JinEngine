#include"JFile.h"
#include"../Resource/JResourceObject.h"

namespace JinEngine
{
	JFile::JFile(JResourceObject* resource)
		:resource(resource), previewScene(nullptr) {}
	JFile::~JFile()
	{
		resource = nullptr;
		previewScene = nullptr;
	}

	std::string JFile::GetName()const noexcept
	{
		return resource->GetName();
	}
	std::string JFile::GetAvailableFormat()const noexcept
	{
		return resource->GetFormat();
	}
	JResourceObject* JFile::GetResource()noexcept
	{
		return resource;
	}
	PreviewScene* JFile::GetPreviewScene()noexcept
	{
		return previewScene;
	}
	void JFile::SetPreviewScene(PreviewScene* previewScene)noexcept
	{
		JFile::previewScene = previewScene;
	}
}