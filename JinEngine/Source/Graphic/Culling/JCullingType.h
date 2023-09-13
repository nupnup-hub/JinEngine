#pragma once
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_CULLING_TYPE
		{
			FRUSTUM,
			HZB_OCCLUSION,
			HD_OCCLUSION,
			COUNT
		}; 
	}
}