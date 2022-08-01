#pragma once 
#include"../JDataType.h"

namespace JinEngine
{
	namespace Core
	{
		enum class J_PARAMETER_TYPE : uint8
		{
			UnKnown,
			Void,
			Bool, 
			Int, //short , int, long,  size_t 
			Float, 
			String,
			Class, 
			Enum,
		};
	}
}