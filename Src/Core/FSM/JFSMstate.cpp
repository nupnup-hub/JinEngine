#include"JFSMstate.h"
#include"JFSMtransition.h"
#include"JFSMcondition.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMstate::~JFSMstate() {}
		std::string JFSMstate::GetName()noexcept
		{
			return name;
		}
		size_t JFSMstate::GetId()noexcept
		{
			return id;
		}
		void JFSMstate::SetName(const std::string& name, const size_t id)
		{
			JFSMstate::name = name;
			JFSMstate::id = id;
		}
		void JFSMstate::SetTransitionCondtion(const size_t outputId, JFSMcondition* newCondition, const uint oldConditionIndex)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputId() == outputId)
				{
					transition[oldConditionIndex]->SetCondition(oldConditionIndex, newCondition);
					break;
				}
			}
		}
		void JFSMstate::SetTransitionCondtionOnValue(const size_t outputId, const uint conditionIndex, const float onValue)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputId() == outputId)
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

			size_t newTransitionId = newTransition->GetOutputId();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transition[i]->GetOutputId() == newTransitionId)
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
		bool JFSMstate::EraseTransition(const size_t outputId)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			bool hasTransition = false;
			uint index = 0;
			for (index; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputId() == outputId)
				{
					hasTransition = true;
					break;
				}
			}
			if (!hasTransition)
				return false;

			transition[index].reset();
			transition.erase(transition.begin() + index);
			return true;
		}
		void JFSMstate::EnterState()noexcept
		{
			decidedNextState = false;
		}
		bool JFSMstate::AddTransitionCondition(const size_t outputId, JFSMcondition* condition)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			bool hasTransition = false;
			uint index = 0;
			for (index; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputId() == outputId)
				{
					hasTransition = true;
					break;
				}
			}
			if (!hasTransition)
				return false;

			return transition[index]->AddCondition(condition);
		}

		bool JFSMstate::EraseCondition(const std::string& conditionName)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			for (uint i = 0; i < transitionSize; ++i)
				transition[i]->PopCondition(conditionName);
			return true;
		}
		bool JFSMstate::EraseTransitionCondition(const size_t outputId, const std::string& conditionName)noexcept
		{
			const uint transitionSize = (uint)transition.size();
			bool hasTransition = false;
			uint index = 0;
			for (index; index < transitionSize; ++index)
			{
				if (transition[index]->GetOutputId() == outputId)
				{
					hasTransition = true;
					break;
				}
			}
			if (!hasTransition)
				return false;

			return transition[index]->PopCondition(conditionName);
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