#include"JFile.h"  
#include"JFilePrivate.h"
#include"JFileInitData.h"
#include"JDirectory.h"
#include"../Resource/JResourceObject.h"
#include"../Resource/JResourceObjectHint.h"
#include"../Resource/JResourceObjectPrivate.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Application/JApplicationProject.h"

namespace JinEngine
{
	struct JFileActData : public JFileData
	{
	public:
		JResourceObject* resource;
	public:
		JFileActData(JResourceObject* resource)
			: resource(resource)
		{}
	public:
		bool IsActData()const noexcept
		{
			return true;
		}
	public:
		size_t GetResourceGuid()const noexcept
		{
			return resource->GetGuid();
		}
		J_RESOURCE_TYPE GetResourceType()const noexcept
		{
			return resource->GetResourceType();
		}
		Core::JTypeInfo& GetResourceTypeInfo()const noexcept
		{
			return resource->GetTypeInfo();
		}
		std::wstring GetName()const noexcept
		{
			return resource->GetName();
		}
		std::wstring GetFullName()const noexcept
		{
			return resource->GetName() + GetOriginalResourceFormat();
		}
		std::wstring GetOriginalResourceFormat()const noexcept
		{
			return resource->GetFormat();
		}
		std::wstring GetPath()const noexcept
		{
			return resource->GetPath();
		}
		std::wstring GetMetaFilePath()const noexcept
		{
			return resource->GetMetaFilePath();
		}
		std::wstring GetCacheFilePath()const noexcept
		{
			return JApplicationProject::ModResourceCachePath() + L"\\" + std::to_wstring(resource->GetGuid()) + Core::JFileConstant::GetCacheFileFormat();
		}
		JDirectory* GetOwnerDirectory()const noexcept 
		{
			return resource->GetDirectory();
		}
		JResourceObject* GetResource()const noexcept
		{
			return resource;
		} 
	};
	struct JFileDeActData : public JFileData
	{
	public:
		const size_t rGuid;
		Core::JTypeInfo& rTypeInfo;
		JDirectory* ownerDir;
		const J_RESOURCE_TYPE resourceType;
		const uint8 formatIndex;
	public:
		const std::wstring name;
	public:
		JFileDeActData(const JFileInitData& initData, JDirectory* ownerDir)
			:JFileData(), 
			rGuid(initData.rGuid),
			rTypeInfo(initData.rTypeInfo),
			ownerDir(ownerDir),
			resourceType(initData.resourceType),
			formatIndex(initData.formatIndex),
			name(initData.name)
		{}
	public:
		bool IsActData()const noexcept
		{
			return false;
			//return rTypeInfo.GetInstanceUserPtr<JResourceObject>(rGuid) != nullptr;	is Existing resource 함수 과거 body
		}
	public:
		size_t GetResourceGuid()const noexcept
		{
			return rGuid;
		}
		J_RESOURCE_TYPE GetResourceType()const noexcept
		{
			return resourceType;
		}
		Core::JTypeInfo& GetResourceTypeInfo()const noexcept
		{
			return rTypeInfo;
		}
		std::wstring GetName()const noexcept
		{
			return name;
		}
		std::wstring GetFullName()const noexcept
		{
			return name + GetOriginalResourceFormat();
		}
		std::wstring GetOriginalResourceFormat()const noexcept
		{ 
			return RTypeCommonCall::GetFormat(resourceType, formatIndex);
		}
		std::wstring GetPath()const noexcept
		{
			return ownerDir->GetPath() + L"\\" + name + Core::JFileConstant::GetFileFormat();
		}
		std::wstring GetMetaFilePath()const noexcept
		{
			return ownerDir->GetPath() + L"\\" + name + Core::JFileConstant::GetMetaFileFormat();
		}
		std::wstring GetCacheFilePath()const noexcept
		{
			return JApplicationProject::ModResourceCachePath() + L"\\" + std::to_wstring(rGuid) + Core::JFileConstant::GetCacheFileFormat();
		}
		JDirectory* GetOwnerDirectory()const noexcept
		{
			return ownerDir;
		}
		JResourceObject* GetResource()const noexcept
		{
			return static_cast<JResourceObject*>(rTypeInfo.GetInstanceRawPtr(rGuid));
		}
	};

	bool JFile::IsExistingResource()const noexcept
	{ 
		return fileData->IsActData();
	}
	size_t JFile::GetResourceGuid()const noexcept
	{
		return fileData->GetResourceGuid();
	}
	J_RESOURCE_TYPE JFile::GetResourceType()const noexcept
	{
		return fileData->GetResourceType();
	}
	Core::JTypeInfo& JFile::GetResourceTypeInfo()const noexcept
	{
		return fileData->GetResourceTypeInfo();
	}
	std::wstring JFile::GetName()const noexcept
	{
		return fileData->GetName();
	}
	std::wstring JFile::GetFullName()const noexcept
	{
		return fileData->GetFullName();
	}
	std::wstring JFile::GetOriginalResourceFormat()const noexcept
	{
		return fileData->GetOriginalResourceFormat();
	}
	std::wstring JFile::GetPath()const noexcept
	{
		return fileData->GetPath();
	}
	std::wstring JFile::GetMetaFilePath()const noexcept
	{
		return fileData->GetMetaFilePath();
	}
	std::wstring JFile::GetCacheFilePath()const noexcept
	{
		return fileData->GetCacheFilePath();
	}
	JDirectory* JFile::GetOwnerDirectory()const noexcept
	{
		return fileData->GetOwnerDirectory();
	}
	JResourceObject* JFile::GetResource()const noexcept
	{
		return fileData->GetResource();
	}
	Core::JUserPtr<JResourceObject> JFile::TryGetResourceUser()const noexcept
	{
		//Act File
		if (fileData->IsActData())
			return Core::GetUserPtr(fileData->GetResource());
		else
		{	
			using IOInterface = JResourceObjectPrivate::AssetDataIOInterface;
			auto initData = IOInterface::CreateLoadAssetDIData(fileData->GetOwnerDirectory(), Core::JAssetFileLoadPathData{ GetPath() });
			auto rPrivate = Core::JIdentifier::GetPrivateInterface(fileData->GetResourceTypeInfo().TypeGuid());
			auto rawPtr = static_cast<JResourceObjectPrivate*>(rPrivate)->GetAssetDataIOInterface().LoadAssetData(initData.get());
			return Core::GetUserPtr<JResourceObject>(rawPtr);
		}	
	}
	JFile::JFile(const JFileInitData& initData, JDirectory* ownerDir)
		:fileData(std::make_unique<JFileDeActData>(initData, ownerDir))
	{}

	std::unique_ptr<JFile> JFilePrivate::CreateFile(const JFileInitData& initData, JDirectory* ownerDir)
	{
		if (ownerDir == nullptr)
			return 	std::unique_ptr<JFile>{};
		return std::unique_ptr<JFile>(new JFile(initData, ownerDir));
	}
	void JFilePrivate::ConvertToActFileData(JFile* file, JResourceObject* rObj)
	{
		if (rObj == nullptr || rObj->GetGuid() != file->GetResourceGuid())
			return;

		file->fileData = std::make_unique<JFileActData>(rObj);
	}
	void JFilePrivate::ConvertToDeActFileData(JFile* file)
	{
		auto rawPtr = file->fileData->GetResource();
		if (rawPtr == nullptr)
			assert("Can't convert deActfile");
		file->fileData = std::make_unique<JFileDeActData>(JFileInitData{rawPtr}, rawPtr->GetDirectory());
	} 
}