#pragma once
#include"../../JEditorWindow.h"   

namespace JinEngine
{ 
	namespace Editor
	{ 
		class JObjectDetail final : public JEditorWindow
		{
		private:
		public:
			JObjectDetail(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType);
			~JObjectDetail();
			JObjectDetail(const JObjectDetail& rhs) = delete;
			JObjectDetail& operator=(const JObjectDetail& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public: 
			void UpdateWindow()final; 
		};
	}
}