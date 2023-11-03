#pragma once
#include<string>
#include<algorithm>
#include"../Reflection/JReflection.h"
 
namespace JinEngine
{
	namespace Core
	{
		REGISTER_ENUM_CLASS(J_FSM_PARAMETER_VALUE_TYPE, int, BOOL, INT, FLOAT)

		template<typename T>
		static float FsmParamTypeValue(const J_FSM_PARAMETER_VALUE_TYPE type, T value)
		{
			if (type == J_FSM_PARAMETER_VALUE_TYPE::BOOL)
				return static_cast<float>(static_cast<bool>(static_cast<int>(value)));
			else if (type == J_FSM_PARAMETER_VALUE_TYPE::INT)
				return static_cast<float>(static_cast<int>(value));
			else
				return static_cast<float>(value);
		}
	}
}