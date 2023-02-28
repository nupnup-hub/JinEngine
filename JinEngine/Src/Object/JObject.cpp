#include"JObject.h"
#include"../Utility/JCommonUtility.h"
#include"../Core/File/JFileIOHelper.h"
#include<fstream>
#include<io.h>

namespace JinEngine
{
	J_OBJECT_FLAG JObject::GetFlag()const noexcept
	{
		return flag;
	}
	bool JObject::HasFlag(const J_OBJECT_FLAG flag)const noexcept
	{	 
		return Core::HasSQValueEnum(GetFlag(), flag);
	}
	bool JObject::Copy(JObject* ori)
	{
		//engine private object has OBJECT_FLAG_COPYABLE flag
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;

		if (GetTypeInfo().IsA(ori->GetTypeInfo()))
		{
			DoCopy(ori);
			return true;
		}
		else
			return false;
	}
	bool JObject::BeginDestroy(JObject* obj)
	{
		return obj->EndDestroy(false);
	}
	bool JObject::BegineForcedDestroy(JObject* obj)
	{  
		return obj->EndDestroy(true);
	}
	bool JObject::EndDestroy(const bool isForced)
	{
		if (Destroy(isForced))
		{
			RemoveInstance();
			return true;
		}
		else
			return false;
	}
	Core::J_FILE_IO_RESULT JObject::StoreMetadata(std::wofstream& stream, JObject* object)
	{
		if (((int)object->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	
		JFileIOHelper::StoreObjectIden(stream, object);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObject::LoadMetadata(std::wifstream& stream, JObjectMetaData& metadata)
	{
		if (stream.is_open())
		{
			JFileIOHelper::LoadObjectIden(stream, metadata.guid, metadata.flag);
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JObject::JObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
		:JIdentifier(name, guid), flag(flag)
	{}
	JObject::~JObject()
	{}
}