#pragma once
#include"../EditorWindow.h" 
#include"../../../../Utility/Vector.h"

namespace JinEngine
{
	class JCamera;
	class EditorCameraControl;
	class StringConvertTest : public EditorWindow
	{ 
	private:
		std::string buf;

		std::string ori;
		std::wstring oriToWstr;
		std::string wstrToStr;
	public:
		StringConvertTest(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid);
		~StringConvertTest();
		StringConvertTest(const StringConvertTest& rhs) = delete;
		StringConvertTest& operator=(const StringConvertTest& rhs) = delete;

		bool Activate(EditorUtility* editorUtility) final;
		bool DeActivate(EditorUtility* editorUtility) final; 
		void UpdateWindow(EditorUtility* editorUtility)override;
	};
}
