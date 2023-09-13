#include"JImGuiInitData.h"

namespace JinEngine::Editor
{
	Graphic::GuiIdentification JImGuiInitData::GetGuiIdentification()const noexcept
	{
		return (Graphic::GuiIdentification)J_GUI_TYPE::IM_GUI;
	}
}