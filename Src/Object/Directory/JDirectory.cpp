#include"JDirectory.h"
#include"JFile.h" 
#include"JDirectoryFactory.h"
#include"../Resource/JResourceObject.h"
#include"../../Application/JApplicationVariable.h"
#include"../../Core/Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JDirectory* JDirectory::GetChildDirctory(const uint index)noexcept
	{
		if (index >= (uint)childrenDir.size())
			return nullptr;
		else
			return childrenDir[index];
	}
	JDirectory* JDirectory::GetChildDirctory(const std::string& path)noexcept
	{
		std::string thisPath = GetPath();
		const uint childCount = (uint)childrenDir.size();
		for (uint i = 0; i < childCount; ++i)
		{
			if (path == thisPath + "\\" + childrenDir[i]->GetName())
				return childrenDir[i];
		}
		return nullptr;
	}
	uint JDirectory::GetChildernDirctoryCount()const noexcept
	{
		return (uint)childrenDir.size();
	}
	uint JDirectory::GetFileCount()const noexcept
	{
		return (uint)fileList.size();
	}
	std::string JDirectory::GetPath()const noexcept
	{
		if (parentDir == nullptr)
			return JApplicationVariable::GetProjectPath();
		else
			return parentDir->GetPath() + "\\" + GetName();
	}
	std::wstring JDirectory::GetWPath()const noexcept
	{
		return JCommonUtility::U8StringToWstring(GetPath());
	}
	JFile* JDirectory::GetFile(const uint index)noexcept
	{
		if (index >= fileList.size())
			return nullptr;
		else
			return fileList[index].get();
	}
	J_OBJECT_TYPE JDirectory::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::DIRECTORY_OBJECT;
	}
	bool JDirectory::HasChild(const std::string& name)const noexcept
	{
		const uint childrenCount = (uint)childrenDir.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			if (childrenDir[i]->GetName() == name)
				return true;
		}
		return false;
	}
	bool JDirectory::IsOpen()const noexcept
	{
		return IsActivated();
	}
	JDirectory* JDirectory::SearchDirectory(const std::string& path)noexcept
	{
		JDirectory* res = GetChildDirctory(path);
		if (res == nullptr)
		{
			const uint childCount = (uint)childrenDir.size();
			for (uint i = 0; i < childCount; ++i)
			{
				res = childrenDir[i]->SearchDirectory(path);
				if (res != nullptr)
					break;
			}
		}	
		return res;
	}
	std::string JDirectory::MakeUniqueFileName(const std::string& name)noexcept
	{
		return JCommonUtility::MakeUniqueName(fileList, name);
	}
	JDirectoryInterface* JDirectory::DirectoryInterface()
	{
		return this;
	}
	bool JDirectory::AddFile(JResourceObject* resource)noexcept
	{
		if (resource == nullptr)
			return false;

		fileList.emplace_back(std::make_unique<JFile>(resource));
		return true;
	}
	bool JDirectory::EraseFile(JResourceObject* resource)noexcept
	{
		if (resource == nullptr)
			return false;

		const size_t guid = resource->GetGuid();
		const uint fileCount = (uint)fileList.size();
		for (uint i = 0; i < fileCount; ++i)
		{
			if (fileList[i]->GetResource()->GetGuid() == guid)
			{ 
				fileList.erase(fileList.begin() + i);
				return true;
			}
		}
		return false;
	}
	void JDirectory::OpenDirectory()
	{
		Activate(); 
	}
	void JDirectory::CloseDirectory()
	{
		DeActivate();
	}
	void JDirectory::RegisterFunc()
	{
		auto defaultC = [](JDirectory* parent)
		{
			std::string name = GetDefaultName<JDirectory>();
			if (parent != nullptr)
				name = JCommonUtility::MakeUniqueName(parent->childrenDir, name);
			return new JDirectory(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
		};
		auto initC = [](const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* parent)
		{
			std::string newName = name;
			if (parent != nullptr)
				newName = JCommonUtility::MakeUniqueName(parent->childrenDir, newName);
			return new JDirectory(newName, guid, objFlag, parent);
		};
		JDFI::Regist(defaultC, initC);
	}
	JDirectory::JDirectory(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* parentDir)
		:JDirectoryInterface(name, guid, flag), parentDir(parentDir)
	{
		if (parentDir != nullptr)
			parentDir->childrenDir.push_back(this);
	}
	JDirectory::~JDirectory() {}
}