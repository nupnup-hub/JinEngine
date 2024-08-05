/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../../../Core/Reflection/JReflection.h" 
#include"../../GraphicRule/FrameResource/JGraphicModuleFrameResourceType.h"
#include"../../GraphicRule/GraphicResource/JGraphicModuleTextureResourceType.h" 

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
		static J_FRAME_RESOURCE_UPLOAD_TYPE LitToFrameR(const J_LIGHT_TYPE litType)
		{
			switch (litType)
			{
			case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
				return J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT;
			case JinEngine::J_LIGHT_TYPE::POINT:
				return J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT;
			case JinEngine::J_LIGHT_TYPE::SPOT:
				return J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT;
			case JinEngine::J_LIGHT_TYPE::RECT:
				return J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT;
			default:
				break;
			}
			return J_FRAME_RESOURCE_UPLOAD_TYPE::DIRECTIONAL_LIGHT;
		}
		static J_FRAME_RESOURCE_UPLOAD_TYPE SmToFrameR(const J_LIGHT_TYPE litType, const bool canAllocCsm)
		{
			return SmToFrameR(LitToSmType(litType, canAllocCsm));
		}
		static J_GRAPHIC_RESOURCE_TYPE SmToGraphicR(const J_LIGHT_TYPE litType, const bool canAllocCsm)
		{
			return SmToGraphicR(LitToSmType(litType, canAllocCsm));
		}
		static J_FRAME_RESOURCE_UPLOAD_TYPE SmToFrameR(const J_SHADOW_MAP_TYPE smType)
		{
			switch (smType)
			{
			case JinEngine::J_SHADOW_MAP_TYPE::NORMAL:
				return J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW;
			case JinEngine::J_SHADOW_MAP_TYPE::CSM:
				return J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_ARRAY_DRAW;
			case JinEngine::J_SHADOW_MAP_TYPE::CUBE:
				return J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_CUBE_DRAW;
			default:
				break;
			}
			return J_FRAME_RESOURCE_UPLOAD_TYPE::SHADOW_MAP_DRAW;
		}
		static J_GRAPHIC_RESOURCE_TYPE SmToGraphicR(const J_SHADOW_MAP_TYPE smType)
		{
			switch (smType)
			{
			case JinEngine::J_SHADOW_MAP_TYPE::NORMAL:
				return J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
			case JinEngine::J_SHADOW_MAP_TYPE::CSM:
				return J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY;
			case JinEngine::J_SHADOW_MAP_TYPE::CUBE:
				return J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE;
			default:
				break;
			}
			return J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
		}
	public:
		static constexpr uint GetGlobalLightCount()
		{
			//Directinal light
			return 1;
		}
		static constexpr uint GetLocalLightCount()
		{
			//Point light, Spot light, Rect light
			return 3;
		}
		static constexpr uint GetLocalLightOffset()
		{
			return 1;
		}
	};

}