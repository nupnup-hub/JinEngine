#pragma once 
#include"../../../Lib/imgui/imgui.h"

namespace JinEngine
{  
	class ImGuiEx
	{
	public:
		static bool Contain(const ImVec2& pos, const ImVec2& size, const ImVec2& point)noexcept;
	};
}