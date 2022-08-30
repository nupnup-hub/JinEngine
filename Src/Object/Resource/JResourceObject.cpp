#include"JResourceObject.h"   
#include"JResourceManager.h"
#include"../Directory/JDirectory.h"
#include"../Directory/JFileFactory.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>
#include<io.h>

namespace JinEngine
{
	std::wstring JResourceObject::GetFullName()const noexcept
	{
		return GetName() + GetFormat();
	}
	std::wstring JResourceObject::GetPath()const noexcept
	{
		return directory->GetPath() + L"\\" + GetName() + GetFormat();
	}
	std::wstring JResourceObject::GetFolderPath()const noexcept
	{
		return directory->GetPath();
	}
	J_OBJECT_TYPE JResourceObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::RESOURCE_OBJECT;
	}
	uint8 JResourceObject::GetFormatIndex()const noexcept
	{
		return formatIndex;
	}
	std::wstring JResourceObject::GetMetafilePath()const noexcept
	{
		return ConvertMetafilePath(GetPath());
	}
	JDirectory* JResourceObject::GetDirectory()noexcept
	{
		return directory;
	}
	bool JResourceObject::HasMetafile(const std::wstring& path)
	{
		if (_access((JCommonUtility::WstrToU8Str(path) + ".meta").c_str(), 00) == -1)
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

		JResourceManager::Instance().ResourceStorageInterface()->RemoveResource(*this);
	}
	void JResourceObject::CopyRFile(JResourceObject& from, JResourceObject& to)
	{
		from.CallStoreResource();
		std::ifstream source(from.GetPath(), std::ios::binary);
		std::ofstream dest(to.GetPath(), std::ios::binary);

		dest << source.rdbuf();
		source.close();
		dest.close();
	}
	JResourceObject::JResourceObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JResourceObjectInterface(name, guid, flag), directory(directory), formatIndex(formatIndex)
	{
		JFFI::Create(*directory, *this);
		if (!HasMetafile(this))
			CallStoreResource();
	}
	JResourceObject::~JResourceObject()
	{ 
		JFFI::Destroy(*directory, *this);
	}
}