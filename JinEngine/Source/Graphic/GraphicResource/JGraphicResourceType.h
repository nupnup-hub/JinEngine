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

		//중간에 타입추가할시
		//엔진 텍스쳐 메타데이터 초기화 필요 
		//메타데이터 유효성 검사필요
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
	}
}