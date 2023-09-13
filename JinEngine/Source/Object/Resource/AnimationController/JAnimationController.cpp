#include"JAnimationController.h"
#include"JAnimationControllerPrivate.h"
#include"FSM/JAnimationFSMdiagram.h"  
#include"FSM/JAnimationFSMdiagramPrivate.h"
#include"FSM/JAnimationFSMstate.h" 
#include"FSM/JAnimationFSMstateClip.h" 
#include"FSM/JAnimationTime.h" 
#include"FSM/JAnimationFSMtransition.h"
#include"FSM/JAnimationUpdateData.h"  
#include"../JResourceObjectHint.h"
#include"../JClearableInterface.h"
#include"../Skeleton/JSkeletonAsset.h"  
#include"../../Directory/JDirectory.h"
#include"../../Directory/JFile.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/FSM/JFSMparameter.h" 
#include"../../../Core/FSM/JFSMparameterStorage.h"   
#include"../../../Core/FSM/JFSMownerInterface.h" 
#include"../../../Core/File/JFileIOHelper.h" 
#include"../../../Core/Utility/JCommonUtility.h"

#include"../../../Application/JApplicationProject.h" 
#include"../../../Graphic/Frameresource/JAnimationConstants.h" 

//수정필요  
#include<fstream>

namespace JinEngine
{ 
	using namespace Graphic;
	namespace
	{
		using DiagramIOInterface = JAnimationFSMdiagramPrivate::AssetDataIOInterface;
		using DiagramUpdateInterface = JAnimationFSMdiagramPrivate::UpdateInterface;
	}
	namespace
	{
		static JAnimationControllerPrivate aPrivate;
	}
 
