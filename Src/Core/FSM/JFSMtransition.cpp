#include"JFSMtransition.h"
#include"JFSMcondition.h"

namespace JinEngine
{
	namespace Core
	{
		FSMConditionWrap::FSMConditionWrap(JFSMcondition* condition)
			:condition(condition), onValue(0)
		{}
		bool FSMConditionWrap::IsSatisfied()const noexcept
		{
			return (PassDefectInspection()) && (condition->GetValue() == onValue);
		}
		bool FSMConditionWrap::PassDefectInspection()const noexcept
		{
			return condition != nullptr;
		}
		JFSMtransition::JFSMtransition(const size_t outputId)
			:outputId(outputId)
		{}
		size_t JFSMtransition::GetOutputId()noexcept
		{
			return outputId;
		}
		bool JFSMtransition::HasSatisfiedCondition()noexcept
		{
			const uint conditionVecSize = (uint)conditionVec.size();

			if (conditionVecSize == 0)
				return true;

			for (uint index = 0; index < conditionVecSize; ++index)
			{
				if (conditionVec[index]->IsSatisfied())
					return true;
			}
			return false;
		}
		void JFSMtransition::Initialize()noexcept
		{
			const uint conditionVecSize = (uint)conditionVec.size();

			for (uint index = 0; index < conditionVecSize; ++index)
				conditionVec[index]->onValue = 0;
		}
		void JFSMtransition::SetConditionOnValue(const uint conditionIndex, const float onValue)noexcept
		{
			if (conditionIndex >= conditionVec.size())
				return;

			conditionVec[conditionIndex]->onValue = onValue;
		}
		void JFSMtransition::SetCondition(const uint oldConditionIndex, JFSMcondition* newCondition)noexcept
		{
			if (oldConditionIndex >= conditionVec.size())
				return;

			conditionVec[oldConditionIndex]->condition = newCondition;
		}
		bool JFSMtransition::AddCondition(JFSMcondition* condition)noexcept
		{
			if (conditionVec.size() >= maxNumberOffCondition)
				return false;

			conditionVec.push_back(std::make_unique<FSMConditionWrap>(condition));
			return true;
		}
		bool JFSMtransition::PopCondition(const std::string& name)noexcept
		{
			bool hasCondition = false;
			const uint conditionVecSize = (uint)conditionVec.size();
			uint index = 0;

			for (; index < conditionVecSize; ++index)
			{
				if (conditionVec[index]->condition->GetName() == name)
				{
					hasCondition = true;
					break;
				}
			}

			if (!hasCondition)
				return false;
			else
			{
				conditionVec[index].reset();
				conditionVec.erase(conditionVec.begin() + index);
				return true;
			}
		}
	}
}