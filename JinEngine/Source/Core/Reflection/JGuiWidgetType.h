#pragma once
namespace JinEngine
{
	namespace Core
	{
		enum class J_GUI_WIDGET_TYPE : int
		{
			NONE = 0,
			WINDOW,
			CHILD_WINDW,
			BUTTON,
			CHECKBOX,
			COMBO,
			COLOR_PICKER,
			DRAW,
			INPUT,
			IMAGE,
			LIST,
			MENU_BAR,
			MENU,
			MENU_ITEM,
			POPUP,
			READONLY_TEXT,
			SLIDER,
			SELECTALBE,
			SELECTOR,		//seletable + list
			SWITCH, 
			TAB_BAR,
			TAB_ITEM,
			TABLE, 
			TREE_NODE,
			TEXT,
			COUNT,
			//METHOD,	// ¹Ì±¸Çö
		};
	}
}