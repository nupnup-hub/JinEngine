#pragma once
#include"../../JEditorWindow.h" 
#include"../../../../Utility/JVector.h"

namespace JinEngine
{
	namespace Editor
	{
		class JAppElapsedTime final : public JEditorWindow
		{
		public:
			JAppElapsedTime(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType);
			~JAppElapsedTime();
			JAppElapsedTime(const JAppElapsedTime& rhs) = delete;
			JAppElapsedTime& operator=(const JAppElapsedTime& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void UpdateWindow()final;
		};
	}
}
