#pragma once
#include"JGuiType.h"
#include"../../Graphic/Gui/JGuiConstants.h"
namespace JinEngine
{
	namespace Editor
	{
		class JGuiUser
		{
		public:
			virtual ~JGuiUser() = default;
		public:
			virtual J_GUI_TYPE GetGuiType()const noexcept = 0;
		public:
			bool IsSameGui(JGuiUser* user)const noexcept;
			bool IsSameGui(const J_GUI_TYPE type)const noexcept;
			bool IsSameGui(const Graphic::GuiIdentification iden)const noexcept;
		};
	}
}