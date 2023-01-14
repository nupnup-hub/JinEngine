#include"JResourceObject.h"   
#include"JResourceManager.h"
#include"../Directory/JDirectory.h"
#include"../Directory/JFileFactory.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/File/JFileConstant.h"
#include<fstream>
#include<io.h>

namespace JinEngine
{
	JResourceObject::JResourceInitData::JResourceInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex)
		:name(directory->MakeUniqueFileName(name)), guid(guid), flag(flag), directory(directory), formatIndex(formatIndex)
	{ }
	JResourceObject::JResourceInitData::JResourceInitData(const std::wstring& name,
		JDirectory* directory,
		const uint8 formatIndex)
		:name(directory->MakeUniqueFileName(name)), guid(Core::MakeGuid()), flag(OBJECT_FLAG_NONE), directory(directory), formatIndex(formatIndex)
	{}
	JDirectory* JResourceObject::JResourceInitData::GetDirectory()const noexcept
	{
		return directory;
	}
	bool JResourceObject::JResourceInitData::IsValidCreateData()
	{
		return formatIndex != JResourceObject::GetInvalidFormatIndex();
	}
	bool JResourceObject::JResourceInitData::IsValidLoadData()
	{
		return formatIndex != JResourceObject::GetInvalidFormatIndex();
	}
	std::wstring JResourceObject::GetFullName()const noexcept
	{
		return GetName() + GetFormat();
	}
	std::wstring JResourceObject::GetPath()const noexcept
	{
		return directory->GetPath() + L"\\" + GetName() + Core::JFileConstant::GetFileFormat();
	}
	std::wstring JResourceObject::GetMetafilePath()const noexcept
	{
		return directory->GetPath() + L"\\" + GetName() + Core::JFileConstant::GetMetafileFormat();
	}
	std::wstring JResourceObject::GetFolderPath()const noexcept
	{
		return directory->GetPath();
	}
	J_OBJECT_TYPE JResourceObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::RESOURCE_OBJECT;
	}
	void JResourceObject::SetName(const std::wstring& newName)noexcept
	{
		const std::wstring preMetaPath = GetMetafilePath();
		const std::wstring prePath = GetPath();

		JObject::SetName(directory->MakeUniqueFileName(newName));

		const std::wstring newMetaPath = GetMetafilePath();
		const std::wstring newPath = GetPath();

		_wrename(preMetaPath.c_str(), newMetaPath.c_str());
		_wrename(prePath.c_str(), newPath.c_str());
	}
	uint8 JResourceObject::GetFormatIndex()const noexcept
	{
		return formatIndex;
	}
	JDirectory* JResourceObject::GetDirectory()noexcept
	{
		return directory;
	}
	bool JResourceObject::HasFile()const noexcept
	{
		if (_waccess(GetPath().c_str(), 00) == -1)
			return true;
		else
			return false;
	}
	bool JResourceObject::HasMetafile()const noexcept
	{
		if (_waccess(GetMetafilePath().c_str(), 00) == -1)
			return true;
		else
			return false; 
	}
	bool JResourceObject::CopyRFile(JResourceObject& ori, bool setNewInnderGuid)
	{
		if (GetTypeInfo().IsA(ori.GetTypeInfo()) && GetGuid() != ori.GetGuid())
		{
			//this->CallStoreResource();
			if (setNewInnderGuid)
			{
				std::unordered_map<size_t, size_t> guidMap;
				std::wifstream fromStream;
				fromStream.open(ori.GetPath(), std::ios::binary | std::ios::in);
				std::wofstream toStream;
				toStream.open(GetPath(), std::ios::binary | std::ios::out);

				std::wstring guide;
				const std::wstring guidSymbol =Core::JFileConstant::StreamHasObjGuidSymbol();
				while (getline(fromStream, guide))
				{
					if (JCUtil::Contain(guide, guidSymbol))
					{
						const size_t storedGuid = JCUtil::WstringToInt(guide.substr(guidSymbol.size()));
						auto guidData = guidMap.find(storedGuid);

						if(guidData != guidMap.end())
							toStream << guidSymbol << guidData->second << '\n';
						else
						{
							const size_t newGuid = Core::MakeGuid();
							guidMap.emplace(storedGuid, newGuid);
							toStream << guidSymbol << newGuid << '\n';
						}				 
					}
					else
						toStream << guide << '\n';
				}
				fromStream.close();
				toStream.close();
			}
			else
			{
				std::wifstream fromStream(ori.GetPath(), std::ios::binary | std::ios::in);
				std::wofstream toStream(GetPath(), std::ios::binary | std::ios::out);
				toStream << fromStream.rdbuf();
				fromStream.close();
				toStream.close();
			}
			return true;
		}
		else
			return false;
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
	bool JResourceObject::Destroy(const bool isForced)
	{ 
		if (HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !isForced)
			return false;
		 
		JResourceManager::Instance().ResourceStorageInterface()->RemoveResource(*this);
		return true;
	}
	void JResourceObject::DeleteRFile()
	{ 
		_wremove(GetPath().c_str());
		_wremove(GetMetafilePath().c_str()); 
	}
	bool JResourceObject::RegisterCashData()noexcept
	{
		return JResourceManager::Instance().ResourceStorageInterface()->AddResource(*this);
	}
	bool JResourceObject::DeRegisterCashData()noexcept
	{
		return JResourceManager::Instance().ResourceStorageInterface()->RemoveResource(*this);
	}
	Core::J_FILE_IO_RESULT JResourceObject::StoreMetadata(std::wofstream& stream, JResourceObject* rObject)
	{
		if (((int)rObject->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (stream.is_open())
		{
			if (JObject::StoreMetadata(stream, rObject) == Core::J_FILE_IO_RESULT::SUCCESS)
			{
				stream << Core::JFileConstant::StreamTypeSymbol<J_RESOURCE_TYPE>() << '\n';
				stream << (int)rObject->GetResourceType() << '\n';
				stream << rObject->GetFormat() << '\n';
				stream << rObject->GetFormatIndex() << '\n';
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT JResourceObject::LoadMetadata(std::wifstream& stream, JResourceMetaData& metadata)
	{
		if (stream.is_open())
		{
			if (JObject::LoadMetadata(stream, metadata) == Core::J_FILE_IO_RESULT::SUCCESS)
			{
				std::wstring guide;
				std::wstring format;
				int rType;
				int formatIndex;
				stream >> guide;
				stream >> rType;
				stream >> format;
				stream >> formatIndex;
				metadata.rType = rType;
				metadata.format = format;
				metadata.formatIndex = formatIndex;
				return Core::J_FILE_IO_RESULT::SUCCESS;
			}
			else
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JResourceObject::JResourceObject(const JResourceInitData& initdata)
		: JResourceObjectInterface(initdata.name, initdata.guid, initdata.flag), directory(initdata.GetDirectory()), formatIndex(initdata.formatIndex)
	{ 
		JFFI::Create(*directory, *this);
	}
	JResourceObject::~JResourceObject()
	{ 
		JFFI::Destroy(*directory, *this);
	}
}