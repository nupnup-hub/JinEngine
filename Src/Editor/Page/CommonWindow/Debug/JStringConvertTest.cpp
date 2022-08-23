#include"JStringConvertTest.h"
#include"../../JEditorAttribute.h"  
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	namespace Editor
	{
		JStringConvertTest::JStringConvertTest(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JEditorWindow(std::move(attribute), ownerPageGuid)
		{
			buf.resize(255);
		}
		JStringConvertTest::~JStringConvertTest() {}

		bool JStringConvertTest::Activate()
		{
			if (JEditorWindow::Activate())
				return true;
			else
				return false;
		}
		bool JStringConvertTest::DeActivate()
		{
			if (JEditorWindow::DeActivate())
				return true;
			else
				return false;
		}
		void JStringConvertTest::UpdateWindow()
		{
			JEditorWindow::UpdateWindow();

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
}