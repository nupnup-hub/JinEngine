#include"JFSMtransition.h"
#include"JFSMcondition.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		JFSMconditionWrap::JFSMconditionWrap(JFSMcondition* condition)
			:condition(condition), onValue(0)
		{}
		JFSMcondition* JFSMconditionWrap::GetCondition()noexcept
		{
			return condition;
		}
		float JFSMconditionWrap::GetOnValue()const noexcept
		{
			J_FSMCONDITION_VALUE_TYPE valueType = condition->GetValueType();
			if (valueType == J_FSMCONDITION_VALUE_TYPE::BOOL)
				return static_cast<bool>(onValue);
			else if (valueType == J_FSMCONDITION_VALUE_TYPE::INT)
				return static_cast<int>(onValue);
			else 
				return onValue;
		}
		void JFSMconditionWrap::SetCondition(JFSMcondition* newCondition)noexcept
		{
			condition = newCondition;
			SetOnValue(onValue);
		}
		void JFSMconditionWrap::SetOnValue(float newValue)noexcept
		{
			onValue = newValue;
		}

		bool JFSMconditionWrap::IsSatisfied()const noexcept
		{
			return (PassDefectInspection()) && (condition->GetValue() == onValue);
		}
		bool JFSMconditionWrap::PassDefectInspection()const noexcept
		{
			return condition != nullptr;
		}

		uint JFSMtransition::GetConditioCount()const noexcept
		{
			return (uint)conditionVec.size();
		}
		float JFSMtransition::GetConditionOnValue(const uint index)const noexcept
		{
			if (index >= conditionVec.size())
				return errorOnValue;

			return conditionVec[index]->GetOnValue();
		}
		size_t JFSMtransition::GetOutputStateGuid()const noexcept
		{
			return outputStateGuid;
		}
		bool JFSMtransition::HasSatisfiedCondition()const noexcept
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
		JFSMcondition* JFSMtransition::GetConditionByIndex(const uint index)const noexcept
		{
			if (index >= conditionVec.size())
				return nullptr;

			return conditionVec[index]->GetCondition();
		}
		void JFSMtransition::SetConditionOnValue(const uint index, const float onValue)noexcept
		{
			if (index >= conditionVec.size())
				return;

			conditionVec[index]->SetOnValue(onValue);
		}
		void JFSMtransition::SetCondition(const uint oldIndex, JFSMcondition* newCondition)noexcept
		{
			if (oldIndex >= conditionVec.size())
				return;

			conditionVec[oldIndex]->SetCondition(newCondition);
		}
		JFSMconditionWrap* JFSMtransition::AddCondition(JFSMcondition* condition)noexcept
		{
			if (conditionVec.size() >= maxNumberOffCondition)
				return nullptr;

			conditionVec.push_back(std::make_unique<JFSMconditionWrap>(condition));
			return conditionVec[conditionVec.size() - 1].get();
		}
		bool JFSMtransition::PopCondition(const size_t outputStateGuid)noexcept
		{
			bool hasCondition = false;
			const uint conditionVecSize = (uint)conditionVec.size();
			uint index = 0;

			for (; index < conditionVecSize; ++index)
			{
				if (conditionVec[index]->GetCondition()->GetGuid() == outputStateGuid)
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
		void JFSMtransition::Initialize()noexcept
		{
			const uint conditionVecSize = (uint)conditionVec.size();

			for (uint index = 0; index < conditionVecSize; ++index)
				conditionVec[index]->SetOnValue(0);
		}
		JFSMtransition::JFSMtransition(const size_t outputStateGuid)
			:outputStateGuid(outputStateGuid)
		{}
		JFSMtransition::~JFSMtransition(){}
	}
}