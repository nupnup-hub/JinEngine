#include"JAnimationController.h"
#include"../Skeleton/JSkeletonAsset.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMDiagram.h"  
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMtransition.h"
#include"../../../Core/FSM/JFSMfactory.h"
#include"../../../Core/FSM/JFSMstate.h" 
#include"../../../Core/FSM/JFSMparameter.h" 
#include"../../../Core/FSM/JFSMparameterStorage.h"   
#include"../../../Core/File/JFileIOHelper.h" 

#include"../../../Application/JApplicationVariable.h"
#include"../../../Editor/Diagram/JEditorDiagram.h" 
#include"../../../Editor/Diagram/JEditorDiagramNode.h" 
#include"../../../Utility/JCommonUtility.h"

//수정필요 
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstateClip.h" 
#include"../JResourceManager.h"
#include<fstream>

namespace JinEngine
{
	using namespace Core;
	using namespace Graphic;

	JAnimationController::JAnimationControllerInitData::JAnimationControllerInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex)
		:JResourceInitData(name, guid, flag, directory, formatIndex)
	{}
	JAnimationController::JAnimationControllerInitData::JAnimationControllerInitData(const std::wstring& name, JDirectory* directory, const uint8 formatIndex)
		: JResourceInitData(name, directory, formatIndex)
	{}
	JAnimationController::JAnimationControllerInitData::JAnimationControllerInitData(JDirectory* directory, const uint8 formatIndex)
		: JResourceInitData(GetDefaultName<JAnimationController>(), directory, formatIndex)
	{}

	J_RESOURCE_TYPE JAnimationController::JAnimationControllerInitData::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::ANIMATION_CONTROLLER;
	}

	void JAnimationController::Initialize(std::vector<JAnimationTime>& animationtimes, JSkeletonAsset* srcSkeletonAsset)noexcept
	{
		animationShaderData.Initialize();

		uint layerSize = (uint)fsmDiagram.size();
		for (uint i = 0; i < layerSize; ++i)
		{
			fsmDiagram[i]->Initialize(animationShaderData, srcSkeletonAsset);
			fsmDiagram[i]->Enter(animationtimes[i], animationShaderData, srcSkeletonAsset);
		}
	}
	void JAnimationController::Update(std::vector<JAnimationTime>& animationtimes, JSkeletonAsset* modelSkeleton, JAnimationConstants& animationConstatns)noexcept
	{
		uint layerSize = (uint)fsmDiagram.size();
		for (uint i = 0; i < layerSize; ++i)
		{
			if (fsmDiagram[i]->HasNowState())
				fsmDiagram[i]->Update(animationtimes[i], animationShaderData, modelSkeleton, animationConstatns, i);
		}
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
		return (uint)fsmDiagram.size();
	}
	uint JAnimationController::GetParameterCount()const noexcept
	{
		return paramStorage->GetParameterCount();
	}  
	Core::JAnimationFSMdiagram* JAnimationController::GetDiagram(const size_t guid)noexcept
	{
		int index = JCUtil::GetJIdenIndex(fsmDiagram, guid);
		if (index != -1)
			return fsmDiagram[index];
		else
			return nullptr;
	}
	Core::JAnimationFSMdiagram* JAnimationController::GetDiagramByIndex(const uint index)noexcept
	{
		if (fsmDiagram.size() <=index)
			return nullptr;
		return fsmDiagram[index];
	}
	const std::vector<Core::JAnimationFSMdiagram*>& JAnimationController::GetDiagramVec()noexcept
	{
		return fsmDiagram;
	}
	Core::JFSMparameter* JAnimationController::GetParameter(const size_t guid)noexcept
	{
		return paramStorage->GetParameter(guid);
	}
	Core::JFSMparameter* JAnimationController::GetParameterByIndex(const uint index)noexcept
	{
		return paramStorage->GetParameterByIndex(index);
	}
	bool JAnimationController::CanCreateDiagram()const noexcept
	{
		return fsmDiagram.size() < diagramMaxCount;
	}
	bool JAnimationController::CanCreateCondition()const noexcept
	{
		return paramStorage->GetParameterCount() < paramStorage->maxNumberOfParameter;
	}
	bool JAnimationController::CanCreateState(Core::JAnimationFSMdiagram* diagram)const noexcept
	{ 
		return diagram && diagram->CanCreateState();
	}
	Core::JAnimationFSMdiagram* JAnimationController::CreateFSMDiagram(const size_t guid)noexcept
	{
		if (!CanCreateDiagram())
			return nullptr;
		 
		return Core::JFFI<Core::JAnimationFSMdiagram>::Create(Core::JPtrUtil::MakeOwnerPtr<Core::JFSMdiagram::InitData>(guid, this));
	}
	Core::JFSMparameter* JAnimationController::CreateFSMCondition(const size_t guid)noexcept
	{
		if (!CanCreateCondition())
			return nullptr;

		return Core::JFFI<Core::JFSMparameter>::Create(Core::JPtrUtil::MakeOwnerPtr<Core::JFSMparameter::InitData>(guid, paramStorage.get()));
	}
	Core::JAnimationFSMstate* JAnimationController::CreateFSMClip(Core::JAnimationFSMdiagram* diagram, const size_t guid)noexcept
	{
		if (!CanCreateState(diagram))
			return nullptr;
		 
		if (GetDiagram(diagram->GetGuid())->GetGuid() != diagram->GetGuid())
			return nullptr;

		return Core::JFFI<Core::JAnimationFSMstateClip>::Create(Core::JPtrUtil::MakeOwnerPtr<Core::JFSMstate::InitData>(guid, Core::GetUserPtr(diagram)));
	}
	Core::JAnimationFSMtransition* JAnimationController::CreateFsmTransition(Core::JAnimationFSMdiagram* diagram,
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

		return JFFI<JAnimationFSMtransition>::Create(Core::JPtrUtil::MakeOwnerPtr<JAnimationFSMtransition::InitData>(GetDefaultName<JFSMtransition>(),
			MakeGuid(),
			to->GetGuid(),
			Core::GetUserPtr(from)));
	}
	bool JAnimationController::AddType(Core::JFSMdiagram* diagram)noexcept
	{
		if (diagram != nullptr)
		{
			int index = JCUtil::GetJIdenIndex(fsmDiagram, diagram->GetGuid());
			if (index == -1 && diagram->GetTypeInfo().IsA(JAnimationFSMdiagram::StaticTypeInfo()))
			{
				diagram->SetName(JCUtil::MakeUniqueName(fsmDiagram, diagram->GetName()));
				fsmDiagram.push_back(static_cast<JAnimationFSMdiagram*>(diagram));
				return true;
			}
		}
		return false;
	}
	bool JAnimationController::RemoveType(Core::JFSMdiagram* diagram)noexcept
	{
		if (diagram != nullptr)
		{
			int index = JCUtil::GetJIdenIndex(fsmDiagram, diagram->GetGuid());
			if (index != -1)
			{
				fsmDiagram.erase(fsmDiagram.begin() + index);
				return true;
			}
		}
		return false;
	}
	Core::JFSMparameterStorageUserAccess* JAnimationController::GetConditionStorageUser()noexcept
	{
		return paramStorage.get();
	}
	Core::JAnimationFSMdiagram* JAnimationController::FindDiagram(const size_t guid)noexcept
	{
		const uint dCount = (uint)fsmDiagram.size();
		for (uint i = 0; i < dCount; ++i)
		{
			if (fsmDiagram[i]->GetGuid() == guid)
				return fsmDiagram[i];
		}
		return nullptr;
	}
	void JAnimationController::DoCopy(JObject* ori)
	{
		JAnimationController* oriC = static_cast<JAnimationController*>(ori);
		CopyRFile(*oriC, true);
		ClearResource();
		StuffResource();
	}
	void JAnimationController::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
	}
	void JAnimationController::DoDeActivate()noexcept
	{ 
		JResourceObject::DoDeActivate();
		ClearResource();
	}
	void JAnimationController::StuffResource()
	{
		if (!IsValid())
		{
			if (ReadAnimationControllerData())
				SetValid(true);
		}
	}
	void JAnimationController::ClearResource()
	{
		if (IsValid())
		{
			std::vector<Core::JAnimationFSMdiagram*> copy = fsmDiagram;
			const uint fsmCount = GetDiagramCount();
			for (uint i = 0; i < fsmCount; ++i)
				JFSMInterface::Destroy(copy[i]);

			fsmDiagram.clear();
			paramStorage->Clear(); 
			SetValid(false);
		}
	}
	bool JAnimationController::WriteAnimationControllerData()
	{
		std::wofstream stream;
		stream.open(GetPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			//For classify copy object call 
			paramStorage->StoreData(stream);

			JFileIOHelper::StoreAtomicData(stream, L"DiagramCount:", fsmDiagram.size());
			const uint diagramCount = (uint)fsmDiagram.size();
			for (uint i = 0; i < diagramCount; ++i)
				fsmDiagram[i]->StoreData(stream);
			  
			stream.close();
			return true;
		}
		else
			return false;
	}
	bool JAnimationController::ReadAnimationControllerData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);

		if (stream.is_open())
		{
			paramStorage->LoadData(stream);

			uint diagramCount = 0;
			JFileIOHelper::LoadAtomicData(stream, diagramCount);
			for (uint i = 0; i < diagramCount; ++i)
				JAnimationFSMdiagram::LoadData(stream, this);

			stream.close();
			return true;
		}
		else
			return false;
	}
	bool JAnimationController::IsValid()const noexcept
	{
		return Core::JValidInterface::IsValid();
	}
	Core::J_FILE_IO_RESULT JAnimationController::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JAnimationController::StoreObject(JAnimationController* cont)
	{
		//수정필요
		if (cont == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (cont->HasFlag(OBJECT_FLAG_DO_NOT_SAVE))
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(cont->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT storeMetaRes = JResourceObject::StoreMetadata(stream, cont);
		stream.close();

		if (storeMetaRes != Core::J_FILE_IO_RESULT::SUCCESS)
			return storeMetaRes;

		if (cont->WriteAnimationControllerData())
			return Core::J_FILE_IO_RESULT::SUCCESS;
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JAnimationController* JAnimationController::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JResourceMetaData metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JAnimationController* newCont = nullptr;
		if (directory->HasFile(pathData.name))
			newCont = JResourceManager::Instance().GetResourceByPath<JAnimationController>(pathData.engineFileWPath);

		if (newCont == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			JAnimationControllerInitData initdata{ pathData.name, metadata.guid,metadata.flag, directory, (uint8)metadata.formatIndex };
			if (initdata.IsValidLoadData())
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(initdata);
				newCont = ownerPtr.Get();
				if (!AddInstance(std::move(ownerPtr)))
					return nullptr;
			}
		}
		 
		return newCont;
	}
	void JAnimationController::RegisterJFunc()
	{
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData>initdata) ->JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::ANIMATION_CONTROLLER && initdata->IsValidCreateData())
			{
				JAnimationControllerInitData* aInitdata = static_cast<JAnimationControllerInitData*>(initdata.Get());
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(*aInitdata);
				JAnimationController* newAniCont = ownerPtr.Get();
				if (AddInstance(std::move(ownerPtr)))
				{
					Core::JAnimationFSMdiagram* baseDiagram = newAniCont->CreateFSMDiagram(Core::MakeGuid());
					baseDiagram->SetName(L"BaseLayer");
					StoreObject(newAniCont);				 
					newAniCont->SetValid(true);
					newAniCont->ClearResource();
					return newAniCont;
				}		 
			}
			return nullptr;
		};
		auto loadC = [](JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(InitData(ori->GetName(),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JAnimationController>(ori->GetFormat())));

			JAnimationController* newController = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newController->Copy(ori);
				return newController;
			}
			else
				return nullptr;
		};
		JRFI<JAnimationController>::Register(defaultC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JAnimationController>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JAnimationController::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JAnimationController::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false};
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JAnimationController::JAnimationController(const JAnimationControllerInitData& initdata)
		:JAnimationControllerInterface(initdata)
	{ 
		paramStorage = std::make_unique<Core::JFSMparameterStorage>(); 
	}
	JAnimationController::~JAnimationController()
	{ 
	}
}