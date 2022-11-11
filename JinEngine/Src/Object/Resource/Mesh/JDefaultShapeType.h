#pragma once
#include<string>
#include"JMeshType.h"
namespace JinEngine
{
	//has sequence dependency
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
	public:
		static constexpr int debugTypeSt = (int)J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX;
		static bool IsDefaultUse(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY:
				return false;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE:
				return false;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID:
				return false;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE:
				return false;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER:
				return false;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD:
				return false;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM:
				return false;
			default:
				return L"Error";
			}
		}
		static std::wstring ConvertToName(const J_DEFAULT_SHAPE type)
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
		static J_MESHGEOMETRY_TYPE GetMeshType(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_EMPTY:
				return J_MESHGEOMETRY_TYPE::STATIC;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE:
				return J_MESHGEOMETRY_TYPE::STATIC;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID:
				return J_MESHGEOMETRY_TYPE::STATIC;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE:
				return J_MESHGEOMETRY_TYPE::STATIC;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER:
				return J_MESHGEOMETRY_TYPE::STATIC;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD:
				return J_MESHGEOMETRY_TYPE::STATIC;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX:
				return J_MESHGEOMETRY_TYPE::STATIC;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM:
				return J_MESHGEOMETRY_TYPE::STATIC;
			default:
				return J_MESHGEOMETRY_TYPE::STATIC;
			}
		}

	};	 
}