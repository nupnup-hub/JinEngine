#pragma once
#include"../Reflection/JReflection.h"

namespace JinEngine
{
	namespace Core
	{
		REGISTER_ENUM_CLASS(J_SCENE_SPATIAL_STRUCTURE_TYPE, short, OCTREE = 0, BVH)
	}
}