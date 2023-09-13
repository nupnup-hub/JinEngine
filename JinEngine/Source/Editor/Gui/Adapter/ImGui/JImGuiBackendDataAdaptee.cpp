#include"JImGuiBackendDataAdaptee.h"

namespace JinEngine::Editor
{
	Graphic::GuiIdentification JImGuiBackendDataAdaptee::GetGuiIdentification()const noexcept
	{
		return (Graphic::GuiIdentification)J_GUI_TYPE::IM_GUI;
	}
}