#include"JFSMcondition.h" 
#include"JFSMconditionStorage.h"
#include"JFSMownerInterface.h" 
#include"JFSMfactory.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMcondition::JFSMconditionInitData::JFSMconditionInitData(const std::wstring& name, const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType, IJFSMconditionOwner* conditionOwner)
			:JFSMIdentifierInitData(name, guid), valueType(valueType), conditionOwner(conditionOwner)
		{}
		JFSMcondition::JFSMconditionInitData::JFSMconditionInitData(const size_t guid, IJFSMconditionStorageOwner* storageOwner)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMcondition>(), guid), valueType(J_FSMCONDITION_VALUE_TYPE::BOOL), storageOwner(storageOwner)
		{}
		bool JFSMcondition::JFSMconditionInitData::IsValid() noexcept
		{
			return conditionOwner != nullptr;
		}
		J_FSM_OBJECT_TYPE JFSMcondition::JFSMconditionInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::CONDITION;
		}

		void JFSMcondition::Initialize()noexcept
		{
			value = 0;
		}
		J_FSM_OBJECT_TYPE JFSMcondition::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::CONDITION;
		}
		float JFSMcondition::GetValue()const noexcept
		{
			if (valueType == J_FSMCONDITION_VALUE_TYPE::BOOL)
				return static_cast<bool>(value);
			else if (valueType == J_FSMCONDITION_VALUE_TYPE::INT)
				return static_cast<int>(value);
			else
				return value;
		}
		J_FSMCONDITION_VALUE_TYPE JFSMcondition::GetValueType()const noexcept
		{
			return valueType;
		} 
		void JFSMcondition::SetValueType(const J_FSMCONDITION_VALUE_TYPE valueType)noexcept
		{
			JFSMcondition::valueType = valueType;
			SetValue(value);
		}
		void JFSMcondition::Clear(){}
		bool JFSMcondition::RegisterCashData()noexcept
		{
			return conditionOwner->AddCondition(this);
		}
		bool JFSMcondition::DeRegisterCashData()noexcept
		{
			return conditionOwner->RemoveCondition(this);
		}
		void JFSMcondition::RegisterJFunc()
		{
			auto createConditionLam = [](JOwnerPtr<JFSMconditionInitData> initData)-> JFSMcondition*
			{
				if (initData.IsValid())
				{
					JOwnerPtr<JFSMcondition> ownerPtr = JPtrUtil::MakeOwnerPtr<JFSMcondition>(*initData.Get());
					JFSMcondition* newCond = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newCond;
				}
				return nullptr;
			};
			JFCFI<JFSMcondition>::RegisterCondition(createConditionLam);
		}
		JFSMcondition::JFSMcondition(const JFSMconditionInitData& initData)
			:JFSMInterface(initData.name, initData.guid), valueType(initData.valueType), conditionOwner(initData.conditionOwner)
		{
			if (initData.conditionOwner)
				conditionOwner = initData.conditionOwner;
			else
				conditionOwner = initData.storageOwner->GetConditionOwner();
		}
		JFSMcondition::~JFSMcondition() {}
	}
}