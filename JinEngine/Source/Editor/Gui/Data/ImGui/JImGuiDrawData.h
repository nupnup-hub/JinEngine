#pragma once 
#include"../../JGuiType.h"
#include"../../../../Graphic/Gui/JGuiData.h"
 
namespace JinEngine
{
	namespace Editor
	{
		struct JImGuiDrawData : public Graphic::JGuiDrawData
		{
		public:
			Graphic::GuiIdentification GetGuiIdentification()const noexcept final;
		};
	}
}