#pragma once
#include"../../../Component/RenderItem/JRenderLayer.h"
#include"../../../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	REGISTER_ENUM_CLASS(J_ACCELERATOR_TYPE, short, OCTREE = 0, BVH, KD_TREE)
	REGISTER_ENUM_CLASS(J_ACCELERATOR_SPLIT_TYPE, short, SAH = 0);
	REGISTER_ENUM_CLASS(J_ACCELERATOR_BUILD_TYPE, short, TOP_DOWN = 0);

	enum class J_ACCELERATOR_LAYER
	{
		COMMON_OBJECT,
		DEBUG_OBJECT,
		COUNT,
		INVALID,
	};

	static J_ACCELERATOR_LAYER ConvertAcceleratorLayer(const J_RENDER_LAYER layer)
	{
		switch (layer)
		{
		case JinEngine::J_RENDER_LAYER::OPAQUE_OBJECT:
			return J_ACCELERATOR_LAYER::COMMON_OBJECT;
		case JinEngine::J_RENDER_LAYER::SKY:
			return J_ACCELERATOR_LAYER::INVALID;
		case JinEngine::J_RENDER_LAYER::DEBUG_OBJECT:
			return J_ACCELERATOR_LAYER::DEBUG_OBJECT;
		case JinEngine::J_RENDER_LAYER::DEBUG_UI:
			return J_ACCELERATOR_LAYER::DEBUG_OBJECT;
		default:
			return J_ACCELERATOR_LAYER::INVALID;
		}
	}

	enum class J_ACCELERATOR_SORT_TYPE
	{
		NOT_USE,
		ASCENDING,
		DESCENDING
	};
}