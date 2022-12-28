#pragma once
#include"../Reflection/JReflection.h"
#include"../../Object/Component/RenderItem/JRenderLayer.h"

namespace JinEngine
{
	namespace Core
	{
		REGISTER_ENUM_CLASS(J_SPACE_SPATIAL_TYPE, short, OCTREE = 0, BVH, KD_TREE)
		REGISTER_ENUM_CLASS(J_SPACE_SPATIAL_SPLIT_TYPE, short, SAH = 0);
		REGISTER_ENUM_CLASS(J_SPACE_SPATIAL_BUILD_TYPE, short, TOP_DOWN = 0);
 
		enum class J_SPACE_SPATIAL_LAYER
		{
			COMMON_OBJECT,
			DEBUG_OBJECT,
			COUNT,
			INVALID,
		};

		static J_SPACE_SPATIAL_LAYER ConvertSpaceSpatialLayer(const J_RENDER_LAYER layer)
		{
			switch (layer)
			{
			case JinEngine::J_RENDER_LAYER::OPAQUE_OBJECT:
				return J_SPACE_SPATIAL_LAYER::COMMON_OBJECT;
			case JinEngine::J_RENDER_LAYER::SKY:
				return J_SPACE_SPATIAL_LAYER::INVALID;
			case JinEngine::J_RENDER_LAYER::DEBUG_LAYER:
				return J_SPACE_SPATIAL_LAYER::DEBUG_OBJECT;
			default:
				return J_SPACE_SPATIAL_LAYER::INVALID;
			}
		} 
	}
}