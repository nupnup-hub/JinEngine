#pragma once 
#include<cstdint>

namespace JinEngine
{
	namespace Core
	{
		enum J_FBX_RESULT : int
		{
			J_FBX_RESULT_FAIL = 0,
			J_FBX_RESULT_HAS_MESH = 1 << 0,
			J_FBX_RESULT_HAS_ANIMATION = 1 << 1,
			J_FBX_RESULT_HAS_SKELETON = 1 << 2,
		};
	}
}