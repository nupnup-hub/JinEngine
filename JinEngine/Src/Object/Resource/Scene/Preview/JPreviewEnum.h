#pragma once
namespace JinEngine
{
	enum class J_PREVIEW_DIMENSION
	{
		TWO_DIMENTIONAL ,
		THREE_DIMENTIONAL,
		COUNT,
	}; 

	enum class J_PREVIEW_FLAG
	{
		NONE = 0,
		NON_FIXED = 1 << 0,
		HAS_SKELETON = 1 << 1,
		HAS_ANIMATIO = 1 << 2,
	}; 
}