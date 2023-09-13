#pragma once
#include"../../JGuiType.h"
#include"../../../../Graphic/Gui/JGuiData.h"

namespace JinEngine
{
	namespace Editor
	{
		struct JImGuiInitData : public Graphic::JGuiInitData
		{
		public:
			Graphic::GuiIdentification GetGuiIdentification()const noexcept final;
		};
	}
}