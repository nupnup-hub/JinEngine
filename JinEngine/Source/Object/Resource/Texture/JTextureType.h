#pragma once
#include"../../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	REGISTER_ENUM_CLASS(J_TEXTURE_RESOLUTION, int, ORIGINAL = 0,
		LOWEST = 256,
		LOW = 512,
		MEDIUM = 1024,
		HIGH = 2048,
		HIGHEST = 4096)
}