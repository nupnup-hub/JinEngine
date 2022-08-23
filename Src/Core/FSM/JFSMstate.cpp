#include"JFSMstate.h"
#include"JFSMtransition.h"
#include"JFSMcondition.h"
#include"../Guid/GuidCreator.h"
namespace JinEngine
{
	namespace Core
	{
		JFSMstate::JFSMstate(const std::string& name, const size_t guid)
			:name(name), guid(guid)
		{}
		JFSMstate::~JFSMstate() {}
		std::string JFSMstate::GetName()noexcept
		{
			return name;
		}
		size_t JFSMstate::GetGuid()noexcept
		{
			return guid;
		}
		void JFSMstate::SetName(const std::string& name)
		{
			if(!name.empty())
				JFSMstate::name = name; 
		}
		void JFSMstate::SetTransitionCondtion(const size_t outputStateGuId, const uint conditionIndex, JFSMcondition* newCondition)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputStateGuId() == outputStateGuId)
				{
					transition[i]->SetCondition(conditionIndex, newCondition);
					break;
				}
			}
		}
		void JFSMstate::SetTransitionCondtionOnValue(const size_t outputStateGuId, const uint conditionIndex, const float onValue)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputStateGuId() == outputStateGuId)
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
		JFSMtransition* JFSMstate::AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept
		{
			JFSMtransition* res = nullptr;
			bool hasSameOutput = false;
			const uint transitionSize = (uint)transition.size();

			if (transitionSize >= maxNumberOffTransistions)
				return res;

			size_t newTransitionId = newTransition->GetOutputStateGuId();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputStateGuId() == newTransitionId)
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
		bool JFSMstate::RemoveTransition(const size_t outputStateGuId)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint index; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputStateGuId() == outputStateGuId)
				{
					transition[index].reset();
					transition.erase(transition.begin() + index);
					return true;
				}
			}	
			return false;
		}
		void JFSMstate::EnterState()noexcept
		{
			decidedNextState = false;
		}
		bool JFSMstate::AddTransitionCondition(const size_t outputStateGuId, JFSMcondition* condition)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint index = 0; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputStateGuId() == outputStateGuId)
					return transition[index]->AddCondition(condition);
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
		bool JFSMstate::RemoveTransitionCondition(const size_t outputStateGuId, const size_t conditionGuid)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint index = 0; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputStateGuId() == outputStateGuId)
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