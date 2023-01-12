#include"JStringConvertTest.h"
#include"../../JEditorAttribute.h"  
#include"../../../../Utility/JCommonUtility.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Editor
	{
		JStringConvertTest::JStringConvertTest(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{
			buf.resize(255);
		}
		JStringConvertTest::~JStringConvertTest() {}

		J_EDITOR_WINDOW_TYPE JStringConvertTest::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::TEST_WINDOW;
		}
		void JStringConvertTest::UpdateWindow(const JEditorWindowUpdateCondition& condition)
		{
			EnterWindow(condition, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			if (IsActivated())
			{
				JImGuiImpl::Text(u8"°á°ú");
				JImGuiImpl::Text(("ori: " + ori));
				JImGuiImpl::Text(("ori -> wstr -> str: " + wstrToStr));

				ImGui::NewLine();
				ImGui::NewLine();
				ImGui::NewLine();

				ImGuiInputTextFlags_ flag = (ImGuiInputTextFlags_)(ImGuiInputTextFlags_EnterReturnsTrue);

				if (ImGui::InputText("##TestInputText", &buf[0], 255, flag))
				{
					ori = JCUtil::EraseChar(buf, ' ');
					oriToWstr = JCUtil::U8StrToWstr(ori);
					wstrToStr = JCUtil::WstrToU8Str(oriToWstr);
					buf[0] = '\0';
				}
			}
			CloseWindow();
		}
	}
}