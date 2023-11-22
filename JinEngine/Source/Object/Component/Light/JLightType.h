#pragma once
#include"../../../Core/Reflection/JReflection.h"
#include"../../../Graphic/FrameResource/JFrameResourceEnum.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceType.h"

namespace JinEngine
{
	REGISTER_ENUM_CLASS(J_LIGHT_TYPE, short, DIRECTIONAL = 0, POINT, SPOT, RECT)

	//affect shadow map texture size
	REGISTER_ENUM_CLASS(J_SHADOW_RESOLUTION, int, LOWEST = 256,
		LOW = 512,
		MEDIUM = 1024,
		HIGH = 2048,
		HIGHEST = 4096)
	
	enum class J_SHADOW_MAP_TYPE : int
	{ 
		NONE = -1, 
		NORMAL = 0,
		CSM,
		CUBE,
		COUNT
	}; 

	class JLightType
	{
	public:
		static J_SHADOW_MAP_TYPE LitToSmType(const J_LIGHT_TYPE litType, const bool canAllocCsm)
		{
			switch (litType)
			{
			case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
			{
				if (canAllocCsm)
					return J_SHADOW_MAP_TYPE::CSM;
				else
					return J_SHADOW_MAP_TYPE::NORMAL;
			}
			case JinEngine::J_LIGHT_TYPE::POINT:
				return J_SHADOW_MAP_TYPE::CUBE;
			case JinEngine::J_LIGHT_TYPE::SPOT:
				return J_SHADOW_MAP_TYPE::NORMAL;
			case JinEngine::J_LIGHT_TYPE::RECT:
				return J_SHADOW_MAP_TYPE::NORMAL;
			default:
				break;
			}
			return J_SHADOW_MAP_TYPE::NORMAL;
		}
		static Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE LitToFrameR(const J_LIGHT_TYPE litType)
		{
			switch (litType)
			{
			case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
				return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT;
			case JinEngine::J_LIGHT_TYPE::POINT:
				return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT;
			case JinEngine::J_LIGHT_TYPE::SPOT:
				return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT;
			case JinEngine::J_LIGHT_TYPE::RECT:
				return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT;
			default:
				break;
			}
			return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT;
		}
		static Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE SmToFrameR(const J_LIGHT_TYPE litType, const bool canAllocCsm)
		{
			return SmToFrameR(LitToSmType(litType, canAllocCsm));
		}
		static Graphic::J_GRAPHIC_RESOURCE_TYPE SmToGraphicR(const J_LIGHT_TYPE litType, const bool canAllocCsm)
		{
			return SmToGraphicR(LitToSmType(litType, canAllocCsm));
		}
		static Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE SmToFrameR(const J_SHADOW_MAP_TYPE smType)
		{
			switch (smType)
			{
			case JinEngine::J_SHADOW_MAP_TYPE::NORMAL:
				return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW;
			case JinEngine::J_SHADOW_MAP_TYPE::CSM:
				return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW;
			case JinEngine::J_SHADOW_MAP_TYPE::CUBE:
				return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW;
			default:
				break;
			}
			return Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW;
		}
		static Graphic::J_GRAPHIC_RESOURCE_TYPE SmToGraphicR(const J_SHADOW_MAP_TYPE smType)
		{
			switch (smType)
			{
			case JinEngine::J_SHADOW_MAP_TYPE::NORMAL:
				return Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
			case JinEngine::J_SHADOW_MAP_TYPE::CSM:
				return Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY;
			case JinEngine::J_SHADOW_MAP_TYPE::CUBE:
				return Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE;
			default:
				break;
			}
			return Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
		}
	};

}