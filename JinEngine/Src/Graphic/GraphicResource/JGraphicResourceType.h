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
			//�߰��ʿ�
			//��Ÿ������ ��ȿ�� �˻��ʿ�
		REGISTER_ENUM_CLASS(J_GRAPHIC_RESOURCE_TYPE, int, SWAP_CHAN = 0,
			SCENE_DEPTH_STENCIL,			
			SCENE_DEPTH_STENCIL_DEBUG, 
			DEBUG_DEPTH_STENCIL,
			OCCLUSION_DEPTH_MAP,
			OCCLUSION_DEPTH_MIP_MAP,
			OCCLUSION_DEPTH_MAP_DEBUG,  
			TEXTURE_2D,
			TEXTURE_CUBE,
			RENDER_RESULT_COMMON,
			SHADOW_MAP)

		/*
				enum class J_GRAPHIC_BIND_TYPE
		{
			RTV = 0,
			DSV,
			SRV,
			UAV,
			COUNT
		};
		enum class J_GRAPHIC_RESOURCE_TYPE
		{			
			//Engine only
			SWAP_CHAN = 0,
			SCENE_DEPTH_STENCIL,
			SCENE_DEPTH_STENCIL_DEBUG,
			OCCLUSION_DEPTH_MAP,
			OCCLUSION_DEPTH_MIP_MAP,
			OCCLUSION_DEPTH_MAP_DEBUG,

			//User share
			TEXTURE_2D,
			TEXTURE_CUBE,
			RENDER_RESULT_COMMON,
			SHADOW_MAP,
			COUNT
		};	 */
	}
}