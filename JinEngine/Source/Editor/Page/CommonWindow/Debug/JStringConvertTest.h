#pragma once
#include"../../JEditorWindow.h" 
#include"../../../../Utility/JVector.h"

namespace JinEngine
{ 
	namespace Editor
	{
		class JStringConvertTest final : public JEditorWindow
		{ 
		private:
			std::string buf;
			std::string ori;
			std::wstring oriToWstr;
			std::string wstrToStr;
		public:
			JStringConvertTest(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute, 
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JStringConvertTest();
			JStringConvertTest(const JStringConvertTest& rhs) = delete;
			JStringConvertTest& operator=(const JStringConvertTest& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void UpdateWindow()final;
		};
	}
}
