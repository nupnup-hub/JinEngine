#include"JAnimationController.h"
#include"JAnimationControllerPrivate.h"
#include"../JResourceObjectHint.h"
#include"../JClearableInterface.h"
#include"../Skeleton/JSkeletonAsset.h"  
#include"../../Directory/JDirectory.h"
#include"../../Directory/JFile.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMdiagram.h"  
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMdiagramPrivate.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMtransition.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationUpdateData.h" 
#include"../../../Core/FSM/JFSMstate.h" 
#include"../../../Core/FSM/JFSMparameter.h" 
#include"../../../Core/FSM/JFSMparameterStorage.h"   
#include"../../../Core/FSM/JFSMownerInterface.h" 
#include"../../../Core/File/JFileIOHelper.h" 

#include"../../../Application/JApplicationProject.h"
#include"../../../Editor/Diagram/JEditorDiagram.h"  
#include"../../../Graphic/FrameResource/JAnimationConstants.h"
#include"../../../Utility/JCommonUtility.h"

//수정필요 
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstateClip.h"  
#include<fstream>

namespace JinEngine
{
	using namespace Core;
	using namespace Graphic;

	namespace
	{
		using DiagramIOInterface = Core::JAnimationFSMdiagramPrivate::AssetDataIOInterface;
		using DiagramUpdateInterface = Core::JAnimationFSMdiagramPrivate::UpdateInterface;
	}
	namespace
	{
		static JAnimationControllerPrivate aPrivate;
	}
 
