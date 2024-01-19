#pragma once
#include<string>
#include"../../../Core/Geometry/Mesh/JMeshType.h"

namespace JinEngine
{
	//has sequence dependency
	enum class J_DEFAULT_SHAPE
	{
		EMPTY = 0,
		CUBE,
		GRID,
		SPHERE,
		CYILINDER,
		QUAD,		//has to transform clip space
		FULL_SCREEN_QUAD,	//vertics range -1 ~ 1(xy), 0(z), hasn't to transform clip space
		LOW_CONE,
		LOW_SPHERE,
		LOW_HEMI_SPHERE,
		BOUNDING_BOX_LINE,
		BOUNDING_BOX_TRIANGLE,
		BOUNDING_FRUSTUM,
		POSITION_ARROW,
		CIRCLE,
		SCALE_ARROW,
		LINE,
		BOUNDING_CONE_LINE,
		DRAGON, 
		COUNT,
	};

	class JDefaultShape
	{ 
	public:
		static constexpr int debugTypeSt = (int)J_DEFAULT_SHAPE::BOUNDING_BOX_LINE;
		static bool IsDefaultUsed(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{ 
			case JinEngine::J_DEFAULT_SHAPE::CUBE:
				return true; 
			case JinEngine::J_DEFAULT_SHAPE::QUAD:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::FULL_SCREEN_QUAD:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::LOW_CONE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::LOW_SPHERE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::LOW_HEMI_SPHERE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::BOUNDING_BOX_LINE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::BOUNDING_FRUSTUM:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::LINE:
				return true; 
			default:
				return false;
			}
		}
		static bool IsExternalFile(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_SHAPE::POSITION_ARROW:
				return true;
			case JinEngine::J_DEFAULT_SHAPE::DRAGON:
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
			case JinEngine::J_DEFAULT_SHAPE::EMPTY:
				return L"Empty";
			case JinEngine::J_DEFAULT_SHAPE::CUBE:
				return L"Cube";
			case JinEngine::J_DEFAULT_SHAPE::GRID:
				return L"Grid";
			case JinEngine::J_DEFAULT_SHAPE::SPHERE:
				return L"Sphere";
			case JinEngine::J_DEFAULT_SHAPE::CYILINDER:
				return L"Cyilinder";
			case JinEngine::J_DEFAULT_SHAPE::QUAD:
				return L"Quad";
			case JinEngine::J_DEFAULT_SHAPE::FULL_SCREEN_QUAD:
				return L"FullScreenQuad";
			case JinEngine::J_DEFAULT_SHAPE::LOW_CONE:
				return L"LowCone";
			case JinEngine::J_DEFAULT_SHAPE::LOW_SPHERE:
				return L"LowSphere";
			case JinEngine::J_DEFAULT_SHAPE::LOW_HEMI_SPHERE:
				return L"LowHemiSphere";
			case JinEngine::J_DEFAULT_SHAPE::BOUNDING_BOX_LINE:
				return L"BoundingBox_L";
			case JinEngine::J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE:
				return L"BoundingBox_T";
			case JinEngine::J_DEFAULT_SHAPE::BOUNDING_FRUSTUM:
				return L"BoundingFrustum";
			case JinEngine::J_DEFAULT_SHAPE::POSITION_ARROW:
				return L"_SceneTransformArrow.fbx";
			case JinEngine::J_DEFAULT_SHAPE::CIRCLE:
				return L"Circle";
			case JinEngine::J_DEFAULT_SHAPE::SCALE_ARROW:
				return L"ScaleArrow";
			case JinEngine::J_DEFAULT_SHAPE::LINE:
				return L"Line";
			case JinEngine::J_DEFAULT_SHAPE::BOUNDING_CONE_LINE:
				return L"BoundingCone_L";
			case JinEngine::J_DEFAULT_SHAPE::DRAGON:
				return L"_Dragon.fbx"; 
			default:
				return L"Error";
			}
		}
		static Core::J_MESHGEOMETRY_TYPE GetMeshType(const J_DEFAULT_SHAPE type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_SHAPE::DRAGON:
				return Core::J_MESHGEOMETRY_TYPE::SKINNED;
			default:
				return Core::J_MESHGEOMETRY_TYPE::STATIC;
			}		
		}
	};	 
}