#pragma once
namespace JinEngine
{
	namespace Graphic
	{
		enum class J_FRAME_RESOURCE_TYPE
		{
			OBJECT,
			PASS,
			ANIMATION,
			CAMERA, 
			BOUNDING_OBJECT,
			MATERIAL, 
			LIGHT,
			LIGHT_INDEX,
			SHADOW_MAP_LIGHT, 
			SHADOW_MAP,
			COUNT,
		};
		enum class FRAME_CAPACITY_CONDITION
		{
			KEEP,
			DOWN_CAPACITY,
			UP_CAPACITY
		};
	}
}