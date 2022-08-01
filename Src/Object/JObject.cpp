#include"JObject.h"
#include"../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{ 
	JObject::JObject(const std::string& name, const size_t guid, const JOBJECT_FLAG flag)
		:name(name), guid(guid), flag(flag)
	{}
	JObject::~JObject(){}
	std::string JObject::GetName()const noexcept
	{   
		return name;
	}
	std::wstring JObject::GetWName() const noexcept
	{
		return JCommonUtility::U8StringToWstring(name);
	}
	size_t JObject::GetGuid()const noexcept
	{
		return guid;
	}
	JOBJECT_FLAG JObject::GetFlag()const noexcept
	{
		return flag;
	}
	void JObject::SetName(const std::string& name)noexcept
	{
		JObject::name = name;
	}
	bool JObject::IsActivated()const noexcept
	{
		return isActivated;
	}
	void JObject::Activate() noexcept
	{
		if (!isActivated)
			DoActivate();
	}
	void JObject::DeActivate()noexcept
	{
		if (isActivated)
			DoDeActivate();
	}
	void JObject::DoActivate() noexcept
	{
		isActivated = true;
	}
	void JObject::DoDeActivate()noexcept
	{
		isActivated = false;
	}
	Core::J_FILE_IO_RESULT JObject::StoreMetadata(std::wofstream& stream, JObject* object)
	{
		if (((int)object->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (stream.is_open())
		{
			stream << object->GetGuid() << '\n';
			stream << object->GetFlag() << '\n';
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT JObject::LoadMetadata(std::wifstream& stream, ObjectMetadata& metadata)
	{
		if (stream.is_open())
		{
			size_t resourceGuid;
			int flag;

			stream >> resourceGuid;
			stream >> flag;
			metadata.guid = resourceGuid;
			metadata.flag = (JOBJECT_FLAG)flag;
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
}