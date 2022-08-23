#include"JResourceObject.h"   
#include"JResourceManager.h"
#include"../Directory/JDirectory.h"
#include"../Directory/JFileFactory.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>
#include<io.h>

namespace JinEngine
{
	std::string JResourceObject::GetFullName()const noexcept
	{
		return GetName() + GetFormat();
	}
	std::string JResourceObject::GetPath()const noexcept
	{
		return directory->GetPath() + "\\" + GetName() + GetFormat();
	}
	std::wstring JResourceObject::GetWPath()const noexcept
	{
		return JCommonUtility::U8StringToWstring(GetPath());
	}
	std::string JResourceObject::GetFolderPath()const noexcept
	{
		return directory->GetPath();
	}
	std::wstring JResourceObject::GetFolderWPath()const noexcept
	{
		return JCommonUtility::U8StringToWstring(GetFolderPath());
	}
	J_OBJECT_TYPE JResourceObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::RESOURCE_OBJECT;
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
	void JResourceObject::DoActivate() noexcept
	{ 
		JObject::DoActivate();
	}
	void JResourceObject::DoDeActivate()noexcept
	{   
		JObject::DoDeActivate();
	}
	void JResourceObject::OnReference()noexcept
	{
		JReferenceInterface::OnReference();
		if (!IsActivated())
			JObject::Activate();
	}
	void JResourceObject::OffReference()noexcept
	{
		JReferenceInterface::OffReference();
		if (GetReferenceCount() == 0)
			JObject::DeActivate();
	}
	void JResourceObject::Destroy()
	{
		if (HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE))
			return;

		JResourceManager::Instance().ResourceRemoveInterface()->RemoveResource(*this);
		delete this;
	}
	JResourceObject::JResourceObject(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JResourceObjectInterface(name, guid, flag), directory(directory), formatIndex(formatIndex)
	{
		JFFI::CreateJFile(*directory, *this);
	}
	JResourceObject::~JResourceObject()
	{ 
		JFFI::DestroyJFile(*directory, *this);
	}
}