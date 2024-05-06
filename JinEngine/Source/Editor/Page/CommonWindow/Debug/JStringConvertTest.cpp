/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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