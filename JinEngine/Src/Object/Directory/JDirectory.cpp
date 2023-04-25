#include"JDirectory.h" 
#include"JDirectoryPrivate.h"
#include"JFile.h"  
#include"JFilePrivate.h"
#include"JFileInitData.h"
#include"../Resource/JResourceObject.h" 
#include"../Resource/JResourceObjectPrivate.h"
#include"../../Application/JApplicationProject.h" 
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Core/Identity/JIdentifierImplBase.h"
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Utility/JCommonUtility.h"
#include<io.h>
#include<fstream>
#include<vector>  

namespace JinEngine
{
	namespace
	{
		using FileMap = Core::JSingletonHolder<std::unordered_map<size_t, JFile*>>;
		static JDirectoryPrivate dPrivate;
	}
 
	namespace
	{
		static std::wstring EraseInvalidNameChar(const std::wstring& wstr)noexcept
		{
			std::wstring res;
			for(const auto& data : wstr)
			{
				if(data == '<' || data == '>' || data == ':' || data == '"' || 
					data == '/' || data == '\\' || data == '?' || data == '*')
					continue;
				res.push_back(data);
			}
			return res;
		}
	}
	class JDirectory::JDirectoryImpl : public Core::JIdentifierImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JDirectoryImpl)
	public:
		JDirectory* thisDir;
	public:
		JDirectory* parent;
		std::vector<JDirectory*> children;
		std::vector<std::unique_ptr<JFile>> fileList;
	public:
		JDirectoryImpl(const InitData& initData, JDirectory* thisDir)
			:parent(initData.parent), thisDir(thisDir)
		{}
	public:
		void Initialize(const InitData& initData){}
	public:
		int GetFileIndex(const size_t guid)const noexcept
		{
			bool (*ptr)(JFile*, size_t) = [](JFile* a, size_t guid) {return a->GetResourceGuid() == guid; };
			return JCUtil::GetJIndex(fileList, ptr, guid);
		}
	public:
		static void ActivateDirectory(JDirectory* tar)
		{
			auto rawVec = JDirectory::StaticTypeInfo().GetInstanceRawPtrVec();
			for (auto& data : rawVec)
				static_cast<JDirectory*>(data)->DeActivate();
			tar->Activate();
		}
		static void DeActivateDirectory(JDirectory* tar)
		{
			tar->DeActivate();
		}
	public:
		void MoveDirectory(JDirectory* newParent)noexcept
		{
			//root node can't set parent
			if (parent == nullptr || newParent == nullptr)
				return;

			const std::wstring prePath = thisDir->GetPath();
			int preIndex = JCUtil::GetJIdenIndex(parent->impl->children, thisDir->GetGuid());
			parent->impl->children.erase(parent->impl->children.begin() + preIndex);

			newParent->impl->children.push_back(thisDir);
			parent = newParent;

			thisDir->SetName(JCUtil::MakeUniqueName(parent->impl->children, thisDir->GetName()));
			const std::wstring newPath = thisDir->GetPath();
			MoveFileExW(prePath.c_str(), newPath.c_str(), MOVEFILE_WRITE_THROUGH);
		}
	public:
		static bool DoCopy(JDirectory* from, JDirectory* to)
		{
			to->impl->Clear();
			JDirectory* fromDir = static_cast<JDirectory*>(from);
			const uint fileCount = (uint)fromDir->impl->fileList.size();
			for (uint i = 0; i < fileCount; ++i)
			{
				Core::JTypeInfo& info = fromDir->impl->fileList[i]->GetResourceTypeInfo();
				auto derivedVec = Core::JReflectionInfo::Instance().GetDerivedTypeInfo(info);
				for (auto& data : derivedVec)
				{
					auto rawPtr = data->GetInstanceRawPtr(fromDir->impl->fileList[i]->GetResourceGuid());
					if (rawPtr != nullptr)
						JResourceObjectPrivate::FileInterface::CopyJFile(static_cast<JResourceObject*>(rawPtr), to);
				}
			}
			for (const auto& child : fromDir->impl->children)
				JICI::CreateAndCopy(child, to);
			return true;
		}
	public:
		std::unique_ptr<JDirectory::InitData> CreateInitData(JDirectory* parent = nullptr)
		{
			return std::make_unique<JDirectory::InitData>(JDirectory::GetDefaultName(JDirectory::StaticTypeInfo()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				parent);
		}
		//Create or exist dir is true
		static bool CreateDirectoryFile(const std::wstring& path)
		{
			if (_waccess(path.c_str(), 00) == -1)
			{
				if (_wmkdir(path.c_str()) == -1)
					return false;
			}
			return true;
		}
		static void DeleteDirectoryFile(const std::wstring& path)
		{
			if (_waccess(path.c_str(), 00) != -1)
				_wremove(path.c_str());
		}
	public:
		void Clear()
		{
			std::vector<JDirectory*> copyD = children;
			const uint childrenCount = (uint)copyD.size();
			for (uint i = 0; i < childrenCount; ++i)
				Core::JIdentifier::BeginForcedDestroy(copyD[i]);

			std::vector<JResourceObject*> rVec;
			for (const auto& data : fileList)
			{
				auto rawPtr = data->GetResource();
				if (rawPtr != nullptr)
					rVec.push_back(rawPtr);
			}
			for (auto& data : rVec)
				Core::JIdentifier::BeginForcedDestroy(data);

			const uint restFileCount = (uint)fileList.size();
			for (uint i = 0; i < restFileCount; ++i)
			{ 
				FileMap::Instance().erase(fileList[i]->GetResourceGuid());
				fileList[i].reset();
			}
			children.clear();
			fileList.clear();
		}
	public:
		bool RegisterInstance()noexcept
		{
			if (parent != nullptr)
				parent->impl->children.push_back(thisDir);
			return true; 
		}
		bool DeRegisterInstance()noexcept
		{
			if (parent != nullptr)
			{
				int index = JCUtil::GetJIdenIndex(parent->impl->children, thisDir->GetGuid());
				parent->impl->children.erase(parent->impl->children.begin() + index);
				parent = nullptr;
			} 
			return true;
		}
	public:
		static void RegisterCallOnce()
		{   
			JIdentifier::RegisterPrivateInterface(JDirectory::StaticTypeInfo(), dPrivate);  
		}
	};

	JDirectory::InitData::InitData(JDirectory* parent)
		:JObject::InitData(JDirectory::StaticTypeInfo()), parent(parent)
	{
		if (parent != nullptr)
			name = JCUtil::MakeUniqueName(parent->impl->children, EraseInvalidNameChar(name));
	}
	JDirectory::InitData::InitData(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* parent)
		: JObject::InitData(JDirectory::StaticTypeInfo(), name, guid, flag), parent(parent)
	{
		if (parent != nullptr)
			InitData::name = JCUtil::MakeUniqueName(parent->impl->children, EraseInvalidNameChar(name));
	}

	JDirectory::LoadData::LoadData(JDirectory* parent, const Core::JAssetFileLoadPathData& pathData)
		: parent(parent), pathData(pathData)
	{}
	JDirectory::LoadData::~LoadData()
	{}

	std::wstring JDirectory::GetPath()const noexcept
	{
		if (impl->parent == nullptr)
			return GetName();
		else
			return Core::JFileConstant::MakeFilePath(impl->parent->GetPath(), GetName());
	}
	std::wstring JDirectory::GetMetaFilePath()const noexcept
	{
		return GetPath() + Core::JFileConstant::GetMetaFileFormat();
	}
	uint JDirectory::GetChildernDirctoryCount()const noexcept
	{
		return (uint)impl->children.size();
	}
	uint JDirectory::GetFileCount()const noexcept
	{
		return (uint)impl->fileList.size();
	}
	JDirectory* JDirectory::GetParent()const noexcept
	{
		return impl->parent;
	}
	JDirectory* JDirectory::GetChildDirctory(const uint index)const noexcept
	{
		if (index >= (uint)impl->children.size())
			return nullptr;
		else
			return impl->children[index];
	}
	JDirectory* JDirectory::GetChildDirctoryByName(const std::wstring& name)const noexcept
	{
		for (const auto& data : impl->children)
		{
			if (data->GetName() == name)
				return data;
		}
		return nullptr;
	}
	JDirectory* JDirectory::GetChildDirctoryByPath(const std::wstring& path)const noexcept
	{
		const std::wstring thisPath = GetPath();
		for (const auto& data : impl->children)
		{
			if (path == thisPath + L"\\" + data->GetName())
				return data;
		}
		return nullptr;
	}
	JFile* JDirectory::GetDirectoryFile(const uint index)const noexcept
	{
		if (index >= impl->fileList.size())
			return nullptr;
		else
			return impl->fileList[index].get();
	}
	JFile* JDirectory::GetDirectoryFile(const std::wstring oriFormatName)const noexcept
	{
		for (const auto& data : impl->fileList)
		{
			if (data->GetFullName() == oriFormatName)
				return data.get();
		}
		return nullptr;
	}
	JFile* JDirectory::GetRecentFile()const noexcept
	{
		return impl->fileList.back().get();
	}
	J_OBJECT_TYPE JDirectory::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::DIRECTORY_OBJECT;
	}
	Core::JIdentifierPrivate& JDirectory::GetPrivateInterface()const noexcept
	{
		return dPrivate;
	}
	void JDirectory::SetName(const std::wstring& newName)noexcept
	{
		std::wstring validName = EraseInvalidNameChar(newName);
		if (validName == GetName())
			return;

		const std::wstring prePath = GetMetaFilePath();
		if (impl->parent != nullptr)
			JObject::SetName(JCUtil::MakeUniqueName(impl->parent->impl->children, validName));
		else
			JObject::SetName(validName);

		const std::wstring newPath = GetMetaFilePath();
		_wrename(prePath.c_str(), newPath.c_str());
	}
	bool JDirectory::HasChild(const std::wstring& name)const noexcept
	{
		for (const auto& data : impl->children)
		{
			if (data->GetName() == name)
				return true;
		}
		return false;
	}
	bool JDirectory::HasFile(const size_t guid)const noexcept
	{
		for (const auto& data : impl->fileList)
		{
			if (data->GetResourceGuid() == guid)
				return true;
		}
		return false;
	}
	bool JDirectory::IsParent(JDirectory* dir)const noexcept
	{
		if (impl->parent == nullptr)
			return false;
		if (impl->parent->GetGuid() == dir->GetGuid())
			return true;
		else
			return impl->parent->IsParent(dir);
	}
	bool JDirectory::IsOpen()const noexcept
	{
		return IsActivated();
	}
	JDirectory* JDirectory::SearchDirectory(const std::wstring& path)const noexcept
	{
		JDirectory* res = GetChildDirctoryByPath(path);
		if (res == nullptr)
		{
			for (const auto& data : impl->children)
			{
				res = data->SearchDirectory(path);
				if (res != nullptr)
					break;
			}
		}
		return res;
	}
	JFile* JDirectory::SearchFile(const std::wstring& oriFormatName)const noexcept
	{
		JFile* file = GetDirectoryFile(oriFormatName);
		if (file != nullptr)
			return file;

		for (const auto& data : impl->children)
		{
			file = data->SearchFile(oriFormatName);
			if (file != nullptr)
				return file;
		}
		return nullptr;
	}
	JFile* JDirectory::SearchFile(const size_t resourceGuid)noexcept
	{
		auto data = FileMap::Instance().find(resourceGuid);
		return data != FileMap::Instance().end() ? data->second : nullptr;
	}
	std::wstring JDirectory::MakeUniqueFileName(const std::wstring& name, const std::wstring& format, const size_t guid)const noexcept
	{
		auto conditionLam = [](JFile* file, std::wstring name, std::wstring format, size_t guid)
		{
			return file->GetName() == name && file->GetOriginalResourceFormat() == format && file->GetResourceGuid() != guid;
		};
		bool(*ptr)(JFile*, std::wstring, std::wstring, size_t) = conditionLam;
		 
		std::wstring validName = EraseInvalidNameChar(name);
		std::vector<JFile*> overlappedFile = JCUtil::GetPassConditionElement(impl->fileList, ptr, validName, format, guid);
		return JCUtil::MakeUniqueName(overlappedFile, validName, &JFile::GetName);
	}
	JDirectory::JDirectory(const InitData& initData)
		:JObject(initData), impl(std::make_unique<JDirectoryImpl>(initData, this))
	{
		impl->Initialize(initData);
	}
	JDirectory::~JDirectory()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JDirectoryPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JDirectoryPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JDirectoryPrivate::AssetDataIOInterface;
	using FileInterface = JDirectoryPrivate::FileInterface;
	using ActivationInterface = JDirectoryPrivate::ActivationInterface;
	using RawDirectoryInterface = JDirectoryPrivate::RawDirectoryInterface;
	using DestroyInstanceInterfaceEx = JDirectoryPrivate::DestroyInstanceInterfaceEx;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		JDirectory::InitData* dInitData = static_cast<JDirectory::InitData*>(initData.get());
		if (dInitData->parent != nullptr)
			dInitData->name = JCUtil::MakeUniqueName(dInitData->parent->impl->children, dInitData->name);

		return Core::JPtrUtil::MakeOwnerPtr<JDirectory>(*dInitData);
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JDirectory::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
	{
		JDirectory* dPtr = static_cast<JDirectory*>(createdPtr);
		dPtr->impl->RegisterInstance();
	}
	void CreateInstanceInterface::SetValidInstance(Core::JIdentifier* createdPtr)noexcept
	{
		JDirectory* dPtr = static_cast<JDirectory*>(createdPtr);
		dPtr->impl->CreateDirectoryFile(dPtr->GetPath());
		if (_waccess(dPtr->GetMetaFilePath().c_str(), 00) == -1)
		{ 
			JDirectory::StoreData storeData(dPtr);
			AssetDataIOInterface::StoreAssetData(&storeData);
		}
	}
	bool CreateInstanceInterface::Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JDirectory::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JDirectory::JDirectoryImpl::DoCopy(static_cast<JDirectory*>(from), static_cast<JDirectory*>(to));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{ 
		static_cast<JDirectory*>(ptr)->impl->Clear();
	}
	void DestroyInstanceInterface::SetInvalidInstance(Core::JIdentifier* ptr)noexcept
	{ 
		if (static_cast<JDirectory*>(ptr)->IsActivated())
			static_cast<JDirectory*>(ptr)->DeActivate();
	}
	void DestroyInstanceInterface::DeRegisterCash(Core::JIdentifier* ptr)noexcept
	{ 
		static_cast<JDirectory*>(ptr)->impl->DeRegisterInstance();
	}

	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateLoadAssetDIData(JDirectory* parent, const Core::JAssetFileLoadPathData& pathData)
	{
		return std::make_unique<JDirectory::LoadData>(parent, pathData);
	}
	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::LoadData::StaticTypeInfo()))
			return nullptr;

		auto loadData = static_cast<JDirectory::LoadData*>(data);
		auto& pathData = loadData->pathData;
		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		std::unique_ptr<JDirectory::InitData> initData = std::make_unique<JDirectory::InitData>(loadData->parent);
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetaData(stream, initData.get());
		stream.close();

		JDirectory* newDir = nullptr;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			initData->name = pathData.name;
			Core::JIdentifier* res = dPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &dPrivate);
			newDir = static_cast<JDirectory*>(res);
		}
		return newDir;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JDirectory::StoreData*>(data);
		JDirectory* dObj = static_cast<JDirectory*>(storeData->obj);

		std::wofstream stream;
		stream.open(dObj->GetMetaFilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT res = StoreMetaData(stream, storeData);
		stream.close();
		return res;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(std::wifstream& stream, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto dirInit = static_cast<JDirectory::InitData*>(data);
		JFileIOHelper::LoadObjectIden(stream, dirInit->guid, dirInit->flag);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(std::wofstream& stream, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto dirStore = static_cast<JDirectory::StoreData*>(data);
		JFileIOHelper::StoreObjectIden(stream, dirStore->obj);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	void FileInterface::ConvertToActFileData(JResourceObject* rObj) noexcept
	{
		auto file = JDirectory::SearchFile(rObj->GetGuid());
		if (file != nullptr)
			JFilePrivate{}.ConvertToActFileData(file, rObj);
	}
	void FileInterface::ConvertToDeActFileData(const size_t guid) noexcept
	{
		auto file = JDirectory::SearchFile(guid);
		if (file != nullptr)
			JFilePrivate{}.ConvertToDeActFileData(file);
	}
	JFile* FileInterface::CreateJFile(const JFileInitData& initData, JDirectory* owner)
	{
		if (owner != nullptr && owner->HasFile(initData.rGuid))
			return nullptr;

		std::unique_ptr<JFile> newFile = JFilePrivate{}.CreateFile(initData, owner);
		FileMap::Instance().emplace(newFile->GetResourceGuid(), newFile.get());
		owner->impl->fileList.emplace_back(std::move(newFile));
		return owner->impl->fileList.back().get();
	}
	bool FileInterface::DestroyJFile(const size_t rGuid)
	{
		JFile* file = JDirectory::SearchFile(rGuid);
		if (file == nullptr)
			return false;

		JDirectory* owner = file->GetOwnerDirectory();
		int index = owner->impl->GetFileIndex(rGuid);
		 
		owner->impl->fileList.erase(owner->impl->fileList.begin() + index);
		FileMap::Instance().erase(rGuid);
		return true;
	}

	void ActivationInterface::OpenDirectory(JDirectory* dir)noexcept
	{
		dir->Activate();
	}
	void ActivationInterface::CloseDirectory(JDirectory* dir)noexcept
	{
		dir->DeActivate();
	}

	void RawDirectoryInterface::MoveDirectory(JDirectory* dir, JDirectory* newParent)noexcept
	{
		dir->impl->MoveDirectory(newParent);
	}
	void RawDirectoryInterface::DeleteDirectory(JDirectory* dir)noexcept
	{
		dir->impl->DeleteDirectoryFile(dir->GetPath());
	}

	void DestroyInstanceInterfaceEx::BeginForcedDestroy(JDirectory* dir)noexcept
	{
		JDirectory::BeginForcedDestroy(dir);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JDirectoryPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JDirectoryPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}