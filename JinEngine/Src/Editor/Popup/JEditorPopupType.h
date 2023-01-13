#pragma once
namespace JinEngine
{
	namespace Editor
	{
		enum class J_EDITOR_POPUP_NODE_TYPE
		{
			ROOT,
			INTERNAL,
			LEAF,
			LEAF_TOGGLE,
			LEAF_CHECK_BOX
		};
		enum class J_EDITOR_POPUP_NODE_RES
		{
			NON_CLICK,
			CLICK_SLELECT_NODE,
			CLICK_TOGGLE_TO_ON,
			CLICK_TOGGLE_TO_OFF,
		};

		enum class J_EDITOR_POPUP_WINDOW_TYPE
		{
			CAUTION,
			CLOSE_CONFIRM
		};
	}
}