	class JAnimationController::JAnimationControllerImpl : public Core::JTypeImplBase,
		public JClearableInterface, 
		public Core::JFSMdiagramOwnerInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationControllerImpl)
	public:
		JWeakPtr<JAnimationController> thisPointer = nullptr;
	public:
		JOwnerPtr<Core::JFSMparameterStorage> paramStorage;
		std::vector<JUserPtr<JAnimationFSMdiagram>> diagramVec;
	public:
		JAnimationControllerImpl(const InitData& initData, JAnimationController* thisPointer)
		{}
		~JAnimationControllerImpl()
		{}
	public:
		Core::JFSMparameterStorageUserAccess* GetParameterStorageUser()noexcept final
		{
			return paramStorage.Get();
		}
	public:
		Core::JUserPtr<JAnimationFSMdiagram> FindDiagram(const size_t guid)noexcept
		{
			const uint dCount = (uint)diagramVec.size();
			for (uint i = 0; i < dCount; ++i)
			{
				if (diagramVec[i]->GetGuid() == guid)
					return diagramVec[i];
			}
			return nullptr;
		}
	public:
		void Initialize(JAnimationUpdateData* updateData)noexcept
		{ 
			uint layerSize = (uint)diagramVec.size();
			for (uint i = 0; i < layerSize; ++i)
			{
				DiagramUpdateInterface::Initialize(diagramVec[i], updateData, i);
				DiagramUpdateInterface::Enter(diagramVec[i], updateData, i);
			}
		}
		void Update(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept
		{
			bool hasValidValue = false;
			uint layerSize = (uint)diagramVec.size();
			for (uint i = 0; i < layerSize; ++i)
			{
				if (diagramVec[i]->GetStateCount() > 0)
				{
					DiagramUpdateInterface::Update(diagramVec[i], updateData, constant, i);
					hasValidValue = true;
				}
			}
			if (!hasValidValue)
				constant.StuffIdentity();
		}
	public:
		void StuffResource()
		{
			if (!thisPointer->IsValid())
			{
				if (ReadAssetData())
					thisPointer->SetValid(true);
			}
		}
		void ClearResource()
		{
			if (thisPointer->IsValid())
			{
				std::vector<JUserPtr<JAnimationFSMdiagram>> copy = diagramVec;
				const uint fsmCount = (uint)copy.size();
				for (uint i = 0; i < fsmCount; ++i)
					Core::JFSMinterface::BeginDestroy(copy[i].Get());

				diagramVec.clear();
				paramStorage->Clear();
				thisPointer->SetValid(false);
			}
		}
	public:
		bool ReadAssetData()
		{
			std::wifstream stream;
			stream.open(thisPointer->GetPath(), std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return false;
			 
			Core::JFSMparameterStorage::LoadData(stream, paramStorage);

			uint diagramCount = 0;
			JFileIOHelper::LoadAtomicData(stream, diagramCount);
			for (uint i = 0; i < diagramCount; ++i)
				DiagramIOInterface::LoadAssetData(stream, this); 

			stream.close();
			return true;
		}
		bool WriteAssetData()
		{
			std::wofstream stream;
			stream.open(thisPointer->GetPath(), std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

			//For classify copy object call 
			Core::JFSMparameterStorage::StoreData(stream, paramStorage);

			JFileIOHelper::StoreAtomicData(stream, L"DiagramCount:", diagramVec.size());
			const uint diagramCount = (uint)diagramVec.size();
			for (uint i = 0; i < diagramCount; ++i)
				DiagramIOInterface::StoreAssetData(stream, diagramVec[i]);

			stream.close();
			return true;
		}
	public:
		static std::unique_ptr<JAnimationController::InitData> CreateLoadAssetInitData(const JUserPtr<JDirectory>& dir)
		{
			auto unq = std::make_unique<JAnimationController::InitData>(JResourceObject::GetDefaultFormatIndex(), dir);
			unq->makeDiagram = false;
			return std::move(unq);
		}
	public:
		bool RegisterDiagram(JUserPtr<Core::JFSMdiagram> diagram)noexcept final
		{	 
			if (diagram != nullptr)
			{
				int index = JCUtil::GetTypeIndex(diagramVec, diagram->GetGuid());
				if (index == -1 && diagram->GetTypeInfo().IsA(JAnimationFSMdiagram::StaticTypeInfo()))
				{
					diagram->SetName(JCUtil::MakeUniqueName(diagramVec, diagram->GetName()));
					diagramVec.push_back(JUserPtr<JAnimationFSMdiagram>::ConvertChild(std::move(diagram)));
					return true;
				}
			}
			return false;
		}
		bool DeRegisterDiagram(JUserPtr<Core::JFSMdiagram> diagram)noexcept final
		{
			if (diagram != nullptr)
			{
				int index = JCUtil::GetTypeIndex(diagramVec, diagram->GetGuid());
				if (index != -1)
				{
					diagramVec.erase(diagramVec.begin() + index);
					return true;
				}
			}
			return false;
		}
	public:
		void NotifyReAlloc()
		{
			for (auto& data : diagramVec)
				SetOwnerPointer(data, this);
		}
	public:
		void Initialize(InitData* initData)
		{
			if (initData->makeDiagram)
				thisPointer->CreateFSMdiagram(Core::MakeGuid());
			thisPointer->SetValid(true);
		}
		void RegisterThisPointer(JAnimationController* aniCont)
		{
			thisPointer = Core::GetWeakPtr(aniCont);
			paramStorage = Core::JPtrUtil::MakeOwnerPtr<Core::JFSMparameterStorage>(thisPointer);
		}
		static void RegisterTypeData()
		{
			auto getFormatIndexLam = [](const std::wstring& format)
			{
				return JResourceObject::GetFormatIndex(GetStaticResourceType(),format);
			};

			static GetRTypeInfoCallable getTypeInfoCallable{ &JAnimationController::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JAnimationController::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false, true, false };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JAnimationController::StaticTypeInfo(), aPrivate);

			IMPL_REALLOC_BIND(JAnimationController::JAnimationControllerImpl, thisPointer)

			NotifyReAllocPtr notifyParamReAllocPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
			{
				auto movedParamStorage = static_cast<Core::JFSMparameterStorage*>(movedPtr);
				JAnimationController* cond = static_cast<JAnimationController*>(movedParamStorage->GetOwner().Get());
				cond->impl->paramStorage.Release();
				cond->impl->paramStorage.Reset(movedParamStorage);
			};
			auto paramReAllocF = std::make_unique<NotifyReAllocF>(notifyParamReAllocPtr);
			std::unique_ptr<JAllocationDesc> paramAllocDesc = std::make_unique<JAllocationDesc>();
			paramAllocDesc->notifyReAllocB = UniqueBind(std::move(paramReAllocF), static_cast<ReceiverPtr>(nullptr), JinEngine::Core::empty, JinEngine::Core::empty);
			Core::JFSMparameterStorage::StaticTypeInfo().SetAllocationOption(std::move(paramAllocDesc));
		}
	};

	JAnimationController::InitData::InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory)
		:JResourceObject::InitData(JAnimationController::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory)
	{}
	JAnimationController::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory)
		: JResourceObject::InitData(JAnimationController::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory)
	{}
	JAnimationController::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory)
		: JResourceObject::InitData(JAnimationController::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JAnimationController::PrivateInterface()const noexcept
	{
		return aPrivate;
	}
	J_RESOURCE_TYPE JAnimationController::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JAnimationController::GetFormat()const noexcept
	{
		return GetAvailableFormat()[GetFormatIndex()];
	}
	std::vector<std::wstring> JAnimationController::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".controller" };
		return format;
	}
	uint JAnimationController::GetDiagramCount()const noexcept
	{
		return (uint)impl->diagramVec.size();
	}
	uint JAnimationController::GetParameterCount()const noexcept
	{
		return impl->paramStorage->GetParameterCount();
	}  
	JUserPtr<JAnimationFSMdiagram> JAnimationController::GetDiagram(const size_t guid)noexcept
	{
		int index = JCUtil::GetTypeIndex(impl->diagramVec, guid);
		if (index != -1)
			return impl->diagramVec[index];
		else
			return nullptr;
	}
	JUserPtr<JAnimationFSMdiagram> JAnimationController::GetDiagramByIndex(const uint index)noexcept
	{
		if (impl->diagramVec.size() <=index)
			return nullptr;
		return impl->diagramVec[index];
	}
	const std::vector<JUserPtr<JAnimationFSMdiagram>>& JAnimationController::GetDiagramVec()noexcept
	{
		return impl->diagramVec;
	}
	JUserPtr<Core::JFSMparameter> JAnimationController::GetParameter(const size_t guid)noexcept
	{
		return impl->paramStorage->GetParameter(guid);
	}
	JUserPtr<Core::JFSMparameter> JAnimationController::GetParameterByIndex(const uint index)noexcept
	{
		return impl->paramStorage->GetParameterByIndex(index);
	}
	bool JAnimationController::CanCreateDiagram()const noexcept
	{
		return impl->diagramVec.size() < JAnimationFixedData::fsmDiagramMaxCount;
	}
	bool JAnimationController::CanCreateParameter()const noexcept
	{
		return impl->paramStorage->GetParameterCount() < impl->paramStorage->maxNumberOfParameter;
	}
	bool JAnimationController::CanCreateState(JAnimationFSMdiagram* diagram)const noexcept
	{ 
		return diagram && diagram->CanCreateState();
	}
	bool JAnimationController::IsValid()const noexcept
	{
		return Core::JValidInterface::IsValid();
	}
	JUserPtr<JAnimationFSMdiagram> JAnimationController::CreateFSMdiagram(const size_t guid)noexcept
	{
		if (!CanCreateDiagram())
			return nullptr;
		 
		return JICI::Create<JAnimationFSMdiagram>(GetDefaultName<JAnimationFSMdiagram>(), guid, impl.get());
	}
	JUserPtr<Core::JFSMparameter> JAnimationController::CreateFSMparameter(const size_t guid)noexcept
	{
		if (!CanCreateParameter())
			return nullptr;

		return JICI::Create<Core::JFSMparameter>(GetDefaultName<Core::JFSMparameter>(), guid, impl->paramStorage);
	}
	JUserPtr<JAnimationFSMstate> JAnimationController::CreateFSMclip(const JUserPtr<JAnimationFSMdiagram>& diagram, const size_t guid)noexcept
	{
		if (!CanCreateState(diagram.Get()))
			return nullptr;
		 
		if (GetDiagram(diagram->GetGuid())->GetGuid() != diagram->GetGuid())
			return nullptr;
 
		return JICI::Create<JAnimationFSMstateClip>(GetDefaultName<JAnimationFSMstateClip>(), guid, diagram);
	}
	JUserPtr<JAnimationFSMtransition> JAnimationController::CreateFsmtransition(const JUserPtr<JAnimationFSMdiagram>& diagram,
		const JUserPtr<JAnimationFSMstate>& from,
		const JUserPtr<JAnimationFSMstate>& to,
		const size_t guid)noexcept
	{
		if (from == nullptr || to == nullptr)
			return nullptr;

		if (GetDiagram(diagram->GetGuid())->GetGuid() != diagram->GetGuid())
			return nullptr;

		if (diagram->GetState(from->GetGuid())->GetGuid() != from->GetGuid() ||
			diagram->GetState(to->GetGuid())->GetGuid() != to->GetGuid())
			return nullptr;

		return JICI::Create<JAnimationFSMtransition>(GetDefaultName<JAnimationFSMtransition>(), guid, from,to);
	}
	void JAnimationController::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		impl->StuffResource();
	}
	void JAnimationController::DoDeActivate()noexcept
	{ 
		impl->ClearResource();
		JResourceObject::DoDeActivate();
	} 
	JAnimationController::JAnimationController(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JAnimationControllerImpl>(initData, this))
	{  }
	JAnimationController::~JAnimationController()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimationControllerPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JAnimationControllerPrivate::AssetDataIOInterface; 
	using FrameUpdateInterface = JAnimationControllerPrivate::FrameUpdateInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JAnimationController>(*static_cast<JAnimationController::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JAnimationController* cont = static_cast<JAnimationController*>(createdPtr);
		cont->impl->RegisterThisPointer(cont);
		cont->impl->Initialize(static_cast<JAnimationController::InitData*>(initData));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationController::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationController::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JAnimationController::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;

		auto initData = JAnimationController::JAnimationControllerImpl::CreateLoadAssetInitData(directory);
		if (LoadMetaData(pathData.engineMetaFileWPath, initData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JAnimationController> newCont; 
		if (directory->HasFile(initData->guid))
			newCont = Core::GetUserPtr<JAnimationController>(JAnimationController::StaticTypeInfo().TypeGuid(), initData->guid);

		if (newCont == nullptr)
		{
			initData->name = pathData.name; 
			auto idenUser = aPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &aPrivate);
			newCont.ConnnectChild(idenUser);
		} 
		newCont->impl->ReadAssetData();
		return newCont;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationController::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JAnimationController::StoreData*>(data);
		if (!storeData->HasCorrectType(JAnimationController::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JAnimationController> cont;
		cont.ConnnectChild(storeData->obj); 
		return cont->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationController::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JAnimationController::InitData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationController::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JAnimationController::StoreData*>(data);
		Core::JUserPtr<JAnimationController> cont;
		cont.ConnnectChild(storeData->obj); 

		std::wofstream stream;
		stream.open(cont->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void FrameUpdateInterface::Initialize(JAnimationController* aniCont, JAnimationUpdateData* updateData)noexcept
	{
		aniCont->impl->Initialize(updateData);
	}
	void FrameUpdateInterface::Update(JAnimationController* aniCont, JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept
	{
		aniCont->impl->Update(updateData, constant);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationControllerPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JAnimationControllerPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JAnimationControllerPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}

}