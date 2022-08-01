#include"ImGuiEx.h"

namespace JinEngine
{
	bool ImGuiEx::Contain(const ImVec2& pos, const ImVec2& size, const ImVec2& point)noexcept
	{ 
		return point.x >= pos.x && point.x <= pos.x + size.x &&
			point.y >= pos.y && point.y <= pos.y + size.y;
	}
}