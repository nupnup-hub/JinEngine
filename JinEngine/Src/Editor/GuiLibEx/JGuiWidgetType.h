#pragma once 

namespace JinEngine
{
	namespace Editor
	{ 
		enum J_GUI_WIDGET_TYPE : int
		{
			NONE = 0,  
			J_GUI_WIDGET_INPUT, 
			J_GUI_WIDGET_CHECKBOX,
			J_GUI_WIDGET_SLIDER,
			J_GUI_WIDGET_COLOR_PICKER,
			J_GUI_WIDGET_SELECTOR, 
			J_GUI_WIDGET_READONLY_TEXT,
			J_GUI_WIDGET_ENUM_COMBO,
			J_GUI_WIDGET_LIST,
			//METHOD,	// ¹Ì±¸Çö
		}; 
	}
}