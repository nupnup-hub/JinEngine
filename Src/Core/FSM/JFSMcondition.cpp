#include"JFSMcondition.h" 

namespace JinEngine
{
	namespace Core
	{
		JFSMcondition::JFSMcondition(const std::string& name, const size_t guid, const J_FSMCONDITION_VALUE_TYPE valueType)
			:name(name), guid(guid), valueType(valueType)
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
		size_t JFSMcondition::GetGuid()const noexcept
		{
			return guid;
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
		void JFSMcondition::SetName(const std::string& name)noexcept
		{
			if (!name.empty()) 
				JFSMcondition::name = name; 
		}
		void JFSMcondition::SetValueType(const J_FSMCONDITION_VALUE_TYPE valueType)noexcept
		{
			JFSMcondition::valueType = valueType;
			SetValue(value);
		}
	}
}