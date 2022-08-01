#include"JResourceObject.h"   
#include"JResourceManager.h"
#include"../Directory/JDirectory.h"
#include"../Directory/JFileFactory.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>
#include<io.h>

namespace JinEngine
{
	JResourceObject::JResourceObject(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JResourceObjectInterface(name, guid, flag), directory(directory), formatIndex(formatIndex)
	{
		JFileFactory::Create(*this); 
	}
	JResourceObject::~JResourceObject()
	{
		JFileFactory::Erase(*this);
	}
	std::string JResourceObject::GetPath()const noexcept
	{
		return directory->GetPath() + "\\" + GetName() + GetFormat();
	}
	std::wstring JResourceObject::GetWPath()const noexcept
	{
		return JCommonUtility::U8StringToWstring(GetPath());
	}
	std::string JResourceObject::GetFolderPath(bool eraseLastBackSlash)const noexcept
	{ 
		return directory->GetPath();
	}
	int JResourceObject::GetReferenceCount()const noexcept
	{
		return referenceCount;
	}
	J_OBJECT_TYPE JResourceObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::RESOURCE_OBJECT;
	}
	void JResourceObject::OnReference()noexcept
	{
		++referenceCount;
		if(!IsActivated())
			JObject::Activate();
	}
	void JResourceObject::OffReference()noexcept
	{
		--referenceCount;
		if (referenceCount < 0)
			referenceCount = 0;

		if (referenceCount == 0)
			JObject::DeActivate();
	}
	void JResourceObject::DoActivate() noexcept
	{ 
		JObject::DoActivate();
	}
	void JResourceObject::DoDeActivate()noexcept
	{  
		referenceCount = 0;
		JObject::DoDeActivate();
	}
	std::string JResourceObject::GetMetafilePath()const noexcept
	{
		return GetPath() + ".meta";
	}
	JDirectory* JResourceObject::GetDirectory()noexcept
	{
		return directory;
	}
	JResourceObject* JResourceObject::CopyResource()
	{
		//수정필요
		return nullptr;
	}
	std::wstring JResourceObject::ConvertMetafilePath(const std::wstring& resourcePath)noexcept
	{
		return resourcePath + L".meta";
	} 
	bool JResourceObject::HasMetafile(const std::string& path)
	{
		if (_access((path + ".meta").c_str(), 00) == -1)
			return true;
		else
			return false;
	}
	bool JResourceObject::HasMetafile(JResourceObject* resource)
	{
		return HasMetafile(resource->GetPath());
	}
}