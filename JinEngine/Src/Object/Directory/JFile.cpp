#include"JFile.h" 
#include"../Resource/JResourceObject.h"

namespace JinEngine
{
	std::wstring JFile::GetName()const noexcept
	{
		return resource->GetName();
	}
	std::wstring JFile::GetFullName()const noexcept
	{
		return resource->GetFullName();
	}
	std::wstring JFile::GetFormat()const noexcept
	{ 
		return resource->GetFormat();
	}
	JResourceObject* JFile::GetResource()noexcept
	{
		return resource;
	}
	JFile::JFile(JResourceObject* resource)
		:resource(resource) 
	{

	}
}