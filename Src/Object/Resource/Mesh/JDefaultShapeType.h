#pragma once
#include<string>

namespace JinEngine
{
	enum class J_DEFAULT_SHAPE
	{
		DEFAULT_SHAPE_EMPTY = 0,
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
		static std::wstring ConvertDefaultShapeName(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY:
				return L"Empty";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE:
				return L"Cube";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID:
				return L"Grid";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE:
				return L"Sphere";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER:
				return L"Cyilinder";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD:
				return L"Quad";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX:
				return L"BoundingBox";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM:
				return L"BoundingFrustum";
			default:
				return L"Error";
			}
		}
	};	 
}