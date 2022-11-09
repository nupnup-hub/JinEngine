#pragma once
#include"../Reflection/JReflection.h"

namespace JinEngine
{
	namespace Core
	{
		REGISTER_ENUM_CLASS(J_SPACE_SPATIAL_TYPE, short, OCTREE = 0, BVH, KD_TREE)
		REGISTER_ENUM_CLASS(J_SPACE_SPATIAL_SPLIT_TYPE, short, SAH = 0);
		REGISTER_ENUM_CLASS(J_SPACE_SPATIAL_BUILD_TYPE, short, TOP_DOWN = 0);
	}
}