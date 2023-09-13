#include"JimGuiDrawData.h"

namespace JinEngine::Editor
{
	Graphic::GuiIdentification JImGuiDrawData::GetGuiIdentification()const noexcept
	{
		return (Graphic::GuiIdentification)J_GUI_TYPE::IM_GUI;
	}
}