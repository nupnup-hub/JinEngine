#include"JDirectory.h"
#include"JDirectoryFactory.h"
#include"JFile.h" 
#include"JFileFactory.h"
#include"../Resource/JResourceObject.h"
#include"../Resource/JResourceManager.h"
#include"../../Application/JApplicationVariable.h" 
#include"../../Core/Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JDirectory* JDirectory::GetChildDirctory(const uint index)noexcept
	{
		if (index >= (uint)children.size())
			return nullptr;
		else
			return children[index];
	}
	JDirectory* JDirectory::GetChildDirctory(const std::string& path)noexcept
	{
		std::string thisPath = GetPath();
		const uint childCount = (uint)children.size();
		for (uint i = 0; i < childCount; ++i)
		{
			if (path == thisPath + "\\" + children[i]->GetName())
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
	std::string JDirectory::GetPath()const noexcept
	{
		if (parent == nullptr)
			return JApplicationVariable::GetProjectPath();
		else
			return parent->GetPath() + "\\" + GetName();
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
	bool JDirectory::HasChild(const std::string& name)const noexcept
	{
		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			if (children[i]->GetName() == name)
				return true;
		}
		return false;
	}
	bool JDirectory::HasFile(const std::string& name)const noexcept
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
	JDirectory* JDirectory::SearchDirectory(const std::string& path)noexcept
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
	std::string JDirectory::MakeUniqueFileName(const std::string& name)noexcept
	{
		return JCommonUtility::MakeUniqueName(fileList, name);
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

		const size_t guid = GetGuid();
		const uint pChildrenCount = (uint)parent->children.size();
		for (uint i = 0; i < pChildrenCount; ++i)
		{
			if (guid == parent->children[i]->GetGuid())
			{
				parent->children.erase(parent->children.begin() + i);
				break;
			}
		}

		const uint childrenCount = (uint)children.size();
		for (uint i = 0; i < childrenCount; ++i)
		{
			children[i]->Destroy();
			children[i] = nullptr;
		}

		bool preIgonre = IsIgnoreUndestroyableFlag();
		if (!preIgonre)
			SetIgnoreUndestroyableFlag(true);

		const uint fileCount = (uint)fileList.size();
		for (uint i = 0; i < fileCount; ++i)
			fileList[i]->GetResource()->BeginDestroy();
		
		if (!preIgonre)
			SetIgnoreUndestroyableFlag(false);

		fileList.clear();
		children.clear();

		JResourceManager::Instance().DirectoryRemoveInterface()->RemoveJDirectory(*this);
		delete this;
	}
	void JDirectory::BeginForcedDestroy()
	{
		bool preIgonre = IsIgnoreUndestroyableFlag();
		if (!preIgonre)
			SetIgnoreUndestroyableFlag(true);
		BeginDestroy();
		if (!preIgonre)
			SetIgnoreUndestroyableFlag(false);
	}
	void JDirectory::OpenDirectory()noexcept
	{
		Activate();
	}
	void JDirectory::CloseDirectory()noexcept
	{
		DeActivate();
	}
	void JDirectory::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* parent)
		{
			std::string name = GetDefaultName<JDirectory>();
			if (parent != nullptr)
				name = JCommonUtility::MakeUniqueName(parent->children, name);
			return new JDirectory(name, Core::MakeGuid(), OBJECT_FLAG_NONE, parent);
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* parent)
		{
			std::string newName = name;
			if (parent != nullptr)
				newName = JCommonUtility::MakeUniqueName(parent->children, newName);
			return new JDirectory(newName, guid, objFlag, parent);
		};
		JDFI::Register(defaultC, initC);
		JFFI::Register(&JDirectory::CreateJFile, &JDirectory::DestroyJFile);
	}
	JDirectory::JDirectory(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parent)
		:JDirectoryOCInterface(name, guid, flag), parent(parent)
	{
		if (parent != nullptr)
			parent->children.push_back(this);
	}
	JDirectory::~JDirectory() {}
}