#include"JFSMstate.h"
#include"JFSMtransition.h"
#include"JFSMcondition.h"
#include"JFSMdiagram.h"
#include"../Guid/GuIdCreator.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMstate::JFSMstateInitData::JFSMstateInitData(const std::wstring& name, const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram)
			:JFSMIdentifierInitData(name, guid), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::JFSMstateInitData::JFSMstateInitData(const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMstate>(), guid), ownerDiagram(ownerDiagram)
		{}
		bool JFSMstate::JFSMstateInitData::IsValid() noexcept
		{
			return ownerDiagram.IsValid();
		}
		J_FSM_OBJECT_TYPE JFSMstate::JFSMstateInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::STATE;
		}

		J_FSM_OBJECT_TYPE JFSMstate::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::STATE;
		}
		uint JFSMstate::GetTransitionCount()const noexcept
		{
			return (uint)transition.size();
		}
		JFSMtransition* JFSMstate::GetTransition(uint index)noexcept
		{
			if (transition.size() <= index)
				return nullptr;
			else
				return transition[index].get();
		}
		void JFSMstate::SetTransitionCondtion(const size_t outputStateGuid, const uint conditionIndex, JFSMcondition* newCondition)noexcept
		{ 
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputStateGuid() == outputStateGuid)
				{
					transition[i]->SetCondition(conditionIndex, newCondition);
					break;
				}
			}
		}
		void JFSMstate::SetTransitionCondtionOnValue(const size_t outputStateGuid, const uint conditionIndex, const float onValue)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputStateGuid() == outputStateGuid)
				{
					transition[i]->SetConditionOnValue(conditionIndex, onValue);
					break;
				}
			}
		}
		void JFSMstate::Initialize()noexcept
		{
			decidedNextState = false;
			const uint transitionSize = (uint)transition.size();

			for (uint index = 0; index < transitionSize; ++index)
				transition[index]->Initialize();
		}
		void JFSMstate::EnterState()noexcept
		{
			decidedNextState = false;
		}
		JFSMtransition* JFSMstate::AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept
		{
			JFSMtransition* res = nullptr; 
			if (newTransition != nullptr && ownerDiagram->IsDiagramState(newTransition->GetOutputStateGuid()))
			{
				res = newTransition.get();
				transition.push_back(std::move(newTransition));
			}
			return res;
		}
		JFSMconditionWrap* JFSMstate::AddTransitionCondition(const size_t outputStateGuid, JFSMcondition* condition)noexcept
		{
			if (condition == nullptr || !ownerDiagram->IsValidCondition(condition))
				return nullptr;

			const uint transitionSize = (uint)transition.size();
			for (uint index = 0; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputStateGuid() == outputStateGuid)
					return transition[index]->AddCondition(condition);
			}
			return nullptr;
		}
		bool JFSMstate::RemoveTransition(const size_t outputStateGuid)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint index = 0; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputStateGuid() == outputStateGuid)
				{
					transition[index].reset();
					transition.erase(transition.begin() + index);
					return true;
				}
			}	
			return false;
		}
		bool JFSMstate::RemoveCondition(const size_t guid)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
				transition[i]->PopCondition(guid);
			return true;
		}
		bool JFSMstate::RemoveTransitionCondition(const size_t outputStateGuid, const size_t conditionGuid)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint index = 0; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputStateGuid() == outputStateGuid)
					return transition[index]->PopCondition(conditionGuid);
			}
			return false;
		}
		void JFSMstate::Clear()noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
				transition[i].reset();
			transition.clear();
		}
		bool JFSMstate::RegisterCashData()noexcept
		{
			return ownerDiagram->AddState(this);
		}
		bool JFSMstate::DeRegisterCashData()noexcept
		{
			return ownerDiagram->RemoveState(this);
		}
		JFSMstate::JFSMstate(const JFSMstateInitData& initData)
			:JFSMInterface(ownerDiagram->GetUniqueStateName(initData.name), initData.guid), ownerDiagram(initData.ownerDiagram.Get())
		{}
		JFSMstate::~JFSMstate() {}
	}
}