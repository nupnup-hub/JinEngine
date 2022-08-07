#include"JFile.h"
#include"../Resource/JResourceObject.h"

namespace JinEngine
{
	JFile::JFile(JResourceObject* resource)
		:resource(resource) {}
	JFile::~JFile()
	{
		resource = nullptr;
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
}