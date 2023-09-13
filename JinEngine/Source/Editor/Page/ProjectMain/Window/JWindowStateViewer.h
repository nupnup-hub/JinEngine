#pragma once 
#include"../../JEditorWindow.h"
#include<vector>

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute;
		class JWindowStateViewer : public JEditorWindow
		{
		private:
			std::vector<JEditorWindow*> wndVec; 
		public:
			JWindowStateViewer(const std::string name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept;
		public:
			void Initialize(const std::vector<JEditorWindow*>& newWndVec);
		public:
			void UpdateWindow()final;
		};
	}
}