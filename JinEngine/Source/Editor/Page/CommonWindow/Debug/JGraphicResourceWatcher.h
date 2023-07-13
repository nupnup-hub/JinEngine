#pragma once
#include"../../JEditorWindow.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JGraphicResourceWatcher final : public JEditorWindow
		{ 
		public:
			JGraphicResourceWatcher(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JGraphicResourceWatcher();
			JGraphicResourceWatcher(const JGraphicResourceWatcher& rhs) = delete;
			JGraphicResourceWatcher& operator=(const JGraphicResourceWatcher& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void UpdateWindow()final;
		};
	}
}
