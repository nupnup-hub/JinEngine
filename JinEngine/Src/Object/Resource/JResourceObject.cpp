#include"JResourceObject.h"   
#include"JResourceObjectPrivate.h"
#include "JResourceObjectHint.h" 
#include "JResourceManager.h" 
#include"../Directory/JFile.h"
#include"../Directory/JFileInitData.h"
#include"../Directory/JDirectory.h" 
#include"../Directory/JDirectoryPrivate.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/Identity/JIdentifierImplBase.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Editor/Interface/JEditorObjectHandleInterface.h"
#include"../../Application/JApplicationEngine.h"
#include"../../Application/JApplicationProject.h"
#include<fstream>
#include<io.h>

namespace JinEngine
{ 
	class JResourceObjectEvent: public JResourceEventManager
	{
	public:
		JResourceObjectEvent()
			:JEventManager([](const size_t& a, const size_t& b) {return a == b; })
		{}
	public:
		JEventInterface* EvInterface()final
		{
			return this;
		}
	public:
		void NotifyEraseEvent(JResourceObject* rObj)
		{
			NotifyEvent(rObj->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE, rObj);
		}
	};
	namespace
	{
		static JResourceObjectEvent rEv;
	}

	class JResourceObject::JResourceObjectImpl : public Core::JIdentifierImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JResourceObjectImpl)
	public:
		JDirectory* directory;
		const uint8 formatIndex;
	public:
		JResourceObjectImpl(const InitData& initData)
			:formatIndex(initData.formatIndex), directory(initData.directory)
		{}
	public:
		static std::wstring GetCacheFilePath(JResourceObject* rObj) noexcept
		{
			return JApplicationProject::ModResourceCachePath() + L"\\" + std::to_wstring(rObj->GetGuid()) + Core::JFileConstant::GetCacheFileFormat();
		}
	public:
		static bool DoCopy(JResourceObject* from, JResourceObject* to)
		{
			bool isAct = to->IsActivated();
			if (isAct)
				to->DeActivate();

			//DestroyJFile(to->GetGuid());
			DeleteRFile(to);
			DoCopyRFile(from, to->GetDirectory(), true, nullptr);
			if (isAct)
				to->Activate();
			return true;
		}
		static JFile* DoCopyRFile(JResourceObject* from, JDirectory* toDir, bool setNewInnderGuid, JFile* existingFile)
		{
			size_t guid = 0;
			std::wstring name;
			if (existingFile == nullptr)
			{
				guid = Core::MakeGuid();
				name = toDir->MakeUniqueFileName(GetDefaultName(from->GetTypeInfo()), from->GetFormat(), from->GetGuid());
			}
			else
				name = toDir->MakeUniqueFileName(existingFile->GetName(), from->GetFormat(), existingFile->GetResourceGuid());

			std::wstring toPath = toDir->GetPath() + L"\\" + name + Core::JFileConstant::GetFileFormat();
			std::wstring toMetaPath = toDir->GetPath() + L"\\" + name + Core::JFileConstant::GetMetaFileFormat();
			std::wifstream fromStream;
			std::wofstream toStream;

			//copy jAsset
			if (setNewInnderGuid)
			{
				std::unordered_map<size_t, size_t> guidMap;
				fromStream.open(from->GetPath(), std::ios::binary | std::ios::in);
				toStream.open(toPath, std::ios::binary | std::ios::out);

				//jasset
				std::wstring guide;
				const std::wstring guidSymbol = Core::JFileConstant::StreamUncopiableGuidSymbol();
				while (getline(fromStream, guide))
				{
					if (JCUtil::Contain(guide, guidSymbol))
					{
						const size_t storedGuid = JCUtil::WstringToInt(guide.substr(guidSymbol.size()));
						auto guidData = guidMap.find(storedGuid);

						if (guidData != guidMap.end())
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
				//jAsset
				std::wifstream fromStream(from->GetPath(), std::ios::binary | std::ios::in);
				std::wofstream toStream(toPath, std::ios::binary | std::ios::out);
				toStream << fromStream.rdbuf();
				fromStream.close();
				toStream.close();
			}

			//copy meta

			fromStream.open(from->GetMetaFilePath(), std::ios::binary | std::ios::in);
			toStream.open(toMetaPath, std::ios::binary | std::ios::out);
			std::wstring guide;
			const std::wstring guidSymbol = Core::JFileConstant::StreamObjGuidSymbol();
			while (getline(fromStream, guide))
			{
				if (JCUtil::Contain(guide, guidSymbol))
					toStream << guidSymbol << guid << '\n';
				else
					toStream << guide << '\n';
			}
			fromStream.close();
			toStream.close();

			if (existingFile == nullptr)
			{
				JFileInitData initData(name, guid, from->GetTypeInfo(), from->GetResourceType(), from->GetFormatIndex());
				CreateJFile(initData, toDir);
			}
			else
			{
				if (existingFile->IsExistingResource())
					existingFile->GetResource()->SetName(name);
				else
				{
					JFileInitData initData(name, guid, from->GetTypeInfo(), from->GetResourceType(), from->GetFormatIndex());
					DestroyJFile(existingFile->GetResourceGuid());
					CreateJFile(initData, toDir);
				}
			}
			return toDir->SearchFile(guid);
		}
	public:
		static void DeleteRFile(JResourceObject* rObj)
		{
			_wremove(rObj->GetPath().c_str());
			_wremove(rObj->GetMetaFilePath().c_str());
			DestroyJFile(rObj->GetGuid());
		}
		static void MoveRFile(JResourceObject* from, JDirectory* toDir)
		{
			if (toDir == nullptr || toDir->GetGuid() == from->impl->directory->GetGuid())
				return;

			const std::wstring prePath = from->GetPath();
			const std::wstring preMethPath = from->GetMetaFilePath();

			DestroyJFile(from->GetGuid());
			from->impl->directory = toDir;
			CreateJFile(JFileInitData{ from }, from->impl->directory);
			ConvertToActFileData(from);

			const std::wstring name = from->impl->directory->MakeUniqueFileName(from->GetName(), from->GetFormat(), from->GetGuid());
			if (name != from->GetName())
				from->SetName(name);

			const std::wstring newPath = from->GetPath();
			const std::wstring newMetaPath = from->GetMetaFilePath();

			MoveFileExW(prePath.c_str(), newPath.c_str(), MOVEFILE_WRITE_THROUGH);
			MoveFileExW(preMethPath.c_str(), newMetaPath.c_str(), MOVEFILE_WRITE_THROUGH);
		}
		static void CreateCacheFile(JResourceObject* rObj)noexcept
		{
			JResourceObject::StoreData storeData(rObj);
			static_cast<JResourceObjectPrivate&>(rObj->GetPrivateInterface()).GetAssetDataIOInterface().StoreAssetData(&storeData);
			if (!RTypeCommonCall::GetRTypeHint(rObj->GetResourceType()).isFixedAssetFile)
				JFileIOHelper::CombineFile(std::vector<std::wstring>{rObj->GetMetaFilePath(), rObj->GetPath()}, GetCacheFilePath(rObj));
			else
				JFileIOHelper::CopyFile(rObj->GetMetaFilePath(), GetCacheFilePath(rObj));
		}
	public: 
		static void ConvertToActFileData(JResourceObject* rObj) noexcept
		{
			JDirectoryPrivate::FileInterface::ConvertToActFileData(rObj);
		}
		static void ConvertToDeActFileData(const size_t guid) noexcept
		{
			JDirectoryPrivate::FileInterface::ConvertToDeActFileData(guid);
		}
		static JFile* CreateJFile(const JFileInitData& initData, JDirectory* owner)
		{
			return JDirectoryPrivate::FileInterface::CreateJFile(initData, owner);
		}
		static bool DestroyJFile(const size_t guid)
		{
			return JDirectoryPrivate::FileInterface::DestroyJFile(guid);
		}
	}; 
	 
	JResourceObject::InitData::InitData(const JTypeInfo& initTypeInfo,
		const uint8 formatIndex,
		const J_RESOURCE_TYPE rType,
		JDirectory* directory)
		:JObject::InitData(initTypeInfo), formatIndex(formatIndex), rType(rType), directory(directory)
	{
		name = directory->MakeUniqueFileName(name, RTypeCommonCall::GetFormat(rType, formatIndex), guid);
	}
	JResourceObject::InitData::InitData(const JTypeInfo& initTypeInfo,
		const size_t guid,
		const uint8 formatIndex,
		const J_RESOURCE_TYPE rType,
		JDirectory* directory)
		: JObject::InitData(initTypeInfo, guid), formatIndex(formatIndex), rType(rType), directory(directory)
	{
		name = directory->MakeUniqueFileName(name, RTypeCommonCall::GetFormat(rType, formatIndex), guid);
	}
	JResourceObject::InitData::InitData(const JTypeInfo& initTypeInfo,
		const std::wstring& name,
		const size_t& guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const J_RESOURCE_TYPE rType,
		JDirectory* directory)
		: JObject::InitData(initTypeInfo, name, guid, flag), formatIndex(formatIndex), rType(rType), directory(directory)
	{
		InitData::name = directory->MakeUniqueFileName(name, GetFormat(), guid);
	} 
	bool JResourceObject::InitData::IsValidData()const noexcept
	{
		return JObject::InitData::IsValidData() && formatIndex != GetInvalidFormatIndex();
	}
	J_RESOURCE_TYPE JResourceObject::InitData::GetResourceType() const noexcept
	{
		return rType;
	}
	std::wstring JResourceObject::InitData::GetFormat()const noexcept
	{
		return RTypeCommonCall::GetFormat(rType, formatIndex);
	}

	JResourceObject::LoadData::LoadData(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
		:directory(directory), pathData(pathData)
	{}
	JResourceObject::LoadData::~LoadData()
	{}
	bool JResourceObject::LoadData::IsValidData()const noexcept
	{
		return directory != nullptr;
	}
 
	JResourceObject::StoreData::StoreData(JResourceObject* jRobj)
		:JObject::StoreData(jRobj)
	{} 

	std::wstring JResourceObject::GetFullName()const noexcept
	{
		return GetName() + GetFormat();
	}
	std::wstring JResourceObject::GetPath()const noexcept
	{
		return impl->directory->GetPath() + L"\\" + GetName() + Core::JFileConstant::GetFileFormat();
	}
	std::wstring JResourceObject::GetMetaFilePath()const noexcept
	{
		return impl->directory->GetPath() + L"\\" + GetName() + Core::JFileConstant::GetMetaFileFormat();
	}
	std::wstring JResourceObject::GetFolderPath()const noexcept
	{
		return impl->directory->GetPath();
	}
	std::wstring JResourceObject::GetDefaultFormat(const J_RESOURCE_TYPE type)noexcept
	{
		return RTypeCommonCall::CallGetAvailableFormat(type)[0];
	}
	J_OBJECT_TYPE JResourceObject::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::RESOURCE_OBJECT;
	}
	uint8 JResourceObject::GetFormatIndex()const noexcept
	{
		return impl->formatIndex;
	}
	uint8 JResourceObject::GetFormatIndex(const J_RESOURCE_TYPE type, const std::wstring& format)noexcept
	{
		std::vector<std::wstring> formatVec = RTypeCommonCall::CallGetAvailableFormat(type);
		const uint formatVecCount = (uint)formatVec.size();
		for (uint i = 0; i < formatVecCount; ++i)
		{
			if (formatVec[i] == format)
				return i;
		}
		return GetInvalidFormatIndex();
	}
	JDirectory* JResourceObject::GetDirectory()const noexcept
	{
		return impl->directory;
	}
	void JResourceObject::SetName(const std::wstring& newName)noexcept
	{
		if (newName == GetName())
			return;

		const std::wstring preMetaPath = GetMetaFilePath();
		const std::wstring prePath = GetPath();
		JObject::SetName(impl->directory->MakeUniqueFileName(newName, GetFormat(), GetGuid()));

		_wrename(preMetaPath.c_str(), GetMetaFilePath().c_str());
		_wrename(prePath.c_str(), GetPath().c_str());
	}
	bool JResourceObject::HasFile()const noexcept
	{
		if (_waccess(GetPath().c_str(), 00) != -1)
			return true;
		else
			return false;
	}
	bool JResourceObject::HasMetafile()const noexcept
	{
		if (_waccess(GetMetaFilePath().c_str(), 00) != -1)
			return true;
		else
			return false;
	}
	bool JResourceObject::CanMakeFile()const noexcept
	{
		return !HasFlag(OBJECT_FLAG_DO_NOT_SAVE);
	}
	bool JResourceObject::IsResourceFormat(const J_RESOURCE_TYPE type, const std::wstring& format)noexcept
	{ 
		std::vector<std::wstring> formatVec = RTypeCommonCall::CallGetAvailableFormat(type);
		return std::count(formatVec.begin(), formatVec.end(), format);
	}
	void JResourceObject::DoActivate() noexcept
	{
		JObject::DoActivate();
	}
	void JResourceObject::DoDeActivate()noexcept
	{
		JObject::DoDeActivate();
	}
	JResourceEventInterface* JResourceObject::EvInterface()noexcept
	{
		return rEv.EvInterface();
	}
	void JResourceObject::RegisterRTypeInfo(const RTypeHint& rTypeHint, const RTypeCommonFunc& rTypeCFunc, const RTypePrivateFunc& rTypePFunc)
	{
		RTypeRegister::RegisterRTypeInfo(rTypeHint, rTypeCFunc, rTypePFunc);
	}
	JResourceObject::JResourceObject(const InitData& initData)
		: JObject(initData), impl(std::make_unique<JResourceObjectImpl>(initData))
	{ }
	JResourceObject::~JResourceObject()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JResourceObjectPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JResourceObjectPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JResourceObjectPrivate::AssetDataIOInterface;
	using FileInterface = JResourceObjectPrivate::FileInterface;
	using DestroyInstanceInterfaceEx = JResourceObjectPrivate::DestroyInstanceInterfaceEx;

	void CreateInstanceInterface::SetValidInstance(Core::JIdentifier* createdPtr)noexcept
	{
		JResourceObject* rObj = static_cast<JResourceObject*>(createdPtr);
		if (rObj->CanMakeFile())
		{
			const bool hasJFile = JDirectory::SearchFile(rObj->GetGuid()) != nullptr;
			if (!hasJFile)
				JResourceObject::JResourceObjectImpl::CreateJFile(JFileInitData{ rObj }, rObj->impl->directory);
			JResourceObject::JResourceObjectImpl::ConvertToActFileData(rObj);
			 
			JResourceObject::StoreData storeData(rObj);
			auto& rPrivate = static_cast<JResourceObjectPrivate&>(rObj->GetPrivateInterface());
			if (!rObj->HasFile())
				rPrivate.GetAssetDataIOInterface().StoreAssetData(&storeData);
			if (!rObj->HasMetafile())
				rPrivate.GetAssetDataIOInterface().StoreMetaData(&storeData);
		}
		//리소스는 생성 후 자원을 초기화한뒤 유효한상태가 된다
		//Has order dependency 
		rObj->Activate();
		TryDestroyUnUseData(createdPtr);

		RTypeHint rTypeHint = RTypeCommonCall::GetRTypeHint(rObj->GetResourceType());
		if (rTypeHint.isFrameResource)
		{
			auto setFrameDirtyCallable = RTypePrivateCall::GetSetFrameDirtyCallable(rObj->GetResourceType());
			setFrameDirtyCallable(nullptr, rObj);

			auto setFrameBuffIndexCallable = RTypePrivateCall::GetSetFrameBuffIndexCallable(rObj->GetResourceType());
			setFrameBuffIndexCallable(nullptr, rObj, rObj->GetTypeInfo().GetInstanceCount());
		}
	}
	void CreateInstanceInterface::TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept{}
	bool CreateInstanceInterface::Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsChildOf(JResourceObject::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JResourceObject::JResourceObjectImpl::DoCopy(static_cast<JResourceObject*>(from), static_cast<JResourceObject*>(to));
	}
 
	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		JResourceObject* rObj = static_cast<JResourceObject*>(ptr);
		auto rTypeHint = RTypeCommonCall::GetRTypeHint(rObj->GetResourceType());
		rEv.NotifyEraseEvent(rObj);	  
		if (rTypeHint.isFrameResource)
		{
			int index = rObj->GetTypeInfo().GetInstanceIndex(rObj->GetGuid());
			auto objVec = rObj->GetTypeInfo().GetInstanceRawPtrVec();
			auto setFrameDirtyCallable = RTypePrivateCall::GetSetFrameDirtyCallable(rObj->GetResourceType());
			JCUtil::ApplyFunc(index, setFrameDirtyCallable, objVec);

			auto setFrameBuffIndexCallable = RTypePrivateCall::GetSetFrameBuffIndexCallable(rObj->GetResourceType());
			JCUtil::ApplyFuncUseIndex(index, setFrameBuffIndexCallable, objVec);
		}
	}
	void DestroyInstanceInterface::SetInvalidInstance(Core::JIdentifier* ptr)noexcept
	{
		JResourceObject* rObj = static_cast<JResourceObject*>(ptr);
		auto rTypeHint = RTypeCommonCall::GetRTypeHint(rObj->GetResourceType());
		const bool hasFile = JDirectory::SearchFile(rObj->GetGuid()) != nullptr;
		const bool canCreateCache = rTypeHint.canKeepDiskFileLife && rTypeHint.canKeepJFileLife;

		//file이 손상된 경우 혹은 리소스를 엔진에서 완전 삭제할시 다시 불러낼 수 없다
		if (hasFile)
		{
			JResourceObject::JResourceObjectImpl::ConvertToDeActFileData(rObj->GetGuid());
			if (canCreateCache && JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			{
				if (JEditorModifedObjectInterface{}.IsModified(rObj->GetGuid()))
				{
					if (!rObj->IsActivated())
					{
						rObj->Activate();
						rObj->impl->CreateCacheFile(rObj);
						rObj->DeActivate();
					}
					else
						rObj->impl->CreateCacheFile(rObj);
				}
			}
		}

		const bool isUndestroyable = rObj->HasFlag(OBJECT_FLAG_UNDESTROYABLE);
		if (!isUndestroyable)
		{
			if (!rTypeHint.canKeepDiskFileLife)
				rObj->impl->DeleteRFile(rObj);	//완전한 resource 파괴
			else if (!rTypeHint.canKeepJFileLife)
				JResourceObject::JResourceObjectImpl::DestroyJFile(rObj->GetGuid());	//JFile을 통한 Load를 막고 새로운 resource 생성만 유효하게 한다		
		}

		//clear data
		if (rObj->IsActivated())
			rObj->DeActivate();
	}
	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateLoadAssetDIData(JDirectory* owner, const Core::JAssetFileLoadPathData& pathData)
	{
		return std::make_unique<JResourceObject::LoadData>(owner, pathData);
	}
	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateStoreAssetDIDate(JResourceObject* rObj)
	{
		return std::make_unique<JResourceObject::StoreData>(rObj);
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadCommonMetaData(std::wifstream& stream, Core::JDITypeDataBase* data, const bool closeSream)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JResourceObject::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		 
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto rInit = static_cast<JResourceObject::InitData*>(data);

		JFileIOHelper::LoadObjectIden(stream, rInit->guid, rInit->flag);
		std::wstring guide;
		std::wstring format;
		int rType;
		int formatIndex;

		stream >> guide >> rType;
		stream >> guide >> format;
		stream >> guide >> formatIndex; 
		if (closeSream)
			stream.close();
		rInit->rType = (J_RESOURCE_TYPE)rType;
		rInit->formatIndex = formatIndex;

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreCommonMetaData(std::wofstream& stream, Core::JDITypeDataBase* data, const bool closeSream)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JResourceObject::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto rStore = static_cast<JResourceObject::StoreData*>(data);
		JResourceObject* rObj = static_cast<JResourceObject*>(rStore->obj);
		 
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
		JFileIOHelper::StoreObjectIden(stream, rObj);

		stream << Core::JFileConstant::StreamTypeSymbol<J_RESOURCE_TYPE>() << (int)rObj->GetResourceType() << '\n';
		stream << Core::JFileConstant::StreamFormatSymbol() << rObj->GetFormat() << '\n';
		stream << Core::JFileConstant::StreamFormatIndexSymbol() << rObj->GetFormatIndex() << '\n';
		if (closeSream)
			stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	JFile* FileInterface::CopyJFile(JResourceObject* from, JDirectory* toDir, bool setNewInnderGuid)noexcept
	{
		return JResourceObject::JResourceObjectImpl::DoCopyRFile(from, toDir, setNewInnderGuid, nullptr);
	}
	void FileInterface::MoveFile(JResourceObject* rObj, JDirectory* toDir)noexcept
	{
		JResourceObject::JResourceObjectImpl::MoveRFile(rObj, toDir);
	}
	void FileInterface::DeleteFile(JResourceObject* rObj)noexcept
	{
		JResourceObject::JResourceObjectImpl::DeleteRFile(rObj);
		JResourceObject::BeginForcedDestroy(rObj);
	}

	void DestroyInstanceInterfaceEx::BeginForcedDestroy(JResourceObject* rObj)noexcept
	{
		JResourceObject::BeginForcedDestroy(rObj);
	}

	Core::JIdentifierPrivate::DestroyInstanceInterface& JResourceObjectPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}