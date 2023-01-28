#include"JFSMcondition.h" 
#include"JFSMconditionStorage.h"
#include"JFSMownerInterface.h" 
#include"JFSMfactory.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMcondition::JFSMconditionInitData::JFSMconditionInitData(const std::wstring& name, const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType, IJFSMconditionOwnerInterface* conditionOwner)
			:JFSMIdentifierInitData(name, guid), valueType(valueType), conditionOwner(conditionOwner)
		{}
		JFSMcondition::JFSMconditionInitData::JFSMconditionInitData(const size_t guid, IJFSMconditionOwnerInterface* conditionOwner)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMcondition>(), guid), valueType(J_FSMCONDITION_VALUE_TYPE::BOOL), conditionOwner(conditionOwner)
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
			auto createConditionLam = [](JOwnerPtr<JFSMIdentifierInitData> initData)-> JFSMInterface*
			{ 
				if (initData.IsValid() && initData->GetFSMobjType() == J_FSM_OBJECT_TYPE::CONDITION)
				{
					JFSMconditionInitData* condInitData = static_cast<JFSMconditionInitData*>(initData.Get());
					JOwnerPtr<JFSMcondition> ownerPtr = JPtrUtil::MakeOwnerPtr<JFSMcondition>(*condInitData);
					JFSMcondition* newCond = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newCond;
				}
				return nullptr;
			};
			JFFI<JFSMcondition>::Register(createConditionLam);
		}
		JFSMcondition::JFSMcondition(const JFSMconditionInitData& initData)
			:JFSMInterface(initData.name, initData.guid), valueType(initData.valueType)
		{
			conditionOwner = initData.conditionOwner;
		}
		JFSMcondition::~JFSMcondition() 
		{
			conditionOwner = nullptr;
		}
	}
}