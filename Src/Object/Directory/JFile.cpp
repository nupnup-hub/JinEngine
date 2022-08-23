#include"JFile.h" 
#include"../Resource/JResourceObject.h"

namespace JinEngine
{
	std::string JFile::GetName()const noexcept
	{
		return resource->GetName();
	}
	std::string JFile::GetFullName()const noexcept
	{
		return resource->GetFullName();
	}
	std::string JFile::GetAvailableFormat()const noexcept
	{
		return resource->GetFormat();
	}
	JResourceObject* JFile::GetResource()noexcept
	{
		return resource;
	}
	JFile::JFile(JResourceObject* resource)
		:resource(resource) {}
}