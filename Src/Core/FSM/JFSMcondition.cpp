#include"JFSMcondition.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMcondition::JFSMcondition(const std::string& name, const size_t id, const J_FSMCONDITION_VALUE_TYPE valueType)
			:name(name), id(id), valueType(valueType)
		{}
		JFSMcondition::~JFSMcondition() {}

		void JFSMcondition::Initialize()noexcept
		{
			value = 0;
		}
		std::string JFSMcondition::GetName()const noexcept
		{
			return name;
		}
		size_t JFSMcondition::GetId()const noexcept
		{
			return id;
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
		void JFSMcondition::SetValue(float value)noexcept
		{
			if (valueType == J_FSMCONDITION_VALUE_TYPE::BOOL)
				JFSMcondition::value = static_cast<float>(std::clamp(static_cast<int>(value), 0, 1));
			else if (valueType == J_FSMCONDITION_VALUE_TYPE::INT)
				JFSMcondition::value = static_cast<float>(static_cast<int>(value));
			else
				JFSMcondition::value = value;
		}
		void JFSMcondition::SetName(const std::string& name, const size_t id)noexcept
		{
			JFSMcondition::name = name;
			JFSMcondition::id = id;
		}
		void JFSMcondition::SetValueType(const J_FSMCONDITION_VALUE_TYPE valueType)noexcept
		{
			JFSMcondition::valueType = valueType;
			SetValue(value);
		}
	}
}