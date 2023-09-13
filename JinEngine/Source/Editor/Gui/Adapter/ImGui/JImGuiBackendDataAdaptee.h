#pragma once
#include"../../JGuiType.h"
#include"../../../../Graphic/Gui/JGuiBackendDataAdaptee.h"

namespace JinEngine
{
	namespace Editor
	{
		class JImGuiBackendDataAdaptee : public Graphic::JGuiBackendDataAdaptee
		{
		public: 
			Graphic::GuiIdentification GetGuiIdentification()const noexcept final;
		};
	}
}