	class JAnimationController::JAnimationControllerImpl : public Core::JIdentifierImplBase,
		public JClearableInterface, 
		public Core::JFSMdiagramOwnerInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationControllerImpl)
	public:
		JAnimationController* thisCont = nullptr;
	public:
		std::unique_ptr<Core::JFSMparameterStorage> paramStorage;
		std::vector<Core::JAnimationFSMdiagram*> diagramVec;
	public:
		JAnimationControllerImpl(const InitData& initData, JAnimationController* thisCont)
			:thisCont(thisCont)
		{
			paramStorage = std::make_unique<Core::JFSMparameterStorage>();
		}
		~JAnimationControllerImpl()
		{}
	public:
		void Initialize(const InitData& initData)
		{
			if (initData.makeDiagram)
				thisCont->CreateFSMdiagram(Core::MakeGuid());
			thisCont->SetValid(true);
		}
	public:
		Core::JFSMparameterStorageUserAccess* GetParameterStorageUser()noexcept final
		{
			return paramStorage.get();
		}
		Core::JAnimationFSMdiagram* FindDiagram(const size_t guid)noexcept
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
		void Initialize(Core::JAnimationUpdateData* updateData)noexcept
		{ 
			uint layerSize = (uint)diagramVec.size();
			for (uint i = 0; i < layerSize; ++i)
			{
				DiagramUpdateInterface::Initialize(diagramVec[i], updateData, i);
				DiagramUpdateInterface::Enter(diagramVec[i], updateData, i);
			}
		}
		void Update(Core::JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept
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
			if (!thisCont->IsValid())
			{
				if (ReadAssetData())
					thisCont->SetValid(true);
			}
		}
		void ClearResource()
		{
			if (thisCont->IsValid())
			{
				std::vector<Core::JAnimationFSMdiagram*> copy = diagramVec;
				const uint fsmCount = (uint)copy.size();
				for (uint i = 0; i < fsmCount; ++i)
					JFSMinterface::BeginDestroy(copy[i]);

				diagramVec.clear();
				paramStorage->Clear();
				thisCont->SetValid(false);
			}
		}
	public:
		bool ReadAssetData()
		{
			std::wifstream stream;
			stream.open(thisCont->GetPath(), std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return false;
			 
			paramStorage->LoadData(stream);

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
			stream.open(thisCont->GetPath(), std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

			//For classify copy object call 
			paramStorage->StoreData(stream);

			JFileIOHelper::StoreAtomicData(stream, L"DiagramCount:", diagramVec.size());
			const uint diagramCount = (uint)diagramVec.size();
			for (uint i = 0; i < diagramCount; ++i)
				DiagramIOInterface::StoreAssetData(stream, diagramVec[i]);

			stream.close();
			return true;
		}
	public:
		static std::unique_ptr<JAnimationController::InitData> CreateLoadAssetInitData(JDirectory* dir)
		{
			auto unq = std::make_unique<JAnimationController::InitData>(JResourceObject::GetDefaultFormatIndex(), dir);
			unq->makeDiagram = false;
			return std::move(unq);
		}
	public:
		bool RegisterDiagram(Core::JFSMdiagram* diagram)noexcept final
		{	 
			if (diagram != nullptr)
			{
				int index = JCUtil::GetJIdenIndex(diagramVec, diagram->GetGuid());
				if (index == -1 && diagram->GetTypeInfo().IsA(JAnimationFSMdiagram::StaticTypeInfo()))
				{
					diagram->SetName(JCUtil::MakeUniqueName(diagramVec, diagram->GetName()));
					diagramVec.push_back(static_cast<JAnimationFSMdiagram*>(diagram));
					return true;
				}
			}
			return false;
		}
		bool DeRegisterDiagram(Core::JFSMdiagram* diagram)noexcept final
		{
			if (diagram != nullptr)
			{
				int index = JCUtil::GetJIdenIndex(diagramVec, diagram->GetGuid());
				if (index != -1)
				{
					diagramVec.erase(diagramVec.begin() + index);
					return true;
				}
			}
			return false;
		}
	public:
		static void RegisterCallOnce()
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
		}
	};

	JAnimationController::InitData::InitData(const uint8 formatIndex, JDirectory* directory)
		:JResourceObject::InitData(JAnimationController::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory)
	{}
	JAnimationController::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		JDirectory* directory)
		: JResourceObject::InitData(JAnimationController::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory)
	{}
	JAnimationController::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		JDirectory* directory)
		: JResourceObject::InitData(JAnimationController::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory)
	{}

	Core::JIdentifierPrivate& JAnimationController::GetPrivateInterface()const noexcept
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
	Core::JAnimationFSMdiagram* JAnimationController::GetDiagram(const size_t guid)noexcept
	{
		int index = JCUtil::GetJIdenIndex(impl->diagramVec, guid);
		if (index != -1)
			return impl->diagramVec[index];
		else
			return nullptr;
	}
	Core::JAnimationFSMdiagram* JAnimationController::GetDiagramByIndex(const uint index)noexcept
	{
		if (impl->diagramVec.size() <=index)
			return nullptr;
		return impl->diagramVec[index];
	}
	const std::vector<Core::JAnimationFSMdiagram*>& JAnimationController::GetDiagramVec()noexcept
	{
		return impl->diagramVec;
	}
	Core::JFSMparameter* JAnimationController::GetParameter(const size_t guid)noexcept
	{
		return impl->paramStorage->GetParameter(guid);
	}
	Core::JFSMparameter* JAnimationController::GetParameterByIndex(const uint index)noexcept
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
	bool JAnimationController::CanCreateState(Core::JAnimationFSMdiagram* diagram)const noexcept
	{ 
		return diagram && diagram->CanCreateState();
	}
	bool JAnimationController::IsValid()const noexcept
	{
		return Core::JValidInterface::IsValid();
	}
	Core::JAnimationFSMdiagram* JAnimationController::CreateFSMdiagram(const size_t guid)noexcept
	{
		if (!CanCreateDiagram())
			return nullptr;
		 
		return JICI::Create<Core::JAnimationFSMdiagram>(GetDefaultName<Core::JAnimationFSMdiagram>(), guid, impl.get());
	}
	Core::JFSMparameter* JAnimationController::CreateFSMparameter(const size_t guid)noexcept
	{
		if (!CanCreateParameter())
			return nullptr;

		return JICI::Create<Core::JFSMparameter>(GetDefaultName<Core::JFSMparameter>(), guid, impl->paramStorage.get());
	}
	Core::JAnimationFSMstate* JAnimationController::CreateFSMclip(Core::JAnimationFSMdiagram* diagram, const size_t guid)noexcept
	{
		if (!CanCreateState(diagram))
			return nullptr;
		 
		if (GetDiagram(diagram->GetGuid())->GetGuid() != diagram->GetGuid())
			return nullptr;
 
		return JICI::Create<Core::JAnimationFSMstateClip>(GetDefaultName<Core::JAnimationFSMstateClip>(), guid, Core::GetUserPtr(diagram));
	}
	Core::JAnimationFSMtransition* JAnimationController::CreateFsmtransition(Core::JAnimationFSMdiagram* diagram,
		Core::JAnimationFSMstate* from,
		Core::JAnimationFSMstate* to,
		const size_t guid)noexcept
	{
		if (from == nullptr || to == nullptr)
			return nullptr;

		if (GetDiagram(diagram->GetGuid())->GetGuid() != diagram->GetGuid())
			return nullptr;

		if (diagram->GetState(from->GetGuid())->GetGuid() != from->GetGuid() ||
			diagram->GetState(to->GetGuid())->GetGuid() != to->GetGuid())
			return nullptr;

		return JICI::Create<JAnimationFSMtransition>(GetDefaultName<JFSMtransition>(),
			guid,
			Core::GetUserPtr(from),
			Core::GetUserPtr(to));
	}
	void JAnimationController::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		impl->StuffResource();
	}
	void JAnimationController::DoDeActivate()noexcept
	{ 
		JResourceObject::DoDeActivate();
		impl->ClearResource();
	} 
	JAnimationController::JAnimationController(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JAnimationControllerImpl>(initData, this))
	{ 
		impl->Initialize(initData);
	}
	JAnimationController::~JAnimationController()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimationControllerPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JAnimationControllerPrivate::AssetDataIOInterface; 
	using FrameUpdateInterface = JAnimationControllerPrivate::FrameUpdateInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JAnimationController>(*static_cast<JAnimationController::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimationController::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimationController::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JAnimationController::LoadData*>(data);
		auto pathData = loadData->pathData;
		JDirectory* directory = loadData->directory; 

		auto initData = JAnimationController::JAnimationControllerImpl::CreateLoadAssetInitData(directory);
		if (LoadMetaData(pathData.engineMetaFileWPath, initData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JAnimationController* newCont = nullptr;
		if (directory->HasFile(initData->guid))
			newCont = static_cast<JAnimationController*>(Core::GetUserPtr(JAnimationController::StaticTypeInfo().TypeGuid(), initData->guid).Get());

		if (newCont == nullptr)
		{
			initData->name = pathData.name; 
			auto rawPtr = aPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &aPrivate);
			newCont = static_cast<JAnimationController*>(rawPtr);
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

		JAnimationController* clip = static_cast<JAnimationController*>(storeData->obj);
		return clip->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
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
		JAnimationController* cont = static_cast<JAnimationController*>(storeData->obj);

		std::wofstream stream;
		stream.open(cont->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void FrameUpdateInterface::Initialize(JAnimationController* aniCont, Core::JAnimationUpdateData* updateData)noexcept
	{
		aniCont->impl->Initialize(updateData);
	}
	void FrameUpdateInterface::Update(JAnimationController* aniCont, Core::JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept
	{
		aniCont->impl->Update(updateData, constant);
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimationControllerPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JAnimationControllerPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}

}