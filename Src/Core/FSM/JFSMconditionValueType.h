#pragma once
#include<string>
namespace JinEngine
{
	namespace Core
	{
		enum class J_FSMCONDITION_VALUE_TYPE
		{
			BOOL,
			INT,
			FLOAT
		};
		struct JFsmConditionValueType
		{
		public:
			static constexpr int count = 3;
			static std::string ConditionEnumToString(const J_FSMCONDITION_VALUE_TYPE type)
			{
				if (type == J_FSMCONDITION_VALUE_TYPE::BOOL)
					return "Bool";
				else if (type == J_FSMCONDITION_VALUE_TYPE::INT)
					return "Int";
				else
					return "Float";
			}
		};
	}
}