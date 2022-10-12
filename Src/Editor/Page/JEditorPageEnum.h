#pragma once
namespace JinEngine
{
	namespace Editor
	{
		enum class J_EDITOR_PAGE_TYPE
		{
			PROJECT_SELECTOR = 0,
			PROJECT_MAIN,
			SKELETON_SETTING,
		};

		enum J_EDITOR_PAGE_FLAG
		{
			J_EDITOR_PAGE_NONE = 0,
			J_EDITOR_PAGE_SUPPORT_DOCK = 1 << 0,
			J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING = 1 << 1,
			J_EDITOR_PAGE_REQUIRE_INIT_OBJECT = 1 << 2,
		};
	}
}