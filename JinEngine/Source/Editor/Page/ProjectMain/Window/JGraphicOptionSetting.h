#pragma once
#include"../../JEditorWindow.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute;
		class JGraphicOptionSetting : public JEditorWindow
		{   
		public:
			JGraphicOptionSetting(const std::string name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept;
		public:
			void UpdateWindow();
		};
	}
}