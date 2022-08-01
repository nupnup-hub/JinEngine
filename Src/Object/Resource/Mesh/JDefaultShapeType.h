#pragma once
#include<string>

namespace JinEngine
{
	enum class J_DEFAULT_SHAPE
	{
		DEFAULT_SHAPE_EMPTY = 1,
		DEFAULT_SHAPE_CUBE,
		DEFAULT_SHAPE_GRID,
		DEFAULT_SHAPE_SPHERE,
		DEFAULT_SHAPE_CYILINDER,
		DEFAULT_SHAPE_QUAD,
		DEFAULT_SHAPE_BOUNDING_BOX,
		DEFAULT_SHAPE_BOUNDING_FRUSTUM,
		COUNT,
	};

	struct JDefaultShape
	{ 
		static constexpr int debugTypeSt = (int)J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX;
		static std::string ConvertDefaultShapeName(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY:
				return "Empty";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE:
				return "Cube";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID:
				return "Grid";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE:
				return "Sphere";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER:
				return "Cyilinder";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD:
				return "Quad";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX:
				return "BoundingBox";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM:
				return "BoundingFrustum";
			default:
				return "Error";
			}
		}
	};	 
}