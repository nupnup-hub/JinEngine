#pragma once 
#include<cstdint>

namespace JinEngine
{
	namespace Core
	{
		enum class J_FBXRESULT : int
		{
			FAIL = 0,
			HAS_MESH = 1 << 0,
			HAS_ANIMATION = 1 << 1,
			HAS_SKELETON = 1 << 2,
		};
	}
}