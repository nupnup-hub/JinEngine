#include"JStringConvertTest.h"
#include"../../JEditorAttribute.h"  
#include"../../../Gui/JGui.h" 
#include"../../../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JStringConvertTest::JStringConvertTest(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute, 
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{
			buf.resize(255);
		}
		JStringConvertTest::~JStringConvertTest() {}

		J_EDITOR_WINDOW_TYPE JStringConvertTest::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::TEST_WINDOW;
		}
		void JStringConvertTest::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR);
			if (IsActivated())
			{
				UpdateMouseClick();
				JGui::Text(u8"°á°ú");
				JGui::Text(("ori: " + ori));
				JGui::Text(("ori -> wstr -> str: " + wstrToStr));

				JGui::NewLine();
				JGui::NewLine();
				JGui::NewLine();

				J_GUI_INPUT_TEXT_FLAG_ flag = J_GUI_INPUT_TEXT_FLAG_ENTER_RETURN_TRUE;

				if (JGui::InputText("##TestInputText", buf, 255, flag))
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