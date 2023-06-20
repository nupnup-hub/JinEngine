#pragma once  
#include"../../Core/Reflection/JReflection.h"
namespace JinEngine
{
	namespace Graphic
	{ 
		REGISTER_ENUM_CLASS(J_UPLOAD_FRAME_RESOURCE_TYPE, int, OBJECT,
			PASS,
			ANIMATION,
			CAMERA,
			BOUNDING_OBJECT,
			MATERIAL,
			LIGHT,
			LIGHT_INDEX,
			SHADOW_MAP_LIGHT,
			SHADOW_MAP,
			HZB_OCC_OBJECT,
			HZB_OCC_REQUESTOR)
		
		enum class J_UPLOAD_CAPACITY_CONDITION
		{
			KEEP,
			DOWN_CAPACITY,
			UP_CAPACITY
		};

		enum class J_UPLOAD_BUFFER_TYPE
		{
			CONSTANT,
			COMMON,
			UNORDERED_ACCEESS,
			READ_BACK,
		};
	}
}