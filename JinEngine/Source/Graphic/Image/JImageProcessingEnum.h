#pragma once
#include"../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_DOWN_SAMPLING_TYPE
		{
			BOX,
			GAUSIAAN,
			KAISER,
			COUNT
		};
		enum class J_BLUR_TYPE
		{
			BOX,
			GAUSIAAN,
			//BILATERAL,
			COUNT
		}; 
		REGISTER_ENUM_CLASS(J_SSAO_TYPE ,int, DEFAULT, HORIZON_BASED)
		REGISTER_ENUM_CLASS(J_KENEL_SIZE, int, _2x2,
			_3x3,
			_5x5,
			_7x7)
	}
}