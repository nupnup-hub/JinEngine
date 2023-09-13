#pragma once
#include"JGuiConstants.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		//Is Same as {J_GUI_TYPE, ...} 
		struct JGuiInitData : public JGraphicDeviceUser, public Core::JValidInterface
		{ 
		public:
			virtual GuiIdentification GetGuiIdentification()const noexcept = 0;
		};
		 
		struct JGuiDrawData : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			virtual GuiIdentification GetGuiIdentification()const noexcept = 0;
		};
	}
}