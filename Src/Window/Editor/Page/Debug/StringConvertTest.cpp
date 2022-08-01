#include"StringConvertTest.h"
#include"../EditorAttribute.h"  
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	StringConvertTest::StringConvertTest(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:EditorWindow(std::move(attribute), ownerPageGuid)
	{
		buf.resize(255);
	}
	StringConvertTest::~StringConvertTest() {}

	bool StringConvertTest::Activate(EditorUtility* editorUtility)
	{
		if (EditorWindow::Activate(editorUtility))
			return true;
		else
			return false;
	}
	bool StringConvertTest::DeActivate(EditorUtility* editorUtility)
	{
		if (EditorWindow::DeActivate(editorUtility))
			return true;
		else
			return false;
	}
	void StringConvertTest::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);

		ImGui::Text(u8"°á°ú");
		ImGui::Text(("ori: " + ori).c_str());
		ImGui::Text(("ori -> wstr -> str: " + wstrToStr).c_str());

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::NewLine();

		ImGuiInputTextFlags_ flag = (ImGuiInputTextFlags_)(ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::InputText("##TestInputText", &buf[0], 255, flag))
		{
			ori = JCommonUtility::EraseEmptySpace(buf);
			oriToWstr = JCommonUtility::U8StringToWstring(ori); 
			wstrToStr = JCommonUtility::WstringToU8String(oriToWstr);
			buf[0] = '\0';
		}
	}
}