#pragma once
#include"JGuiType.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Editor
	{
		struct JGuiOption
		{
		public:
			static constexpr float windowMinRate = 0.1f;
		public:
			J_GUI_TYPE guiType = J_GUI_TYPE::IM_GUI;
		public:
			uint textBufRange = 100;
		public:
			bool enablePopup = true;
			bool enableSelector = true; 
		};
	}
}