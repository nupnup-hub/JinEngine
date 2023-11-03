#pragma once
#include"../../Core/Reflection/JReflection.h"

namespace JinEngine
{
	namespace Graphic
	{
		REGISTER_ENUM_CLASS(J_GRAPHIC_BIND_TYPE, int, RTV = 0,
			DSV,
			SRV,
			UAV)

		//�߰��� Ÿ���߰��ҽ�
		//���� �ؽ��� ��Ÿ������ �ʱ�ȭ �ʿ� 
		//��Ÿ������ ��ȿ�� �˻��ʿ�
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_TYPE, int, SWAP_CHAN = 0,
			SCENE_LAYER_DEPTH_STENCIL,
			DEBUG_LAYER_DEPTH_STENCIL,
			LAYER_DEPTH_MAP_DEBUG,
			OCCLUSION_DEPTH_MAP,
			OCCLUSION_DEPTH_MIP_MAP,
			OCCLUSION_DEPTH_MAP_DEBUG, 
			TEXTURE_2D,
			TEXTURE_CUBE,
			RENDER_RESULT_COMMON, 
			SHADOW_MAP,
			SHADOW_MAP_ARRAY,
			SHADOW_MAP_CUBE,
			VERTEX,
			INDEX)

		//Option�� Bind View�� EnumClass�� ����� ������ ���ĵȴ�
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_OPTION_TYPE, int, POST_PROCESSING)

		class JGraphicResourceType
		{
		public:
			static bool CanUseOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE optionType, const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				switch (optionType)
				{
				case JinEngine::Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::POST_PROCESSING:
				{
					switch (rType)
					{ 
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
						return true;
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
						return true;
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY:
						return true;
					case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE:
						return true;
					default:
						return false;
					}
				}
				default:
					return false;
				}
			}	 
		};
	}
}