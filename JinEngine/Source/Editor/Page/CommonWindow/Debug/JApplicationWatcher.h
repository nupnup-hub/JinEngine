#pragma once
#include"../../JEditorWindow.h" 
#include"../../../../Utility/JVector.h"

namespace JinEngine
{
	namespace Editor
	{
		class JApplicationWatcher final : public JEditorWindow
		{
		public:
			JApplicationWatcher(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE pageType, 
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JApplicationWatcher();
			JApplicationWatcher(const JApplicationWatcher& rhs) = delete;
			JApplicationWatcher& operator=(const JApplicationWatcher& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void UpdateWindow()final;
		private:
			void DisplayTimeOnScreen();
			void DisplayMemoryUsageOnScreen();
		};
	}
}
