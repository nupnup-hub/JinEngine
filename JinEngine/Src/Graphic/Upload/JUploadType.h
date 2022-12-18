#pragma once
namespace JinEngine
{
	namespace Graphic
	{
		enum class J_UPLOAD_RESOURCE_TYPE
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
			 
			//HZB_OBJECT,
			COUNT,
		};
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