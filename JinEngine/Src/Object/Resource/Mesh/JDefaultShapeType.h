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
		DEFAULT_SHAPE_BOUNDING_BOX_LINE,
		DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE,
		DEFAULT_SHAPE_BOUNDING_FRUSTUM,
		DEFAULT_SHAPE_POSITION_ARROW,
		DEFAULT_SHAPE_CIRCLE,
		DEFAULT_SHAPE_SCALE_ARROW,
		DEFAULT_SHAPE_LINE,
		COUNT,
	};

	struct JDefaultShape
	{ 
	public:
		static constexpr int debugTypeSt = (int)J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE;
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
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_POSITION_ARROW:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CIRCLE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_SCALE_ARROW:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_LINE:
				return true;
			default:
				return L"Error";
			}
		}
		static bool IsExternalFile(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_POSITION_ARROW:
				return true;
			default:
				return false;
			}
		}
		//Has format mesh is extenal file
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
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_LINE:
				return L"BoundingBox_L";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX_TRIANGLE:
				return L"BoundingBox_T";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM:
				return L"BoundingFrustum";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_POSITION_ARROW:
				return L"_SceneTransformArrow.fbx";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_CIRCLE:
				return L"Circle";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_SCALE_ARROW:
				return L"ScaleArrow";
			case JinEngine::J_DEFAULT_SHAPE::DEFAULT_SHAPE_LINE:
				return L"Line";
			default:
				return L"Error";
			}
		}
		static J_MESHGEOMETRY_TYPE GetMeshType(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			default:
				return J_MESHGEOMETRY_TYPE::STATIC;
			}
		}

	};	 
}