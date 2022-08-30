#include"JDirectory.h"
#include"JDirectoryFactory.h"
#include"JFile.h" 
#include"JFileFactory.h"
#include"../Resource/JResourceObject.h"
#include"../Resource/JResourceManager.h"
#include"../Resource/JResourceObjectFactory.h"
#include"../../Application/JApplicationVariable.h" 
#include"../../Core/Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"
#include<io.h>

namespace JinEngine
{
	JDirectory* JDirectory::GetChildDirctory(const uint index)noexcept
	{
		if (index >= (uint)children.size())
			return nullptr;
		else
			return children[index];
	}
	JDirectory* JDirectory::GetChildDirctory(const std::wstring& path)noexcept
	{
		std::wstring thisPath = GetPath();
		const uint childCount = (uint)children.size();
		for (uint i = 0; i < childCount; ++i)
		{
			if (path == thisPath + L"\\" + children[i]->GetName())
				return children[i];
		}
		return nullptr;
	}
	uint JDirectory::GetChildernDirctoryCount()const noexcept
	{
		return (uint)children.size();
	}
	uint JDirectory::GetFileCount()const noexcept
	{
		return (uint)fileList.size();
	}
	std::wstring JDirectory::GetPath()const noexcept
	{
		if (parent == nullptr)
			return JApplicationVariable::GetProjectPath();
		else
			return parent->GetPath() + L"\\" + GetName();
	}
	JFile* JDirectory::GetFile(const uint index)noexcept
	{
		if (index >= fileList.size())
			return nullptr;
		else
			return fileList[index];
	}
	JFile* JDirectory::GetRecentFile()noexcept
	{
		return fileList[fileList.size() - 1];
	}
	J_OBJECT_TYPE JDirectory::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::DIRECTORY_OBJECT;
	}
	bool JDirectory::HasChild(const std::wstring& name)const noexcept
	{
		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			if (children[i]->GetName() == name)
				return true;
		}
		return false;
	}
	bool JDirectory::HasFile(const std::wstring& name)const noexcept
	{
		const uint fileCount = (uint)fileList.size();
		for (uint i = 0; i < fileCount; ++i)
		{
			if (fileList[i]->GetFullName() == name)
				return true;
		}
		return false;
	}
	bool JDirectory::IsOpen()const noexcept
	{
		return IsActivated();
	}
	JDirectory* JDirectory::SearchDirectory(const std::wstring& path)noexcept
	{
		JDirectory* res = GetChildDirctory(path);
		if (res == nullptr)
		{
			const uint childCount = (uint)children.size();
			for (uint i = 0; i < childCount; ++i)
			{
				res = children[i]->SearchDirectory(path);
				if (res != nullptr)
					break;
			}
		}
		return res;
	}
	std::wstring JDirectory::MakeUniqueFileName(const std::wstring& name)noexcept
	{
		return JCommonUtility::MakeUniqueName(fileList, name);
	}
	bool JDirectory::Copy(JObject* ori)
	{
		//engine private dir has OBJECT_FLAG_COPYABLE flag
		//and these dir can't copy
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;

		if (typeInfo.IsA(ori->GetTypeInfo()))
		{
			Clear();
			JDirectory* oriDir = static_cast<JDirectory*>(ori);
			const uint fileCount = (uint)oriDir->fileList.size();
			for (uint i = 0; i < fileCount; ++i)
				JRFIB::CopyByName(oriDir->fileList[i]->GetResource()->GetTypeInfo().Name(), *oriDir->fileList[i]->GetResource(), *this);			

			const uint childrenCount = (uint)oriDir->children.size();
			for (uint i = 0; i < childrenCount; ++i)
				JDFI::Copy(*oriDir->children[i], *this);

			return true;
		}
		else
			return false;

	}
	JDirectoryDestroyInterface* JDirectory::DestroyInterface()
	{
		return this;
	}
	JDirectoryOCInterface* JDirectory::OCInterface()
	{
		return this;
	}
	bool JDirectory::CreateJFile(JResourceObject& resource)noexcept
	{
		fileList.emplace_back(new JFile(&resource));
		return true;
	}
	bool JDirectory::DestroyJFile(JResourceObject& resource)noexcept
	{
		const size_t guid = resource.GetGuid();
		const uint fileCount = (uint)fileList.size();
		for (uint i = 0; i < fileCount; ++i)
		{
			if (fileList[i]->GetResource()->GetGuid() == guid)
			{
				delete fileList[i];
				fileList.erase(fileList.begin() + i);
				return true;
			}
		}
		return false;
	}
	void JDirectory::Destroy()
	{
		if (HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !IsIgnoreUndestroyableFlag())
			return;

		Clear();
		JResourceManager::Instance().DirectoryStorageInterface()->RemoveJDirectory(*this);
	}
	void JDirectory::Clear()
	{
		if (parent != nullptr)
		{
			const size_t guid = GetGuid();
			const uint pChildrenCount = (uint)parent->children.size();
			for (uint i = 0; i < pChildrenCount; ++i)
			{
				if (guid == parent->children[i]->GetGuid())
				{
					parent->children.erase(parent->children.begin() + i);
					parent = nullptr;
					break;
				}
			}
		}

		bool preIgnore = IsIgnoreUndestroyableFlag();
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(true);
		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			children[i]->BeginDestroy();
			children[i] = nullptr;
		}
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(false);
		 
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(true);
		const uint fileCount = (uint)fileList.size();
		for (uint i = 0; i < fileCount; ++i)
			fileList[i]->GetResource()->BeginDestroy();
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(false);

		fileList.clear();
		children.clear();
	}
	void JDirectory::DeleteResourceFile(JResourceObject& resource)
	{
		const size_t guid = resource.GetGuid();
		const uint fileCount = (uint)fileList.size();
		for (uint i = 0; i < fileCount; ++i)
		{
			if (fileList[i]->GetResource()->GetGuid() == guid)
			{ 
				std::string metaPath = JCommonUtility::WstrToU8Str(ConvertMetafilePath(resource.GetPath()));
				if (_access(metaPath.c_str(), 00) != -1)
					remove(metaPath.c_str());

				remove(JCommonUtility::WstrToU8Str(resource.GetPath()).c_str());
				SetIgnoreUndestroyableFlag(true);
				resource.BeginDestroy();
				delete fileList[i];
				fileList.erase(fileList.begin() + i); 
			}
		} 
	}
	void JDirectory::BeginForcedDestroy()
	{
		bool preIgnore = IsIgnoreUndestroyableFlag();
		if (!preIgnore)
			SetIgnoreUndestroyableFlag(true);
		BeginDestroy();
	}
	void JDirectory::OpenDirectory()noexcept
	{
		Activate();
	}
	void JDirectory::CloseDirectory()noexcept
	{
		DeActivate();
	}
	Core::J_FILE_IO_RESULT JDirectory::StoreObject(JDirectory* dir)
	{ 
		if (dir->HasFlag(OBJECT_FLAG_DO_NOT_SAVE))
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(ConvertMetafilePath(dir->GetPath()), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, dir);
		stream.close();

		return res;
	}
	JDirectory* JDirectory::LoadObject(JDirectory* parent, const JDirectoryPathData& pathData)
	{
		std::wifstream stream; 
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		JObject::ObjectMetadata metafile;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metafile);
		stream.close();

		JDirectory* newDir = nullptr;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(pathData.name, metafile.guid, metafile.flag, parent);
			newDir = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
		}
		else
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(pathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
			newDir = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			StoreObject(newDir);
		}

		return newDir;
	}
	void JDirectory::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* parent)
		{
			std::wstring name = GetDefaultName<JDirectory>();
			if (parent != nullptr)
				name = JCommonUtility::MakeUniqueName(parent->children, name);

			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
			JDirectory* newDir = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			StoreObject(newDir);
			return newDir;
		};
		auto initC = [](const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* parent)
		{
			std::wstring newName = name;
			if (parent != nullptr)
				newName = JCommonUtility::MakeUniqueName(parent->children, newName);

			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(newName, guid, objFlag, parent);
			JDirectory* newDir = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return newDir;
		};
		auto loadC = [](JDirectory* parentDir, const JDirectoryPathData& pathData)
		{
			return LoadObject(parentDir, pathData);
		};
		auto copyC = [](JDirectory* ori, JDirectory* parent)
		{
			//copy root dir is restricted
			std::wstring name = JCommonUtility::MakeUniqueName(parent->children, ori->GetName());
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(name, Core::MakeGuid(), ori->GetFlag(), parent);
			JDirectory* newDir = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			newDir->Copy(ori);
			StoreObject(newDir);
			return newDir;
		};

		JDFI::Register(defaultC, initC, loadC, copyC);
		JFFI::Register(&JDirectory::CreateJFile, &JDirectory::DestroyJFile);
	}
	JDirectory::JDirectory(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parent)
		:JDirectoryInterface(name, guid, flag), parent(parent)
	{
		if (parent != nullptr)
			parent->children.push_back(this);
	}
	JDirectory::~JDirectory() {}
}