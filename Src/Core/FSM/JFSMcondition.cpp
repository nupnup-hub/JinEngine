#include"JFSMcondition.h" 

namespace JinEngine
{
	namespace Core
	{
		JFSMcondition::JFSMcondition(const std::wstring& name, const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType)
			:JFSMIdentifier(name ,guid), valueType(valueType)
		{}
		JFSMcondition::~JFSMcondition() {}

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
				return std::clamp(value, 0.0f, 1.0f);
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
	}
}