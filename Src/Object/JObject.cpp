#include"JObject.h"
#include"../Utility/JCommonUtility.h"
#include"../Core/File/JFileIOHelper.h"
#include<fstream>
#include<io.h>

namespace JinEngine
{
	struct JObjectDestroyData
	{
	public:
		size_t beginGuid;
		bool isIgnoreUndestroyAbleFlag = false;
		bool isEnd = true;
	};
	static JObjectDestroyData destroyData;

	J_OBJECT_FLAG JObject::GetFlag()const noexcept
	{
		return flag;
	}
	bool JObject::HasFlag(const J_OBJECT_FLAG flag)const noexcept
	{
		return (JObject::flag & flag) != 0;
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
	bool JObject::BeginDestroy()
	{
		if (destroyData.isEnd)
		{
			//lock 필요 
			destroyData.beginGuid = GetGuid();
			destroyData.isEnd = false;
		}
		return EndDestroy();
	}
	bool JObject::BegineForcedDestroy()
	{
		SetIgnoreUndestroyableFlag(true);
		return BeginDestroy();
	}
	bool JObject::IsIgnoreUndestroyableFlag()noexcept
	{
		return destroyData.isIgnoreUndestroyAbleFlag;
	}
	void JObject::SetIgnoreUndestroyableFlag(const bool value)noexcept
	{
		destroyData.isIgnoreUndestroyAbleFlag = value;
	}
	bool JObject::EndDestroy()
	{
		const size_t guid = GetGuid();
		bool res = Destroy();
		if (destroyData.beginGuid == guid)
		{
			//unlock 필요 
			destroyData.isEnd = true;
			destroyData.isIgnoreUndestroyAbleFlag = false;
		}

		if (res)
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

		if (stream.is_open())
		{ 
			JFileIOHelper::StoreObjectIden(stream, object); 
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
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