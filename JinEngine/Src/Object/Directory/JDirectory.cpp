#include"JDirectory.h"
#include"JDirectoryFactory.h"
#include"JFile.h" 
#include"JFileFactory.h"
#include"../Resource/JResourceObject.h"
#include"../Resource/JResourceManager.h"
#include"../Resource/JResourceObjectFactory.h"
#include"../../Application/JApplicationVariable.h" 
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Utility/JCommonUtility.h"
#include<io.h>
#include<fstream>

namespace JinEngine
{
	JDirectory* JDirectory::GetParent()noexcept
	{
		return parent;
	}
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
			return GetName();
		else
			return Core::JFileConstant::MakeFilePath(parent->GetPath(), GetName());
	}
	std::wstring JDirectory::GetMetafilePath()const noexcept
	{
		return GetPath() + Core::JFileConstant::GetMetafileFormat();
	}
	JFile* JDirectory::GetFile(const uint index)noexcept
	{
		if (index >= fileList.size())
			return nullptr;
		else
			return fileList[index];
	}
	JFile* JDirectory::GetFile(const std::wstring name)noexcept
	{
		const uint fileCount = (uint)fileList.size();
		for (uint i = 0; i < fileCount; ++i)
		{ 
			if (fileList[i]->GetName() == name)
				return fileList[i];
		}
		return nullptr;
	}
	JFile* JDirectory::GetRecentFile()noexcept
	{
		return fileList[fileList.size() - 1];
	}
	J_OBJECT_TYPE JDirectory::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::DIRECTORY_OBJECT;
	}
	void JDirectory::SetName(const std::wstring& name)noexcept
	{
		const std::wstring prePath = GetMetafilePath();
		JObject::SetName(MakeUniqueFileName(name));
		const std::wstring newPath = GetMetafilePath();
		_wrename(prePath.c_str(), newPath.c_str());
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
		return JCUtil::MakeUniqueName(JCUtil::MakeUniqueName(fileList, name, &JFile::GetName), GetName());
	}
	JDirectoryOCInterface* JDirectory::OCInterface()
	{
		return this;
	}
	void JDirectory::DoCopy(JObject* ori)
	{
		Clear();
		JDirectory* oriDir = static_cast<JDirectory*>(ori);
		const uint fileCount = (uint)oriDir->fileList.size();
		for (uint i = 0; i < fileCount; ++i)
			JRFIB::CopyByName(oriDir->fileList[i]->GetResource()->GetTypeInfo().Name(), *oriDir->fileList[i]->GetResource(), *this);

		const uint childrenCount = (uint)oriDir->children.size();
		for (uint i = 0; i < childrenCount; ++i)
			JDFI::Copy(*oriDir->children[i], *this);
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
	bool JDirectory::Destroy(const bool isForced)
	{
		if (HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) && !isForced)
			return false;
		 
		Clear();
		JResourceManager::Instance().DirectoryStorageInterface()->RemoveJDirectory(*this);
		return true;
	}
	void JDirectory::Clear()
	{
		std::vector<JDirectory*> copyD = children;
		const uint childrenCount = (uint)copyD.size();
		for (uint i = 0; i < childrenCount; ++i)
			JObject::BegineForcedDestroy(copyD[i]);

		std::vector<JFile*> copyF = fileList;
		const uint fileCount = (uint)copyF.size();
		for (uint i = 0; i < fileCount; ++i)
			JObject::BegineForcedDestroy(copyF[i]->GetResource());
		
		children.clear();
		fileList.clear();
	}
	void JDirectory::OpenDirectory()noexcept
	{
		Activate();
	}
	void JDirectory::CloseDirectory()noexcept
	{
		DeActivate();
	}
	bool JDirectory::CreateDirectoryFile(const std::wstring& path)
	{
		if (_waccess(path.c_str(), 00) == -1)
		{
			if (_wmkdir(path.c_str()) == -1)
				return false;
		}
		return true;
	}
	void JDirectory::DeleteDirectoryFile(const std::wstring& path)
	{
		if (_waccess(path.c_str(), 00) != -1)
			_wremove(path.c_str());
	}
	bool JDirectory::RegisterCashData()noexcept
	{
		if (parent != nullptr)
			parent->children.push_back(this);
		return JResourceManager::Instance().DirectoryStorageInterface()->AddJDirectory(*this);
	}
	bool JDirectory::DeRegisterCashData()noexcept
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
		return JResourceManager::Instance().DirectoryStorageInterface()->RemoveJDirectory(*this);
	}
	Core::J_FILE_IO_RESULT JDirectory::StoreObject(JDirectory* dir)
	{
		if (dir->HasFlag(OBJECT_FLAG_DO_NOT_SAVE))
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(dir->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, dir);
		stream.close();
		return res;
	}
	JDirectory* JDirectory::LoadObject(JDirectory* parent, const Core::JAssetFileLoadPathData& pathData)
	{
		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JObject::JObjectMetaData metafile;
		Core::J_FILE_IO_RESULT loadMetaRes = JObject::LoadMetadata(stream, metafile);
		stream.close();

		JDirectory* newDir = nullptr;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(pathData.name, metafile.guid, metafile.flag, parent);
			newDir = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newDir;
		}	
		return nullptr;
	}
	void JDirectory::RegisterJFunc()
	{ 
		auto defaultC = [](JDirectory* parent) -> JDirectory*
		{
			std::wstring name = GetDefaultName<JDirectory>();
			if (parent != nullptr)
				name = JCUtil::MakeUniqueName(parent->children, name);

			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
			JDirectory* newDir = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)) && CreateDirectoryFile(newDir->GetPath()))
			{
				StoreObject(newDir);
				return newDir;
			}
			else
				return nullptr;
		};
		auto initC = [](const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* parent) -> JDirectory*
		{
			std::wstring newName = name;
			if (parent != nullptr)
				newName = JCUtil::MakeUniqueName(parent->children, newName);

			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(newName, guid, objFlag, parent);
			JDirectory* newDir = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)) && CreateDirectoryFile(newDir->GetPath()))
			{ 
				StoreObject(newDir);
				return newDir;
			}
			else
				return nullptr;
		};
		auto loadC = [](JDirectory* parentDir, const Core::JAssetFileLoadPathData& pathData)
		{
			return LoadObject(parentDir, pathData);
		};
		auto copyC = [](JDirectory* ori, JDirectory* parent) -> JDirectory*
		{
			//copy root dir is restricted
			std::wstring name = JCUtil::MakeUniqueName(parent->children, ori->GetName());
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JDirectory>(name, Core::MakeGuid(), ori->GetFlag(), parent);
			JDirectory* newDir = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)) && CreateDirectoryFile(newDir->GetPath()))
			{
				newDir->Copy(ori);
				StoreObject(newDir);
				return newDir;
			}
			else
				return nullptr;
		};

		JDFI::Register(defaultC, initC, loadC, copyC); 
		JFFI::Register(&JDirectory::CreateJFile, &JDirectory::DestroyJFile);
	}
	JDirectory::JDirectory(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parent)
		:JDirectoryInterface(name, guid, flag), parent(parent)
	{
		
	}
	JDirectory::~JDirectory() {}
}