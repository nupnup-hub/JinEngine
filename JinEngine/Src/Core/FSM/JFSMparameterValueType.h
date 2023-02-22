#pragma once
#include<string>
#include<algorithm>
#include"../Reflection/JReflection.h"
 
namespace JinEngine
{
	namespace Core
	{
		REGISTER_ENUM_CLASS(J_FSM_PARAMETER_VALUE_TYPE, int, BOOL, INT, FLOAT);

		template<typename T>
		static float TypeValue(const J_FSM_PARAMETER_VALUE_TYPE type, T value)
		{
			if (type == J_FSM_PARAMETER_VALUE_TYPE::BOOL)
				return (bool)std::clamp(value, (T)0.0f, (T)1.0f);
			else if (type == J_FSM_PARAMETER_VALUE_TYPE::INT)
				return (int)value;
			else
				return (float)value;
		}
	}
}