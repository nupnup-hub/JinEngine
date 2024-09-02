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
#include"JGraphicResourceConstants.h" 
#include"../../Object/GraphicRule/GraphicResource/JGraphicModuleTextureResourceType.h"
#include"../../Object/GraphicRule/JGraphicModuleType.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceTypeAttribute
		{
		public:
			static bool CanUseOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE optionType, const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (optionType)
				{
				case JinEngine::J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP:
				case JinEngine::J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY:
				case JinEngine::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP:
				case JinEngine::J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY:
				{
					switch (rType)
					{
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
						return true; 
					default:
						return false;
					}
				}
				case JinEngine::J_GRAPHIC_RESOURCE_OPTION_TYPE::BLUR:
				{
					switch (rType)
					{
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
						return true;
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
						return true;
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
						return true;
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
						return true;
					default:
						return false;
					}
				}
				case JinEngine::J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNTER_BUFFER:
				{
					switch (rType)
					{
					case JinEngine::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
						return true;
					default:
						return false;
					}
				};
				default:
					return false;
				}
			}
			/**
			* @brief create graphic resource and swap exist JGraphicResourceInfo graphic resource
			* 주로 user ptr을 소유하는 객체가 아닌 다른 요소에 의해서 graphic resource만 재할당이 필요한경우 사용된다
			* ex) graphic option에 cluster option이 변경될경우 light cluster관련 resource 전부 재할당.
			*/
			static bool CanReAlloc(const J_GRAPHIC_RESOURCE_TYPE type)
			{
				switch (type)
				{
				case JinEngine::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
					return true;
				case JinEngine::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_LIGHT_CULLING:
					return true;
				case JinEngine::J_GRAPHIC_RESOURCE_TYPE::LIGHT_LINKED_LIST:
					return true;
				case JinEngine::J_GRAPHIC_RESOURCE_TYPE::LIGHT_OFFSET:
					return true;
				default:
					return false;
				}
			}
		};
	}
}