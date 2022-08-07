#include"JAnimationController.h"
#include"../Skeleton/JSkeletonAsset.h" 
#include"../JResourceObjectFactory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMDiagram.h"  
#include"../../../Core/FSM/AnimationFSM/JAnimationFSMstate.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h" 
#include"../../../Core/FSM/JFSMstate.h" 
#include"../../../Core/FSM/JFSMcondition.h" 
#include"../../../Core/FSM/JFSMconditionStorage.h"  

#include"../../../Application/JApplicationVariable.h"
#include"../../../Window/Editor/Diagram/EditorDiagram.h" 
#include"../../../Window/Editor/Diagram/EditorDiagramNode.h" 
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

		uint layerSize = (uint)stateDiagram.size();
		for (uint i = 0; i < layerSize; ++i)
		{
			stateDiagram[i]->Initialize(animationShaderData, srcSkeletonAsset);
			stateDiagram[i]->Enter(animationtimes[i], animationShaderData, srcSkeletonAsset);
		}
	}
	void JAnimationController::Update(std::vector<JAnimationTime>& animationtimes, JSkeletonAsset* modelSkeleton, JAnimationConstants& animationConstatns)noexcept
	{
		uint layerSize = (uint)stateDiagram.size();
		for (uint i = 0; i < layerSize; ++i)
		{
			if (stateDiagram[i]->HasAnimationData())
				stateDiagram[i]->Update(animationtimes[i], animationShaderData, modelSkeleton, animationConstatns, i);
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
	uint JAnimationController::GetAnimationDiagramCount()const noexcept
	{
		return (uint)stateDiagram.size();
	}
	std::string JAnimationController::GetAnimationDiagramName(const uint diagramIndex)noexcept
	{
		if (diagramIndex >= stateDiagram.size())
			return nullptr;
		else
			return stateDiagram[diagramIndex]->GetName();
	}
	EditorDiagram* JAnimationController::GetEditorDiagram(const uint diagramIndex)noexcept
	{
		if (diagramIndex >= stateDiagram.size())
			return nullptr;
		else
			return stateDiagram[diagramIndex]->GetEditorDiagram();
	}
	bool JAnimationController::GetAnimationFSMstateVectorHandle(const uint diagramIndex, _Out_ uint& stateCount, _Out_ std::vector<JAnimationFSMstate*>::const_iterator& iter)noexcept
	{
		if (diagramIndex >= stateDiagram.size())
		{
			stateCount = 0;
			return false;
		}
		else
		{
			iter = stateDiagram[diagramIndex]->GetAnimationFSMstateVectorHandle(stateCount);
			return true;
		}
	}
	uint JAnimationController::GetConditionCount()const noexcept
	{
		return conditionStorage->GetConditionCount();
	}
	Core::JFSMcondition* JAnimationController::GetCondition(const uint index)noexcept
	{
		return conditionStorage->GetCondition(index);
	}
	std::vector<std::string> JAnimationController::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".controller" };
		return format;
	}
	void JAnimationController::SetConditionName(const std::string& oldName, const std::string& newName)noexcept
	{
		conditionStorage->SetConditionName(oldName, newName);
	}
	void JAnimationController::SetConditionValueType(const std::string& name, const J_FSMCONDITION_VALUE_TYPE type)noexcept
	{
		conditionStorage->SetConditionValueType(name, type);
	}
	bool JAnimationController::CreateAnimationDiagram()noexcept
	{
		if (stateDiagram.size() >= diagramMaxCount)
			return false;

		std::string newName = "NewAnimationDiagram";
		bool isOk = false;
		int sameCount = 0;
		const uint stateDiagramCount = (uint)stateDiagram.size();

		while (!isOk)
		{
			bool hasSameName = false;
			for (uint i = 0; i < stateDiagramCount; ++i)
			{
				if (stateDiagram[i]->GetName() == newName)
				{
					hasSameName = true;
					break;
				}
			}

			if (hasSameName)
			{
				JCommonUtility::ModifyOverlappedName(newName, newName.length(), sameCount);
				++sameCount;
			}
			else
				isOk = true;
		}

		stateDiagram.push_back(std::make_unique<JAnimationFSMdiagram>(newName, conditionStorage.get()));
		return true;
	}
	bool JAnimationController::CreateAnimationClip(const uint diagramIndex, const float initPosX, const float initPosY)noexcept
	{
		if (diagramIndex >= stateDiagram.size())
			return false;

		JAnimationFSMstate* state = stateDiagram[diagramIndex]->AddAnimationClipState();
		if (state != nullptr)
		{
			EditorDiagramNode* stateCoord = state->GetEditorDiagramNode();
			stateCoord->SetPos(initPosX, initPosY);
			return true;
		}
		else
			return false;
	}
	bool JAnimationController::CreateTransition(const uint diagramIndex, const std::string& stateName, const std::string& outputStateName)
	{
		if (diagramIndex >= stateDiagram.size())
			return false;

		return stateDiagram[diagramIndex]->AddAnimationTransition(stateName, outputStateName);
	}
	bool JAnimationController::CreateParameter()noexcept
	{
		JFSMcondition* newCondition = conditionStorage->AddConditionValue();
		return newCondition != nullptr;
	}
	bool JAnimationController::EraseAnimationDiagram(const uint diagramIndex)noexcept
	{
		if (diagramIndex >= stateDiagram.size())
			return false;

		stateDiagram[diagramIndex]->Clear();
		stateDiagram.erase(stateDiagram.begin() + diagramIndex);
		return true;
	}
	bool JAnimationController::EraseAnimationState(const uint diagramIndex, const std::string& stateName)noexcept
	{
		if (diagramIndex >= stateDiagram.size())
			return false;

		return stateDiagram[diagramIndex]->EraseAnimationState(stateName);
	}
	bool JAnimationController::EraseParameter(const std::string& paraName)noexcept
	{
		return conditionStorage->EraseCondition(paraName);
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
	bool JAnimationController::IsValidResource()const noexcept
	{
		return JValidInterface::IsValidResource();
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
	void JAnimationController::RegisterFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JAnimationController(owner->MakeUniqueFileName(GetDefaultName<JAnimationController>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
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
		 
		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, false, false, false};
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JAnimationController::JAnimationController(const std::string& name, size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JAnimationControllerInterface(name, guid, flag, directory, formatIndex)
	{
		conditionStorage = std::make_unique<JFSMconditionStorage>();
		stateDiagram.push_back(std::make_unique<JAnimationFSMdiagram>("BaseLayer", conditionStorage.get()));

		//수정필요
		//포트폴리오용 임시코드

		JAnimationFSMstateClip* clip00 = dynamic_cast<JAnimationFSMstateClip*>(stateDiagram[0]->AddAnimationClipState());
		JAnimationFSMstateClip* clip01 = dynamic_cast<JAnimationFSMstateClip*>(stateDiagram[0]->AddAnimationClipState());
		JAnimationFSMstateClip* clip02 = dynamic_cast<JAnimationFSMstateClip*>(stateDiagram[0]->AddAnimationClipState());
		JAnimationFSMstateClip* clip03 = dynamic_cast<JAnimationFSMstateClip*>(stateDiagram[0]->AddAnimationClipState());
		JAnimationFSMstateClip* clip04 = dynamic_cast<JAnimationFSMstateClip*>(stateDiagram[0]->AddAnimationClipState());
		JAnimationFSMstateClip* clip05 = dynamic_cast<JAnimationFSMstateClip*>(stateDiagram[0]->AddAnimationClipState());

		stateDiagram[0]->AddAnimationTransition(clip00->GetName(), clip01->GetName());
		stateDiagram[0]->AddAnimationTransition(clip01->GetName(), clip02->GetName());
		stateDiagram[0]->AddAnimationTransition(clip02->GetName(), clip03->GetName());
		stateDiagram[0]->AddAnimationTransition(clip03->GetName(), clip04->GetName());
		stateDiagram[0]->AddAnimationTransition(clip04->GetName(), clip05->GetName());
		stateDiagram[0]->AddAnimationTransition(clip05->GetName(), clip00->GetName());

		/*const size_t aniclip00Guid = JCommonUtility::CalculateGuid("D:\\NewProjTextFolder\\Douka\\Content\\Resource\\Anim_Blade_Attack_1.clip");
		const size_t aniclip01Guid = JCommonUtility::CalculateGuid("D:\\NewProjTextFolder\\Douka\\Content\\Resource\\Anim_Blade_Attack_2.clip");
		const size_t aniclip02Guid = JCommonUtility::CalculateGuid("D:\\NewProjTextFolder\\Douka\\Content\\Resource\\Anim_Blade_Attack_3.clip");
		const size_t aniclip03Guid = JCommonUtility::CalculateGuid("D:\\NewProjTextFolder\\Douka\\Content\\Resource\\Anim_Blade_Attack_4.clip");
		const size_t aniclip04Guid = JCommonUtility::CalculateGuid("D:\\NewProjTextFolder\\Douka\\Content\\Resource\\Anim_Blade_Attack_5.clip");
		const size_t aniclip05Guid = JCommonUtility::CalculateGuid("D:\\NewProjTextFolder\\Douka\\Content\\Resource\\Anim_Blade_Attack_6.clip");

		clip00->SetClip(JResourceManager::Instance().GetResource<JAnimationClip>(aniclip00Guid));
		clip01->SetClip(JResourceManager::Instance().GetResource<JAnimationClip>(aniclip01Guid));
		clip02->SetClip(JResourceManager::Instance().GetResource<JAnimationClip>(aniclip02Guid));
		clip03->SetClip(JResourceManager::Instance().GetResource<JAnimationClip>(aniclip03Guid));
		clip04->SetClip(JResourceManager::Instance().GetResource<JAnimationClip>(aniclip04Guid));
		clip05->SetClip(JResourceManager::Instance().GetResource<JAnimationClip>(aniclip05Guid));*/
	}
	JAnimationController::~JAnimationController(){}
}