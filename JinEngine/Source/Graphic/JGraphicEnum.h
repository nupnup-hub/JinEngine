#pragma once

namespace JinEngine
{
	namespace Graphic
	{
		enum class J_GRAPHIC_PROJECTION_TYPE
		{
			PERSPECTIVE,
			ORTHOLOGIC,
			COUNT
		};

		//contain draw & compute process
		enum class J_GRAPHIC_TASK_TYPE
		{
			SCENE_DRAW,
			SHADOW_MAP_DRAW,
			OUT_LINE_DRAW,
			DEPTH_MAP_DRAW,
			DEPTH_MAP_VISUALIZE,		//convert non linear to linear depth map (debug)
			NORMAL_MAP_VISUALIZE,
			APPLY_BLUR,
			APPLY_DOWN_SAMPLING,
			APPLY_SSAO,
			SSAO_VISUALIZE,
			//FRUSTUM_CULLING,		//mostly execute on cpu
			HZB_CULLING,
			HD_CULLING,
			LIGHT_CULLING,
			LIGHT_LIST_DRAW,		//visualize light list (debug)
		};
	}
}