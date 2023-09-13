#pragma once
#include"JGuiUser.h"
namespace JinEngine
{
	namespace Editor
	{
		bool JGuiUser::IsSameGui(JGuiUser* user)const noexcept
		{
			return GetGuiType() == user->GetGuiType();
		}
		bool JGuiUser::IsSameGui(const J_GUI_TYPE type)const noexcept
		{
			return GetGuiType() == type;
		}
		bool JGuiUser::IsSameGui(const Graphic::GuiIdentification iden)const noexcept
		{
			return (Graphic::GuiIdentification)GetGuiType() == iden;
		}
	}
}