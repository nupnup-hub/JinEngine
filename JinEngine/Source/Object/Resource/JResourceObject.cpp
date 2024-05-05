#include"JResourceObject.h"   
#include"JResourceObjectPrivate.h"
#include "JResourceObjectHint.h" 
#include "JResourceManager.h" 
#include"JResourceObjectEventDesc.h"
#include"../Directory/JFile.h"
#include"../Directory/JFileInitData.h"
#include"../Directory/JDirectory.h" 
#include"../Directory/JDirectoryPrivate.h"
#include"../JObjectFileIOHelper.h"
#include"../JObjectModifyInterface.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../../Core/File/JFileConstant.h" 
#include"../../Core/File/JFileIOHelper.h" 
#include"../../Application/Engine/JApplicationEngine.h"
#include"../../Application/Project/JApplicationProject.h"
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
		void NotifyUpdateEvent(JResourceObject* rObj, std::unique_ptr<JResourceEventDesc>&& desc = nullptr)
		{
			NotifyEvent(rObj->GetGuid(), J_RESOURCE_EVENT_TYPE::UPDATE_NON_FRAME_RESOURCE, rObj, desc.get());
		}
		void NotifyEraseEvent(JResourceObject* rObj, std::unique_ptr<JResourceEventDesc>&& desc = nullptr)
		{
			NotifyEvent(rObj->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE, rObj, desc.get());
		}
	};
	namespace
	{
		static JResourceObjectEvent rEv;
	}

	class JResourceObject::JResourceObjectImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JResourceObjectImpl)
	public:
		JWeakPtr<JResourceObject> thisPointer;
	public:
		JUserPtr<JDirectory> directory;
		const uint8 formatIndex;
	public:
		JResourceObjectImpl(const InitData& initData)
			:formatIndex(initData.formatIndex), directory(initData.directory)
		{}
	public:
		static std::wstring GetCacheFilePath(JResourceObject* rObj) noexcept
		{ 
			return JApplicationProject::ModResourceCachePath() + L"\\" + std::to_wstring(rObj->GetGuid()) + Core::JFileConstant::GetCacheFileFormatW();
		}
	public:
		static bool DoCopy(const JUserPtr<JResourceObject>& from, const JUserPtr<JResourceObject>& to)
		{
			bool isAct = to->IsActivated();
			if (isAct)
				to->DeActivate();

			//DestroyJFile(to->GetGuid());
			DeleteRFile(to.Get());
			DoCopyRFile(from, to->GetDirectory(), true, nullptr);
			if (isAct)
				to->Activate();
			return true;
		}
		static JUserPtr<JFile> DoCopyRFile(const JUserPtr<JResourceObject>& from, const JUserPtr<JDirectory>& toDir, bool setNewInnderGuid, JFile* existingFile)
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

			std::wstring toPath = toDir->GetPath() + L"\\" + name + Core::JFileConstant::GetFileFormatW();
			std::wstring toMetaPath = toDir->GetPath() + L"\\" + name + Core::JFileConstant::GetMetaFileFormatW();
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
				const std::wstring guidSymbol = Core::JFileConstant::GetUncopiableGuidSymbolW();
				while (getline(fromStream, guide))
				{
					if (JCUtil::Contain(guide, guidSymbol))
					{
						const uint index = (uint)guide.find(guidSymbol);
						const size_t storedGuid = JCUtil::WstringToInt(JCUtil::EraseSideWChar(guide.substr(index), L' '));
						auto guidData = guidMap.find(storedGuid);

						if (guidData != guidMap.end())
							toStream<< guide.substr(0, index + guidSymbol.size()) << guidData->second << '\n';
						else
						{
							const size_t newGuid = Core::MakeGuid();
							guidMap.emplace(storedGuid, newGuid);
							toStream << guide.substr(0, index + guidSymbol.size()) << newGuid << '\n';
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
			const std::wstring guidSymbol = Core::JFileConstant::GetObjGuidSymbolW();
			while (getline(fromStream, guide))
			{
				if (JCUtil::Contain(guide, guidSymbol))
				{
					const uint index = (uint)guide.find(guidSymbol);
					toStream << guide.substr(0, index + guidSymbol.size()) << guid << '\n';
				}
				else
					toStream << guide << '\n';
			}
			fromStream.close();
			toStream.close();

			if (existingFile == nullptr)
			{
				JFileInitData initData(name, guid, from->GetTypeInfo(), from->GetResourceType(), from->GetFlag(), from->GetFormatIndex());
				CreateJFile(initData, toDir);
			}
			else
			{
				if (existingFile->IsExistingResource())
					existingFile->GetResource()->SetName(name);
				else
				{
					JFileInitData initData(name, guid, from->GetTypeInfo(), from->GetResourceType(), from->GetFlag(), from->GetFormatIndex());
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
		static void MoveRFile(const JUserPtr<JResourceObject>& from, const JUserPtr<JDirectory>& toDir)
		{
			if (toDir == nullptr || toDir->GetGuid() == from->impl->directory->GetGuid())
				return;

			const std::wstring prePath = from->GetPath();
			const std::wstring preMethPath = from->GetMetaFilePath();

			DestroyJFile(from->GetGuid());
			from->impl->directory = toDir;
			CreateJFile(JFileInitData{ from }, from->impl->directory);
			from->impl->ConvertToActFileData();

			const std::wstring name = from->impl->directory->MakeUniqueFileName(from->GetName(), from->GetFormat(), from->GetGuid());
			if (name != from->GetName())
				from->SetName(name);

			const std::wstring newPath = from->GetPath();
			const std::wstring newMetaPath = from->GetMetaFilePath();

			MoveFileExW(prePath.c_str(), newPath.c_str(), MOVEFILE_WRITE_THROUGH);
			MoveFileExW(preMethPath.c_str(), newMetaPath.c_str(), MOVEFILE_WRITE_THROUGH);
		}
		void CreateCacheFile()noexcept
		{
			JResourceObject::StoreData storeData(thisPointer);
			static_cast<JResourceObjectPrivate&>(thisPointer->PrivateInterface()).GetAssetDataIOInterface().StoreAssetData(&storeData);
			if (!RTypeCommonCall::GetRTypeHint(thisPointer->GetResourceType()).isFixedAssetFile)
				JObjectFileIOHelper::CombineFile(std::vector<std::wstring>{thisPointer->GetMetaFilePath(), thisPointer->GetPath()}, GetCacheFilePath(thisPointer.Get()));
			else
				JObjectFileIOHelper::CopyFile(thisPointer->GetMetaFilePath(), GetCacheFilePath(thisPointer.Get()));
		}
	public: 
		void ConvertToActFileData() noexcept
		{
			JDirectoryPrivate::FileInterface::ConvertToActFileData(thisPointer);
		}
		void ConvertToDeActFileData() noexcept
		{
			JDirectoryPrivate::FileInterface::ConvertToDeActFileData(thisPointer->GetGuid());
		}
		static JUserPtr<JFile> CreateJFile(const JFileInitData& initData, const JUserPtr<JDirectory>& owner)
		{
			return JDirectoryPrivate::FileInterface::CreateJFile(initData, owner);
		}
		static bool DestroyJFile(const size_t guid)
		{
			return JDirectoryPrivate::FileInterface::DestroyJFile(guid);
		}
	public:
		void RegisterThisPointer(JResourceObject* rObj)
		{
			thisPointer = Core::GetUserPtr(rObj);
		}
		static void RegisterTypeData()
		{
			IMPL_REALLOC_BIND(JResourceObject::JResourceObjectImpl, thisPointer)
		}
	}; 
	 
	JResourceObject::InitData::InitData(const JTypeInfo& initTypeInfo,
		const uint8 formatIndex,
		const J_RESOURCE_TYPE rType,
		const JUserPtr<JDirectory>& directory)
		:JObject::InitData(initTypeInfo), formatIndex(formatIndex), rType(rType), directory(directory)
	{
		name = directory->MakeUniqueFileName(name, RTypeCommonCall::GetFormat(rType, formatIndex), guid);
	}
	JResourceObject::InitData::InitData(const JTypeInfo& initTypeInfo,
		const size_t guid,
		const uint8 formatIndex,
		const J_RESOURCE_TYPE rType,
		const JUserPtr<JDirectory>& directory)
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
		const JUserPtr<JDirectory>& directory)
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

	JResourceObject::LoadData::LoadData(const JUserPtr<JDirectory>& directory, const Core::JAssetFilePathData& pathData)
		:directory(directory), pathData(pathData)
	{}
	JResourceObject::LoadData::~LoadData()
	{}
	bool JResourceObject::LoadData::IsValidData()const noexcept
	{
		return directory != nullptr;
	}
 
	JResourceObject::StoreData::StoreData(const JUserPtr<JResourceObject>& jRobj)
		:JObject::StoreData(jRobj)
	{} 

	std::wstring JResourceObject::GetFullName()const noexcept
	{
		return GetName() + GetFormat();
	}
	std::wstring JResourceObject::GetPath()const noexcept
	{
		return impl->directory->GetPath() + L"\\" + GetName() + Core::JFileConstant::GetFileFormatW();
	}
	std::wstring JResourceObject::GetMetaFilePath()const noexcept
	{
		return impl->directory->GetPath() + L"\\" + GetName() + Core::JFileConstant::GetMetaFileFormatW();
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
	JUserPtr<JDirectory> JResourceObject::GetDirectory()const noexcept
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
	using EventInterface = JResourceObjectPrivate::EventInterface;

	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JResourceObject* rObj = static_cast<JResourceObject*>(createdPtr);
		rObj->impl->RegisterThisPointer(rObj);
	}
	void CreateInstanceInterface::SetValidInstance(Core::JIdentifier* createdPtr)noexcept
	{
		JResourceObject* rObj = static_cast<JResourceObject*>(createdPtr);
		if (rObj->CanMakeFile())
		{
			const bool hasJFile = JDirectory::SearchFile(rObj->GetGuid()) != nullptr;
			if (!hasJFile)
				JResourceObject::JResourceObjectImpl::CreateJFile(JFileInitData{ rObj }, rObj->impl->directory);
			rObj->impl->ConvertToActFileData(); 
			 
			JResourceObject::StoreData storeData(Core::GetUserPtr(rObj));
			auto& rPrivate = static_cast<JResourceObjectPrivate&>(rObj->PrivateInterface());
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
		}
	}
	void CreateInstanceInterface::TryDestroyUnUseData(Core::JIdentifier* createdPtr)noexcept{}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsChildOf(JResourceObject::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JResourceObject::JResourceObjectImpl::DoCopy(Core::ConvertChildUserPtr<JResourceObject>(from), Core::ConvertChildUserPtr<JResourceObject>(to));
	}
 
	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		JResourceObject* rObj = static_cast<JResourceObject*>(ptr);
		rEv.NotifyEraseEvent(rObj, nullptr);	  
		auto rTypeHint = RTypeCommonCall::GetRTypeHint(rObj->GetResourceType());
		if (rTypeHint.isFrameResource)
		{
			int index = rObj->GetTypeInfo().GetInstanceIndex(rObj->GetGuid());
			auto objVec = rObj->GetTypeInfo().GetInstanceRawPtrVec();
			auto setFrameDirtyCallable = RTypePrivateCall::GetSetFrameDirtyCallable(rObj->GetResourceType());
			JCUtil::ApplyFunc(index, setFrameDirtyCallable, objVec);
		}		
		JObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
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
			rObj->impl->ConvertToDeActFileData(); 
			if (canCreateCache && JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			{
				if (JModifedObjectInterface{}.IsModifiedAndStoreAble(rObj->GetGuid()))
				{
					if (!rObj->IsActivated())
					{
						rObj->Activate();
						rObj->impl->CreateCacheFile();
						rObj->DeActivate();
					}
					else
						rObj->impl->CreateCacheFile();
				}
			}
		}

		const bool isUndestroyable = rObj->HasFlag(OBJECT_FLAG_UNDESTROYABLE);
		if (!isUndestroyable)
		{
			if (!rTypeHint.canKeepDiskFileLife)
			{
				rObj->impl->DeleteRFile(rObj);	//완전한 resource 파괴
				JModifedObjectInterface{}.RemoveInfo(rObj->GetGuid());
			}
			else if (!rTypeHint.canKeepJFileLife)
			{
				JResourceObject::JResourceObjectImpl::DestroyJFile(rObj->GetGuid());	//JFile을 통한 Load를 막고 새로운 resource 생성만 유효하게 한다		
				JModifedObjectInterface{}.RemoveInfo(rObj->GetGuid());
			}
		}
		//clear data
		if (rObj->IsActivated())
			rObj->DeActivate();
	}
	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateLoadAssetDIData(const JUserPtr<JDirectory>& owner, const Core::JAssetFilePathData& pathData)
	{
		return std::make_unique<JResourceObject::LoadData>(owner, pathData);
	}
	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateStoreAssetDIDate(const JUserPtr<JResourceObject>& rObj)
	{
		return std::make_unique<JResourceObject::StoreData>(rObj);
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadCommonMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data, const bool canClose)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JResourceObject::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		if (!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto rInit = static_cast<JResourceObject::InitData*>(data);
		std::wstring guide;
		std::wstring format;

		JObjectFileIOHelper::LoadObjectIden(tool, rInit->guid, rInit->flag);
		JObjectFileIOHelper::LoadEnumData(tool, rInit->rType, Core::JFileConstant::GetTypeSymbol<J_RESOURCE_TYPE>());
		JObjectFileIOHelper::LoadJString(tool, format, Core::JFileConstant::GetFormatSymbol());
		JObjectFileIOHelper::LoadAtomicData(tool, rInit->formatIndex, Core::JFileConstant::GetFormatIndexSymbol());
		
		if (canClose)
			tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreCommonMetaData(JFileIOTool& tool, Core::JDITypeDataBase* data, const bool canClose)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JResourceObject::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto rStore = static_cast<JResourceObject::StoreData*>(data);
		JUserPtr<JResourceObject> rUser;
		rUser.ConnnectChild(rStore->obj);
 
		JObjectFileIOHelper::StoreObjectIden(tool, rUser.Get());
		JObjectFileIOHelper::StoreEnumData(tool, rUser->GetResourceType(), Core::JFileConstant::GetTypeSymbol<J_RESOURCE_TYPE>());
		JObjectFileIOHelper::StoreJString(tool, rUser->GetFormat(), Core::JFileConstant::GetFormatSymbol());
		JObjectFileIOHelper::StoreAtomicData(tool, rUser->GetFormatIndex(), Core::JFileConstant::GetFormatIndexSymbol());

		if (canClose)
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	} 

	JUserPtr<JFile> FileInterface::CopyJFile(const JUserPtr<JResourceObject>& from, const JUserPtr<JDirectory>& toDir, bool setNewInnderGuid)noexcept
	{
		return JResourceObject::JResourceObjectImpl::DoCopyRFile(from, toDir, setNewInnderGuid, nullptr);
	}
	void FileInterface::MoveFile(const JUserPtr<JResourceObject>& rObj, const JUserPtr<JDirectory>& toDir)noexcept
	{
		JResourceObject::JResourceObjectImpl::MoveRFile(rObj, toDir);
	}
	/**
	* DeleteFile 호출자는 함수호출이후 JResourceObject Destroy에 대한 책임을 진다.
	*/
	void FileInterface::DeleteFile(JResourceObject* rObj)noexcept
	{
		JResourceObject::JResourceObjectImpl::DeleteRFile(rObj);
		//함수 호출자에게 책임을 전가한다.
		//JIdentifier 들은 보통 BeginDestroy함수를 통해 삭제가 되며
		//JResourceObject는 특수하게 resource가 없어져도 engine내부에 file, hardware disk에 data가 존재하며
		//일반적인 경우 JResourceObject만 삭제하면 되지만 engine을 통해 hardware disk상에서 data를 지워야 하는 특수한경우가 존재한다.
		//BeginDestroy에 매개변수를 통해 처리해도 되지만 JResourceObject계통만에 특수한 경우고 
		//DeleteFile 호출하는 clsss가 JWindowDirectory(Editor)뿐이므로 우선은 따로 JWindowDirectory 책임을 지고 올바른 호출을 하는 방향으로
		//구성한다. 
		//추후 BeginDestroy에 여러 조건이 필요해지는 경우 수청한다
		//JResourceObject::BeginForcedDestroy(rObj);
	}

	void DestroyInstanceInterfaceEx::BeginForcedDestroy(JResourceObject* rObj)noexcept
	{
		JResourceObject::BeginForcedDestroy(rObj);
	}

	void EventInterface::NotifyEvent(JResourceObject* rObj, const J_RESOURCE_EVENT_TYPE type, std::unique_ptr<JResourceEventDesc>&& desc)
	{
		if (rObj == nullptr)
			return;

		switch (type)
		{
		case JinEngine::J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE:
		{
			rEv.NotifyEraseEvent(rObj, std::move(desc));
			break;
		}
		case JinEngine::J_RESOURCE_EVENT_TYPE::UPDATE_NON_FRAME_RESOURCE:
		{
			rEv.NotifyUpdateEvent(rObj, std::move(desc));
			break;
		}
		default:
			break;
		}
	}

	Core::JIdentifierPrivate::DestroyInstanceInterface& JResourceObjectPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}