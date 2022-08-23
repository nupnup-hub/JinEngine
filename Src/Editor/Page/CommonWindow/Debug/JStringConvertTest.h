#pragma once
#include"../../JEditorWindow.h" 
#include"../../../../Utility/JVector.h"

namespace JinEngine
{ 
	namespace Editor
	{
		class JStringConvertTest : public JEditorWindow
		{
		private:
			std::string buf;

			std::string ori;
			std::wstring oriToWstr;
			std::string wstrToStr;
		public:
			JStringConvertTest(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid);
			~JStringConvertTest();
			JStringConvertTest(const JStringConvertTest& rhs) = delete;
			JStringConvertTest& operator=(const JStringConvertTest& rhs) = delete;

			bool Activate(JEditorUtility* editorUtility) final;
			bool DeActivate(JEditorUtility* editorUtility) final;
			void UpdateWindow(JEditorUtility* editorUtility)override;
		};
	}
}
