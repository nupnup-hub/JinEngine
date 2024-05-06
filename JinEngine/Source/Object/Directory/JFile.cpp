/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JFile.h"  
#include"JFilePrivate.h"
#include"JFileInitData.h"
#include"JDirectory.h"
#include"../Resource/JResourceObject.h"
#include"../Resource/JResourceObjectHint.h"
#include"../Resource/JResourceObjectPrivate.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Application/Project/JApplicationProject.h"

namespace JinEngine
{
	struct JFileActData : public JFileData
	{
		REGISTER_CLASS_USE_ALLOCATOR(JFileActData)
	public:
		JWeakPtr<JResourceObject> resource;
	public:
		JFileActData(const JWeakPtr<JResourceObject>& resource)
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
		J_OBJECT_FLAG GetObjectFlag()const noexcept
		{
			return resource->GetFlag();
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
			return JApplicationProject::ModResourceCachePath() + L"\\" + std::to_wstring(resource->GetGuid()) + Core::JFileConstant::GetCacheFileFormatW();
		}
		JUserPtr<JDirectory> GetOwnerDirectory()const noexcept
		{
			return resource->GetDirectory();
		}
		JUserPtr<JResourceObject> GetResource()const noexcept
		{
			return resource;
		}
	};
	struct JFileDeActData : public JFileData
	{
		REGISTER_CLASS_USE_ALLOCATOR(JFileDeActData)
	public:
		const size_t rGuid;
		Core::JTypeInfo& rTypeInfo;
		JWeakPtr<JDirectory> ownerDir;
		const J_RESOURCE_TYPE resourceType;
		const J_OBJECT_FLAG flag;
		const uint8 formatIndex;
	public:
		const std::wstring name;
	public:
		JFileDeActData(const JFileInitData& initData, const JWeakPtr<JDirectory>& ownerDir)
			:JFileData(),
			rGuid(initData.rGuid),
			rTypeInfo(initData.rTypeInfo),
			ownerDir(ownerDir),
			resourceType(initData.resourceType),
			flag(initData.flag),
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
		J_OBJECT_FLAG GetObjectFlag()const noexcept
		{
			return flag;
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
			return ownerDir->GetPath() + L"\\" + name + Core::JFileConstant::GetFileFormatW();
		}
		std::wstring GetMetaFilePath()const noexcept
		{
			return ownerDir->GetPath() + L"\\" + name + Core::JFileConstant::GetMetaFileFormatW();
		}
		std::wstring GetCacheFilePath()const noexcept
		{
			return JApplicationProject::ModResourceCachePath() + L"\\" + std::to_wstring(rGuid) + Core::JFileConstant::GetCacheFileFormatW();
		}
		JUserPtr<JDirectory> GetOwnerDirectory()const noexcept
		{
			return ownerDir;
		}
		JUserPtr<JResourceObject> GetResource()const noexcept
		{
			return rTypeInfo.GetInstanceUserPtr<JResourceObject>(rGuid);
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
	J_OBJECT_FLAG JFile::GetObjectFlag()const noexcept
	{
		return fileData->GetObjectFlag();
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
	JUserPtr<JDirectory> JFile::GetOwnerDirectory()const noexcept
	{
		return fileData->GetOwnerDirectory();
	}
	JUserPtr<JResourceObject> JFile::GetResource()const noexcept
	{
		return fileData->GetResource();
	}
	JUserPtr<JResourceObject> JFile::TryGetResourceUser()const noexcept
	{
		//Act File
		if (fileData->IsActData())
			return fileData->GetResource();
		else
		{
			using IOInterface = JResourceObjectPrivate::AssetDataIOInterface;
			auto initData = IOInterface::CreateLoadAssetDIData(fileData->GetOwnerDirectory(), Core::JAssetFilePathData{ GetPath() });
			auto rPrivate = Core::JIdentifier::PrivateInterface(fileData->GetResourceTypeInfo().TypeGuid());
			auto idenUser = static_cast<JResourceObjectPrivate*>(rPrivate)->GetAssetDataIOInterface().LoadAssetData(initData.get());
			JUserPtr<JResourceObject> rUser = JUserPtr<JResourceObject>::ConvertChild(std::move(idenUser));
			return rUser;
		}
	}
	void JFile::RegisterTypeData()
	{ 
		using ReceiverPtr = JinEngine::Core::JAllocationDesc::ReceiverPtr;
		using ReAllocatedPtr = JinEngine::Core::JAllocationDesc::ReAllocatedPtr;
		using MemIndex = JinEngine::Core::JAllocationDesc::MemIndex;

		JinEngine::Core::JAllocationDesc::NotifyReAllocF::Ptr notifyActFilePtr = [](ReceiverPtr receiver,
			ReAllocatedPtr movedPtr,
			MemIndex index)
		{
			JFileActData* movedAct = static_cast<JFileActData*>(movedPtr);
			auto filePtr = JFile::StaticTypeInfo().GetInstanceRawPtr<JFile>(movedAct->GetResourceGuid());
			filePtr->fileData.release();
			filePtr->fileData.reset(movedAct); 
		};
		JinEngine::Core::JAllocationDesc::NotifyReAllocF::Ptr notifyDeActFilePtr = [](ReceiverPtr receiver,
			ReAllocatedPtr movedPtr,
			MemIndex index)
		{
			JFileDeActData* movedDeAct = static_cast<JFileDeActData*>(movedPtr);
			auto filePtr = JFile::StaticTypeInfo().GetInstanceRawPtr<JFile>(movedDeAct->GetResourceGuid());
			filePtr->fileData.release();
			filePtr->fileData.reset(movedDeAct);
		};
		auto actReAllocF = std::make_unique<JinEngine::Core::JAllocationDesc::NotifyReAllocF::Functor>(notifyActFilePtr);
		auto deactReAllocF = std::make_unique<JinEngine::Core::JAllocationDesc::NotifyReAllocF::Functor>(notifyActFilePtr);

		std::unique_ptr<JinEngine::Core::JAllocationDesc> actDesc = std::make_unique<JinEngine::Core::JAllocationDesc>();
		std::unique_ptr<JinEngine::Core::JAllocationDesc> deactDesc = std::make_unique<JinEngine::Core::JAllocationDesc>();

		actDesc->notifyReAllocB = UniqueBind(std::move(actReAllocF), static_cast<ReceiverPtr>(&typeInfo), JinEngine::Core::empty, JinEngine::Core::empty);
		deactDesc->notifyReAllocB = UniqueBind(std::move(deactReAllocF), static_cast<ReceiverPtr>(&typeInfo), JinEngine::Core::empty, JinEngine::Core::empty);
		JFileActData::StaticTypeInfo().SetAllocationOption(std::move(actDesc));
		JFileDeActData::StaticTypeInfo().SetAllocationOption(std::move(deactDesc));
	}

	JFile::JFile(const JFileInitData& initData, const JUserPtr<JDirectory>& ownerDir)
		:JTypeBase(initData.rGuid),
		fileData(std::make_unique<JFileDeActData>(initData, ownerDir))
	{}

	JOwnerPtr<JFile> JFilePrivate::CreateFile(const JFileInitData& initData, const JUserPtr<JDirectory>& ownerDir)
	{
		if (ownerDir == nullptr)
			return nullptr;
		return Core::JPtrUtil::MakeOwnerPtr<JFile>(initData, ownerDir);
	}
	void JFilePrivate::ConvertToActFileData(const JUserPtr<JFile>& file, const JUserPtr<JResourceObject>& rObj)
	{
		if (rObj == nullptr || rObj->GetGuid() != file->GetResourceGuid())
			return;

		file->fileData = std::make_unique<JFileActData>(rObj);
	}
	void JFilePrivate::ConvertToDeActFileData(const JUserPtr<JFile>& file)
	{
		auto userPtr = file->fileData->GetResource();
		if (userPtr == nullptr)
			assert("Can't convert deActfile");
		file->fileData = std::make_unique<JFileDeActData>(JFileInitData{ userPtr }, userPtr->GetDirectory());
	}
}