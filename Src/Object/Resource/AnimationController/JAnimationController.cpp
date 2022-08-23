#include"JAnimationController.h"
#include"../Skeleton/JSkeletonAsset.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMDiagram.h"  
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h" 
#include"../../../Core/FSM/JFSMstate.h" 
#include"../../../Core/FSM/JFSMcondition.h" 
#include"../../../Core/FSM/JFSMconditionStorage.h"  

#include"../../../Application/JApplicationVariable.h"
#include"../../../Editor/Diagram/JEditorDiagram.h" 
#include"../../../Editor/Diagram/JEditorDiagramNode.h" 
#include"../../../Utility/JCommonUtility.h"

//수정필요
//포트폴리오용 임시코드
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
	std::string JAnimationController::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::string> JAnimationController::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".controller" };
		return format;
	}
	uint JAnimationController::GetAnimationDiagramCount()const noexcept
	{
		return (uint)fsmDiagram.size();
	}
	std::string JAnimationController::GetAnimationDiagramName(const size_t diagramGuid) noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		return diagarm != nullptr ? diagarm->GetName() : "Nullptr";
	}
	std::string JAnimationController::GetUniqueDiagramName(const std::string& initName)noexcept
	{
		return JCommonUtility::MakeUniqueName(fsmDiagram, initName);
	}
	std::string JAnimationController::GetUniqueStateName(const size_t diagramGuid, const std::string& initName)noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		return diagarm->GetStateUniqueName(initName);
	}
	std::string JAnimationController::GetUniqueConditionName(const std::string& initName)noexcept
	{
		return conditionStorage->GetConditionUniqueName(initName);
	}
	uint JAnimationController::GetConditionCount()const noexcept
	{
		return conditionStorage->GetConditionCount();
	}
	void JAnimationController::SetConditionName(const size_t guid, const std::string& newName)noexcept
	{
		conditionStorage->SetConditionName(guid, newName);
	}
	void JAnimationController::SetConditionValueType(const size_t guid, const J_FSMCONDITION_VALUE_TYPE type)noexcept
	{
		conditionStorage->SetConditionValueType(guid, type);
	}
	bool JAnimationController::CreateAnimationDiagram(const std::string& name, const size_t guid)noexcept
	{
		if (fsmDiagram.size() >= diagramMaxCount)
			return false;

		fsmDiagram.push_back(std::make_unique<JAnimationFSMdiagram>(GetUniqueDiagramName(name), guid, conditionStorage.get()));
		return true;
	}
	bool JAnimationController::CreateAnimationClipState(const std::string& name, const size_t clipGuid, const size_t diagramGuid)noexcept
	{
		JAnimationFSMdiagram* diagarm = FindDiagram(diagramGuid);
		if (diagarm == nullptr)
			return false;

		JAnimationFSMstate* state = diagarm->CreateAnimationClipState(name, clipGuid);
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
	bool JAnimationController::CreateCondition(const std::string& name, const size_t guid)noexcept
	{
		JFSMcondition* newCondition = conditionStorage->AddCondition(name, guid);
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
		SetValid(true);
	}
	void JAnimationController::ClearResource()
	{
		SetValid(false);
	}
	bool JAnimationController::IsValid()const noexcept
	{
		return JValidInterface::IsValid();
	}
	Core::J_FILE_IO_RESULT JAnimationController::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JAnimationController::StoreObject(JAnimationController* animationCont)
	{
		//수정필요
		if (animationCont == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;
		if (((int)animationCont->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JAnimationController* JAnimationController::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		//수정필요
		return nullptr;
	}
	void JAnimationController::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JAnimationController(owner->MakeUniqueFileName(GetDefaultName<JAnimationController>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JAnimationController(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JAnimationController*>(ori)->CopyResource();
		};
		JRFI<JAnimationController>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JAnimationController>(format); };

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
		conditionStorage = std::make_unique<JFSMconditionStorage>();
		fsmDiagram.push_back(std::make_unique<JAnimationFSMdiagram>("BaseLayer", conditionStorage.get()));
	}
	JAnimationController::~JAnimationController() {}
}