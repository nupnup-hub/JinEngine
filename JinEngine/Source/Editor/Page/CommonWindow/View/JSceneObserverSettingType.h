#pragma once
#include<vector>
namespace JinEngine
{
	namespace Editor
	{
		enum class J_OBSERVER_SETTING_TYPE
		{
			//OPTION is control option panel
			//VIEW is rendering render result
			//TOOL is help editor

			OPTION_SPACE_SPATIAL,
			OPTION_EDITOR_OBSERVER,
			OPTION_INSTANCE_TEST,
			VIEW_SETTING_ACCELERATOR_TREE,
			VIEW_SHADOW_VIEWER,
			VIEW_RENDER_RESULT,
			VIEW_TEXTURE_DETAIL,
			VIEW_SCENE_COORD_GRID,
			VIEW_CAMERA,	 
			VIEW_LIGHT,
			TOOL_EDIT_GOBJ_POS,//TOOL
			TOOL_EDIT_GOBJ_ROT,
			TOOL_EDIT_GOBJ_SCALE, 
			TOOL_PLAY_SCENE_TIME,
			TOOL_PAUSE_SCENE_TIME,
			COUNT
		};

		namespace Constants
		{
			static std::vector<J_OBSERVER_SETTING_TYPE> GetAllObserverSetting()noexcept
			{
				std::vector<J_OBSERVER_SETTING_TYPE> res((int)J_OBSERVER_SETTING_TYPE::COUNT);
				for (int i = 0; i < (int)J_OBSERVER_SETTING_TYPE::COUNT; ++i)
					res[i] = (J_OBSERVER_SETTING_TYPE)i;
				return res;
			}
		}
	}
}