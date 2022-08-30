#include"JAnimationController.h"
#include"../Skeleton/JSkeletonAsset.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMDiagram.h"  
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMtransition.h"
#include"../../../Core/FSM/JFSMstate.h" 
#include"../../../Core/FSM/JFSMcondition.h" 
#include"../../../Core/FSM/JFSMconditionStorage.h"  
#include"../../../Core/FSM/JFSMLoadGuidMap.h"
#
#include"../../../Application/JApplicationVariable.h"
#include"../../../Editor/Diagram/JEditorDiagram.h" 
#include"../../../Editor/Diagram/JEditorDiagramNode.h" 
#include"../../../Utility/JCommonUtility.h"

//수정필요 
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstateClip.h" 
#include"../JResourceManager.h"

namespace JinEngine
{
	using namespace Core;
	using namespace Graphic;

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
			if (fsmDiagram[i]->HasAnimationData())
				fsmDiagram[i]->Update(animationtimes[i], animationShaderData, modelSkeleton, animationConstatns, i);
		}
	}
	J_RESOURCE_TYPE JAnimationController::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JAnimationController::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::wstring> JAnimationController::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".controller" };
		return format;
	}
	uint JAnimationController::GetAnimationDiagramCount()const noexcept
	{
		return (uint)fsmDiagram.size();
	}
	std::wstring JAnimationController::GetAnimationDiagramName(const size_t diagramGuid) noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		return diagarm != nullptr ? diagarm->GetName() : L"Nullptr";
	}
	std::wstring JAnimationController::GetUniqueDiagramName(const std::wstring& initName)noexcept
	{
		return JCommonUtility::MakeUniqueName(fsmDiagram, initName);
	}
	std::wstring JAnimationController::GetUniqueStateName(const size_t diagramGuid, const std::wstring& initName)noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		return diagarm->GetStateUniqueName(initName);
	}
	std::wstring JAnimationController::GetUniqueConditionName(const std::wstring& initName)noexcept
	{
		return conditionStorage->GetConditionUniqueName(initName);
	}
	uint JAnimationController::GetConditionCount()const noexcept
	{
		return conditionStorage->GetConditionCount();
	}
	void JAnimationController::SetConditionName(const size_t guid, const std::wstring& newName)noexcept
	{
		conditionStorage->GetCondition(guid)->SetName(newName);
	}
	void JAnimationController::SetConditionValueType(const size_t guid, const J_FSMCONDITION_VALUE_TYPE type)noexcept
	{
		conditionStorage->GetCondition(guid)->SetValueType(type); 
	}
	bool JAnimationController::CreateAnimationDiagram(const std::wstring& name)noexcept
	{
		if (fsmDiagram.size() >= diagramMaxCount)
			return false;

		fsmDiagram.push_back(std::make_unique<JAnimationFSMdiagram>(GetUniqueDiagramName(name), Core::MakeGuid(), conditionStorage.get()));
		return true;
	}
	bool JAnimationController::CreateAnimationClipState(const std::wstring& name, const size_t diagramGuid)noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		if (diagarm == nullptr)
			return false;

		JAnimationFSMstate* state = diagarm->CreateAnimationClipState(name);
		if (state != nullptr)
			return true;
		else
			return false;
	}
	bool JAnimationController::CreateTransition(const size_t intputStateGuid, const size_t outputStateGuid, const size_t diagramGuid)
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		if (diagarm == nullptr)
			return false;

		return diagarm->CreateAnimationTransition(intputStateGuid, outputStateGuid);
	}
	bool JAnimationController::CreateCondition(const std::wstring& name)noexcept
	{
		JFSMcondition* newCondition = conditionStorage->AddCondition(name);
		return newCondition != nullptr;
	}
	bool JAnimationController::DestroyAnimationDiagram(const size_t diagramGuid)noexcept
	{
		const uint dCount = (uint)fsmDiagram.size();
		for (uint i = 0; i < dCount; ++i)
		{
			if (fsmDiagram[i]->GetGuid() == diagramGuid)
			{
				fsmDiagram.erase(fsmDiagram.begin() + i);
				return true;
			}
		}
		return false;
	}
	bool JAnimationController::DestroyAnimationState(const size_t statGuid, const size_t diagramGuid)noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		if (diagarm == nullptr)
			return false;

		return diagarm->DestroyAnimationState(statGuid);
	}
	bool JAnimationController::DestroyCondition(const size_t conditionGuid)noexcept
	{
		return conditionStorage->RemoveCondition(conditionGuid);
	}
	JAnimationControllerEditInterface* JAnimationController::EditorInterface()
	{
		return this;
	}
	Core::JAnimationFSMdiagram* JAnimationController::GetDiagram(const size_t guid)noexcept
	{
		return FindDiagram(guid);
	}
	std::vector<Core::JAnimationFSMdiagram*> JAnimationController::GetDiagramVec()noexcept
	{
		std::vector<Core::JAnimationFSMdiagram*> diagramCash;
		const uint digramCount = (uint)fsmDiagram.size();
		diagramCash.resize(digramCount);
		for (uint i = 0; i < digramCount; ++i)
			diagramCash[i] = fsmDiagram[i].get();
		return diagramCash;
	}
	Core::JAnimationFSMstate* JAnimationController::GetState(const size_t diagramGuid, const size_t stateGuid) noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		if (diagarm == nullptr)
			return nullptr;

		return diagarm->GetState(stateGuid);
	}
	std::vector<Core::JAnimationFSMstate*>& JAnimationController::GetStateVec(const size_t diagramGuid)noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		if (diagarm == nullptr)
		{
			std::vector<Core::JAnimationFSMstate*> nll;
			return nll;
		}

		return diagarm->GetStateVec();
	}
	Core::JFSMcondition* JAnimationController::GetCondition(const size_t guid) noexcept
	{
		return conditionStorage->GetCondition(guid);
	}
	Core::JAnimationFSMdiagram* JAnimationController::FindDiagram(const size_t guid)noexcept
	{
		const uint dCount = (uint)fsmDiagram.size();
		for (uint i = 0; i < dCount; ++i)
		{
			if (fsmDiagram[i]->GetGuid() == guid)
				return fsmDiagram[i].get();
		}
		return nullptr;
	}
	bool JAnimationController::Copy(JObject* ori)
	{
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;

		if (typeInfo.IsA(ori->GetTypeInfo()))
		{
			JAnimationController* oriC = static_cast<JAnimationController*>(ori);
			CopyRFile(*oriC, *this);
			ClearResource();
			StuffResource(); 
			return true;
		}
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
			if(ReadAnimationControllerData())
				SetValid(true);
		}
	}
	void JAnimationController::ClearResource()
	{
		if (IsValid())
		{
			conditionStorage.reset();
			fsmDiagram.clear();
			SetValid(false);
		}
	}
	bool JAnimationController::ReadAnimationControllerData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);

		if (stream.is_open())
		{
			JFSMLoadGuidMap guidMap;
			size_t storeGuid;
			stream >> storeGuid;

			if (storeGuid != GetGuid())
				guidMap.isNewGuid = true;
			else
				guidMap.isNewGuid = false;

			conditionStorage->LoadIdentifierData(stream, guidMap);

			uint diagramCount = 0;
			stream >> diagramCount;

			for (uint i = 0; i < diagramCount; ++i)
			{ 
				std::unique_ptr<Core::JAnimationFSMdiagram> newDiagram = Core::JAnimationFSMdiagram::LoadIdentifierData(stream, guidMap, conditionStorage.get());
				if (newDiagram != nullptr)
					fsmDiagram.push_back(std::move(newDiagram));
			}

			conditionStorage->LoadContentsData(stream, guidMap);
			for (uint i = 0; i < diagramCount; ++i)
				fsmDiagram[i]->LoadContentsData(stream, guidMap);

			stream.close();
			return true;
		}
		else
			return false;
	}
	bool JAnimationController::IsValid()const noexcept
	{
		return JValidInterface::IsValid();
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

		if (((int)cont->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(cont->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT storeMetaRes = JResourceObject::StoreMetadata(stream, cont);
		stream.close();

		if (storeMetaRes != Core::J_FILE_IO_RESULT::SUCCESS)
			return storeMetaRes;

		stream.open(cont->GetPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			//For classify copy object call
			stream << cont->GetGuid() << '\n';
			cont->conditionStorage->StoreIdentifierData(stream);

			const uint diagramCount = (uint)cont->fsmDiagram.size();
			stream << diagramCount << '\n';

			for (uint i = 0; i < diagramCount; ++i)
				cont->fsmDiagram[i]->StoreIdentifierData(stream);

			cont->conditionStorage->StoreContentsData(stream);
			for (uint i = 0; i < diagramCount; ++i)
				cont->fsmDiagram[i]->StoreContentsData(stream);

			stream.close();
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JAnimationController* JAnimationController::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		if (!JResourceObject::IsResourceFormat<JAnimationController>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JAnimationController* newCont = nullptr;
		if (directory->HasFile(pathData.fullName))
			newCont = JResourceManager::Instance().GetResourceByPath<JAnimationController>(pathData.wstrPath);

		if (newCont == nullptr)
		{
			if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(pathData.name,
					metadata.guid,
					(J_OBJECT_FLAG)metadata.flag,
					directory,
					GetFormatIndex<JAnimationController>(pathData.format));
				newCont = ownerPtr.Get();
				AddInstance(std::move(ownerPtr));
			}
			else
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(pathData.name,
					Core::MakeGuid(),
					J_OBJECT_FLAG::OBJECT_FLAG_NONE,
					directory,
					GetFormatIndex<JAnimationController>(pathData.format));
				newCont = ownerPtr.Get();
				AddInstance(std::move(ownerPtr));
			}
		}

		if (newCont->IsValid())
			return newCont;
		else if (newCont->ReadAnimationControllerData())
		{
			newCont->SetValid(true);
			return newCont;
		}
		else
		{
			newCont->SetIgnoreUndestroyableFlag(true);
			newCont->BeginDestroy();
			return nullptr;
		}
	}
	void JAnimationController::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* directory) ->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(directory->MakeUniqueFileName(GetDefaultName<JAnimationController>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				directory,
				JResourceObject::GetDefaultFormatIndex());

			JResourceObject* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto initC = [](const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(name, guid, objFlag, directory, formatIndex);
			JResourceObject* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationController>(directory->MakeUniqueFileName(ori->GetName()),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JAnimationClip>(ori->GetFormat()));

			JAnimationController* newController = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			newController->Copy(ori);
			return newController;
		};
		JRFI<JAnimationController>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JAnimationController>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JAnimationController::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JAnimationController::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JAnimationController::JAnimationController(const std::string& name, size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JAnimationControllerInterface(name, guid, flag, directory, formatIndex)
	{
		conditionStorage = std::make_unique<Core::JFSMconditionStorage>();
		fsmDiagram.push_back(std::make_unique<JAnimationFSMdiagram>("BaseLayer", conditionStorage.get()));
	}
	JAnimationController::~JAnimationController() {}
}