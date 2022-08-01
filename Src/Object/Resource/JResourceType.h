#pragma once

namespace JinEngine
{
	enum class J_RESOURCE_TYPE
	{
		NONE = 0,
		MESH,
		MATERIAL,
		TEXTURE,
		SCRIPT,
		SHADER, 
		SCENE, 
		MODEL,
		SKELETON,
		ANIMATION_CLIP,
		ANIMATION_CONTROLLER, 
		COUNT
	};

	enum class RESOURCE_ALIGN_TYPE : short
	{
		NONE,
		NAME = 1, 
		DEPENDENCY = 2,
	};
}
