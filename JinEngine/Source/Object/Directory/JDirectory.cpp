#include"JDirectory.h" 
#include"JDirectoryPrivate.h"
#include"JFile.h"  
#include"JFilePrivate.h"
#include"JFileInitData.h"
#include"../JObjectFileIOHelper.h"
#include"../Resource/JResourceObject.h" 
#include"../Resource/JResourceObjectPrivate.h"
#include"../../Application/JApplicationProject.h" 
#include"../../Core/Identity/JIdenCreator.h"
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../../Core/Reflection/JTypeBasePrivate.h"
#include"../../Core/Guid/JGuidCreator.h" 
#include"../../Core/File/JFileConstant.h"
#include"../../Core/Utility/JCommonUtility.h"
#include<io.h>
#include<fstream>
#include<vector>  

namespace JinEngine
{
	namespace
	{
		using InstanceInterface = Core::JTypeBasePrivate::InstanceInterface;
	}
	namespace
	{ 
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
	class JDirectory::JDirectoryImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JDirectoryImpl)
	public:
		JWeakPtr<JDirectory> thisPointer;
	public:
		JUserPtr<JDirectory> parent;
		std::vector<JUserPtr<JDirectory>> children;
		std::vector<JUserPtr<JFile>> fileList;
	public:
		JDirectoryImpl(const InitData& initData, JDirectory* thisDirRaw)
			:parent(initData.parent)
		{} 
	public:
		std::vector<JUserPtr<JFile>> GetDirectoryFileVec()const noexcept
		{
			return fileList;
		}
		std::vector<JUserPtr<JFile>> GetDirectoryFileVec(const J_RESOURCE_TYPE type)const noexcept
		{
			std::vector<JUserPtr<JFile>> res;
			for (const auto& data : fileList)
			{
				if (data->GetResourceType() == type)
					res.push_back(data);
			}
			return res;
		}
		int GetFileIndex(const size_t guid)const noexcept
		{ 
			bool (*ptr)(const JUserPtr<JFile>&, size_t) = [](const JUserPtr<JFile>& a, size_t guid)
			{
				//clear중에는 fileList에 invalidFile이 섞여있음
				return a != nullptr ? a->GetResourceGuid() == guid : false;
			};
			return JCUtil::GetIndex(fileList, ptr, guid);
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

			const std::wstring prePath = thisPointer->GetPath();
			int preIndex = JCUtil::GetTypeIndex(parent->impl->children, thisPointer->GetGuid());
			parent->impl->children.erase(parent->impl->children.begin() + preIndex);

			newParent->impl->children.push_back(thisPointer);
			parent = Core::GetUserPtr(newParent);

			thisPointer->SetName(JCUtil::MakeUniqueName(parent->impl->children, thisPointer->GetName()));
			const std::wstring newPath = thisPointer->GetPath();
			MoveFileExW(prePath.c_str(), newPath.c_str(), MOVEFILE_WRITE_THROUGH);
		}
	public:
		static bool DoCopy(JDirectory* from, JDirectory* to)
		{ 
			JUserPtr<JDirectory> toUser = Core::GetUserPtr(to);
			toUser->impl->Clear();

			const uint fileCount = (uint)from->impl->fileList.size();
			for (uint i = 0; i < fileCount; ++i)
			{
				Core::JTypeInfo& info = from->impl->fileList[i]->GetResourceTypeInfo();
				auto derivedVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(info);
				for (auto& data : derivedVec)
				{
					auto rUser = data->GetInstanceUserPtr<JResourceObject>(from->impl->fileList[i]->GetResourceGuid());
					if (rUser != nullptr)
						JResourceObjectPrivate::FileInterface::CopyJFile(rUser, toUser);
				}
			}
			 
			for (const auto& child : from->impl->children)
			{
				auto initData = std::make_unique<JDirectory::InitData>(child->GetName(), Core::MakeGuid(), OBJECT_FLAG_NONE, toUser);
				JICI::CreateAndCopy(std::move(initData), child);
			}
			return true;
		}
	public:
		std::unique_ptr<JDirectory::InitData> CreateInitData(JUserPtr<JDirectory> parent = nullptr)
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
			JFileIOHelper::DestroyDirectory(path); 
		}
	public:
		void Clear()
		{
			auto copyD = children;
			for(auto& data: copyD)
				Core::JIdentifier::BeginForcedDestroy(data.Get());

			std::vector<JUserPtr<JResourceObject>> rVec;
			auto copyF = fileList;
		 
			for (auto& data : copyF)
			{
				auto rawPtr = data->GetResource().Get();
				if (rawPtr != nullptr)
					Core::JIdentifier::BeginForcedDestroy(rawPtr);

				//Resource Destory시 File이 소유하는 Data만 Act에서 DeAct로 컨버트하나
				//Shader같이 Resource Destory시 File도 Destory하는 경우도 존재함 
				if (data.IsValid())
				{
					InstanceInterface::RemoveInstance(data.Get());
					data.Clear();
				}
			}
 
			children.clear();
			fileList.clear();
		}
	public:
		bool RegisterInstance()noexcept
		{
			if (parent != nullptr)
				parent->impl->children.push_back(thisPointer);
			return true; 
		}
		bool DeRegisterInstance()noexcept
		{
			if (parent != nullptr)
			{
				int index = JCUtil::GetTypeIndex(parent->impl->children, thisPointer->GetGuid());
				parent->impl->children.erase(parent->impl->children.begin() + index);
				parent = nullptr;
			} 
			return true;
		}
	public:
		void RegisterThisPointer(JDirectory* dir)
		{
			thisPointer = Core::GetWeakPtr(dir);
		}
		static void RegisterTypeData()
		{   
			Core::JIdentifier::RegisterPrivateInterface(JDirectory::StaticTypeInfo(), dPrivate);  
			IMPL_REALLOC_BIND(JDirectory::JDirectoryImpl, thisPointer)
		}
	};

	JDirectory::InitData::InitData(const JUserPtr<JDirectory>& parent)
		:JObject::InitData(JDirectory::StaticTypeInfo()), parent(parent)
	{
		if (parent != nullptr)
			name = JCUtil::MakeUniqueName(parent->impl->children, EraseInvalidNameChar(name));
	}
	JDirectory::InitData::InitData(const size_t guid, const JUserPtr<JDirectory>& parent)
		: JObject::InitData(JDirectory::StaticTypeInfo(), guid), parent(parent)
	{
		if (parent != nullptr)
			name = JCUtil::MakeUniqueName(parent->impl->children, EraseInvalidNameChar(name));
	}

	JDirectory::InitData::InitData(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JDirectory>& parent)
		: JObject::InitData(JDirectory::StaticTypeInfo(), name, guid, flag), parent(parent)
	{
		if (parent != nullptr)
			InitData::name = JCUtil::MakeUniqueName(parent->impl->children, EraseInvalidNameChar(name));
	}

	JDirectory::LoadData::LoadData(const JUserPtr<JDirectory>& parent, const Core::JAssetFileLoadPathData& pathData)
		: parent(parent), pathData(pathData)
	{}
	JDirectory::LoadData::~LoadData()
	{}

	Core::JIdentifierPrivate& JDirectory::PrivateInterface()const noexcept
	{
		return dPrivate;
	} 
	J_OBJECT_TYPE JDirectory::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::DIRECTORY_OBJECT;
	}
	std::wstring JDirectory::GetPath()const noexcept
	{
		if (impl->parent == nullptr)
			return GetName();
		else
			return Core::JFileConstant::MakeFilePath(impl->parent->GetPath(), GetName());
	}
	std::wstring JDirectory::GetMetaFilePath()const noexcept
	{
		return GetPath() + Core::JFileConstant::GetMetaFileFormatW();
	}
	uint JDirectory::GetChildernDirctoryCount()const noexcept
	{
		return (uint)impl->children.size();
	}
	uint JDirectory::GetFileCount()const noexcept
	{
		return (uint)impl->fileList.size();
	}
	JUserPtr<JDirectory> JDirectory::GetParent()const noexcept
	{
		return impl->parent;
	}
	JUserPtr<JDirectory> JDirectory::GetChildDirctory(const uint index)const noexcept
	{
		if (index >= (uint)impl->children.size())
			return nullptr;
		else
			return impl->children[index];
	}
	JUserPtr<JDirectory> JDirectory::GetChildDirctoryByName(const std::wstring& name)const noexcept
	{
		for (const auto& data : impl->children)
		{
			if (data->GetName() == name)
				return data;
		}
		return nullptr;
	}
	JUserPtr<JDirectory> JDirectory::GetChildDirctoryByPath(const std::wstring& path)const noexcept
	{
		const std::wstring thisPath = GetPath();
		for (const auto& data : impl->children)
		{
			if (path == thisPath + L"\\" + data->GetName())
				return data;
		}
		return nullptr;
	}
	JUserPtr<JFile> JDirectory::GetDirectoryFile(const uint index)const noexcept
	{
		if (index >= impl->fileList.size())
			return nullptr;
		else
			return impl->fileList[index];
	}
	JUserPtr<JFile> JDirectory::GetDirectoryFileByName(const std::wstring& name)const noexcept
	{
		for (const auto& data : impl->fileList)
		{
			if (data->GetName() == name)
				return data;
		}
		return nullptr;
	}
	JUserPtr<JFile> JDirectory::GetDirectoryFileByFullName(const std::wstring& name, const std::wstring& format)const noexcept
	{
		const std::wstring fullname = name + format;
		for (const auto& data : impl->fileList)
		{
			if (data->GetFullName() == fullname)
				return data;
		}
		return nullptr;
	}
	JUserPtr<JFile> JDirectory::GetRecentFile()const noexcept
	{
		return impl->fileList.back();
	}
	std::vector<JUserPtr<JDirectory>> JDirectory::GetChildDirctoryVec()const noexcept
	{
		return impl->children;
	}
	std::vector<JUserPtr<JFile>> JDirectory::GetDirectoryFileVec(const bool containChildFile)const noexcept
	{
		if(!containChildFile)
			return impl->fileList;
		else
		{
			std::vector<JUserPtr<JFile>> files = impl->fileList;
			for (const auto& data : impl->children)
			{
				auto childFile = data->GetDirectoryFileVec(containChildFile); 
				files.insert(files.end(), childFile.begin(), childFile.end());
			}
			return files;
		}
	}
	std::vector<JUserPtr<JFile>> JDirectory::GetDirectoryFileVec(const bool containChildFile, const J_RESOURCE_TYPE type)const noexcept
	{
		if (!containChildFile)
			return impl->GetDirectoryFileVec(type);
		else
		{
			std::vector<JUserPtr<JFile>> files = impl->GetDirectoryFileVec(type);
			for (const auto& data : impl->children)
			{
				auto childFile = data->GetDirectoryFileVec(containChildFile, type);
				files.insert(files.end(), childFile.begin(), childFile.end());
			}
			return files;
		}
	}
	void JDirectory::SetName(const std::wstring& newName)noexcept
	{
		std::wstring validName = EraseInvalidNameChar(newName);
		if (validName == GetName())
			return;

		const std::wstring preMetafilePath = GetMetaFilePath();
		const std::wstring prePath = GetPath();
		if (impl->parent != nullptr)
			JObject::SetName(JCUtil::MakeUniqueName(impl->parent->impl->children, validName));
		else
			JObject::SetName(validName);
		 
		_wrename(preMetafilePath.c_str(), GetMetaFilePath().c_str());
		_wrename(prePath.c_str(), GetPath().c_str());
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
	JUserPtr<JDirectory> JDirectory::SearchDirectory(const std::wstring& path)const noexcept
	{
		JUserPtr<JDirectory> res = GetChildDirctoryByPath(path);
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
	JUserPtr<JFile> JDirectory::SearchFile(const std::wstring& name)const noexcept
	{
		JUserPtr<JFile> file = GetDirectoryFileByName(name);
		if (file != nullptr)
			return file;

		for (const auto& data : impl->children)
		{
			file = data->SearchFile(name);
			if (file != nullptr)
				return file;
		}
		return nullptr;
	}
	JUserPtr<JFile> JDirectory::SearchFile(const std::wstring& name, const std::wstring& format)const noexcept
	{
		JUserPtr<JFile> file = GetDirectoryFileByFullName(name, format);
		if (file != nullptr)
			return file;

		for (const auto& data : impl->children)
		{
			file = data->SearchFile(name, format);
			if (file != nullptr)
				return file;
		}
		return nullptr;
	}
	JUserPtr<JFile> JDirectory::SearchFile(const size_t resourceGuid)noexcept
	{ 
		return Core::GetUserPtr<JFile>(JFile::StaticTypeInfo().TypeGuid(), resourceGuid);
	}
	void JDirectory::AlignByName(std::vector<JUserPtr<JFile>>& vec, const bool isAscending)noexcept
	{ 
		auto aSortLam = [](const JUserPtr<JFile>& a, const JUserPtr<JFile>& b)
		{
			return tolower(a->GetName()[0]) < tolower(b->GetName()[0]);
		};
		auto dSortLam = [](const JUserPtr<JFile>& a, const JUserPtr<JFile>& b)
		{
			return tolower(a->GetName()[0]) > tolower(b->GetName()[0]);
		};	 
		if (isAscending)
			std::sort(vec.begin(), vec.end(), aSortLam);
		else
			std::sort(vec.begin(), vec.end(), dSortLam);
	}
	std::wstring JDirectory::MakeUniqueFileName(const std::wstring& name, const std::wstring& format, const size_t guid)const noexcept
	{
		auto conditionLam = [](const JUserPtr<JFile>& file, const std::wstring& format, size_t guid)
		{		
			return file->GetOriginalResourceFormat() == format && file->GetResourceGuid() != guid;
		};
		bool(*ptr)(const JUserPtr<JFile>&, const std::wstring&, size_t) = conditionLam;
		  
		auto overlappedFile = JCUtil::GetPassConditionElement<JUserPtr<JFile>, JWeakPtr<JFile>, const std::wstring&, size_t>(impl->fileList, ptr, format, guid);
		return JCUtil::MakeUniqueName(overlappedFile, EraseInvalidNameChar(name));
	}
	JDirectory::JDirectory(const InitData& initData)
		:JObject(initData), impl(std::make_unique<JDirectoryImpl>(initData, this))
	{}
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

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		JDirectory::InitData* dInitData = static_cast<JDirectory::InitData*>(initData);
		if (dInitData->parent != nullptr)
			dInitData->name = JCUtil::MakeUniqueName(dInitData->parent->impl->children, dInitData->name);

		return Core::JPtrUtil::MakeOwnerPtr<JDirectory>(*dInitData);
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JDirectory::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JDirectory* dir = static_cast<JDirectory*>(createdPtr);
		dir->impl->RegisterThisPointer(dir);
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
			JDirectory::StoreData storeData(Core::GetUserPtr(dPtr));
			AssetDataIOInterface::StoreAssetData(&storeData);
		}
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JDirectory::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JDirectory::JDirectoryImpl::DoCopy(static_cast<JDirectory*>(from.Get()), static_cast<JDirectory*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JDirectory*>(ptr)->impl->Clear();
		JObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
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

	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateLoadAssetDIData(const JUserPtr<JDirectory>& parent, const Core::JAssetFileLoadPathData& pathData)
	{
		return std::make_unique<JDirectory::LoadData>(parent, pathData);
	}
	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::LoadData::StaticTypeInfo()))
			return nullptr;

		auto loadData = static_cast<JDirectory::LoadData*>(data);
		auto& pathData = loadData->pathData;

		JFileIOTool tool;
		if (!tool.Begin(pathData.engineMetaFileWPath, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return nullptr;

		std::unique_ptr<JDirectory::InitData> initData = std::make_unique<JDirectory::InitData>(loadData->parent);
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetaData(tool, initData.get());
		tool.Close();
		  
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			initData->name = pathData.name;
			return dPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &dPrivate);
		}
		else
			return nullptr;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JDirectory::StoreData*>(data);
		JUserPtr<JDirectory> dirUser;
		dirUser.ConnnectChild(storeData->obj);

		JFileIOTool tool;
		if (!tool.Begin(dirUser->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		Core::J_FILE_IO_RESULT res = StoreMetaData(tool, storeData);
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		return res;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto dirInit = static_cast<JDirectory::InitData*>(data);
		JObjectFileIOHelper::LoadObjectIden(tool, dirInit->guid, dirInit->flag);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectory::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		if (!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto dirStore = static_cast<JDirectory::StoreData*>(data);
		JObjectFileIOHelper::StoreObjectIden(tool, dirStore->obj.Get());
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	void FileInterface::ConvertToActFileData(const JUserPtr<JResourceObject>& rObj) noexcept
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
	JUserPtr<JFile> FileInterface::CreateJFile(const JFileInitData& initData, const JUserPtr<JDirectory>& owner)
	{
		if (owner != nullptr && owner->HasFile(initData.rGuid))
			return nullptr;

		JOwnerPtr<JFile> newFile = JFilePrivate{}.CreateFile(initData, owner);
		owner->impl->fileList.push_back(newFile);
		InstanceInterface::AddInstance(std::move(newFile));
		return owner->impl->fileList.back();
	}
	bool FileInterface::DestroyJFile(const size_t rGuid)
	{
		JUserPtr<JFile> file = JDirectory::SearchFile(rGuid);
		if (file == nullptr)
			return false;

		JUserPtr<JDirectory> ownerDir = file->GetOwnerDirectory();
		int index = ownerDir->impl->GetFileIndex(rGuid);
		 
		ownerDir->impl->fileList.erase(ownerDir->impl->fileList.begin() + index);
		InstanceInterface::RemoveInstance(file.Get());
		return true;
	}

	void ActivationInterface::OpenDirectory(const JUserPtr<JDirectory>& dir)noexcept
	{
		dir->Activate();
	}
	void ActivationInterface::CloseDirectory(const JUserPtr<JDirectory>& dir)noexcept
	{
		dir->DeActivate();
	}

	void RawDirectoryInterface::MoveDirectory(const JUserPtr<JDirectory>& dir, const JUserPtr<JDirectory>& newParent)noexcept
	{
		dir->impl->MoveDirectory(newParent.Get());
	}
	void RawDirectoryInterface::DeleteDirectory(const JUserPtr<JDirectory>& dir)noexcept
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