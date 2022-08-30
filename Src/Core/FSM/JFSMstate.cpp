#include"JFSMstate.h"
#include"JFSMtransition.h"
#include"JFSMcondition.h"
#include"../Guid/GuIdCreator.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMstate::JFSMstate(const std::wstring& name, const size_t guid)
			:JFSMIdentifier(name, guid)
		{}
		JFSMstate::~JFSMstate() {}
		J_FSM_OBJECT_TYPE JFSMstate::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::STATE;
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
			bool hasSameOutput = false;
			const uint transitionSize = (uint)transition.size();

			if (transitionSize >= maxNumberOffTransistions)
				return res;

			size_t newTransitionId = newTransition->GetOutputStateGuid();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputStateGuid() == newTransitionId)
				{
					hasSameOutput = true;
					break;
				}
			}

			if (hasSameOutput)
				return res;
			else
			{
				res = newTransition.get();
				transition.push_back(std::move(newTransition));
				return res;
			}
		}
		bool JFSMstate::RemoveTransition(const size_t outputStateGuid)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint index; index < transitionSize; ++index)
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
		JFSMconditionWrap* JFSMstate::AddTransitionCondition(const size_t outputStateGuid, JFSMcondition* condition)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint index = 0; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputStateGuid() == outputStateGuid)
					return transition[index]->AddCondition(condition);
			}
			return nullptr;
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
	}
}