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


#include"JEditorSearchBar.h"
#include"../Gui/JGui.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Object/JObject.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorSearchBar::JEditorSearchBar(const bool caseSensitive)
			:guid(Core::MakeGuid()), caseSensitive(caseSensitive)
		{
			inputHelper = std::make_unique<JEditorInputBuffHelper>(JGui::GetTextBuffRange());
		}
		void JEditorSearchBar::ClearInputBuffer()noexcept
		{
			inputHelper->Clear(); 
		}
		void JEditorSearchBar::UpdateSearchBar(const bool isReadonly)
		{ 
			J_GUI_INPUT_TEXT_FLAG_ flag = J_GUI_INPUT_TEXT_FLAG_NONE;
			if (isReadonly)
				flag = J_GUI_INPUT_TEXT_FLAG_READ_ONLY;
			 
			const uint preCount = (uint)inputHelper->result.size();
			const JVector2<float> preFramePadding = JGui::GetFramePadding();
			JGui::SetFramePadding({ JGui::GetFramePadding().x , JGui::GetFramePadding().y * 0.8f });
			 
			const JVector2<float> prePos = JGui::GetCursorScreenPos();
			JGui::InputText("##" +std::to_string(guid), inputHelper->buff, inputHelper->result, "Search...", flag);
			inputHelper->result = JCUtil::EraseLeftSideChar(inputHelper->result, ' ');

			if (inputHelper->result.size() != preCount)
				isUpdateInputData = true;
			else
				isUpdateInputData = false;
			  
			const JVector2<float> inputSize = JGui::GetLastItemRectSize();
			JGui::SetFramePadding(preFramePadding); 
		}
		std::string JEditorSearchBar::GetInputData()const noexcept
		{
			return inputHelper->result;
		}
		bool JEditorSearchBar::CanSrcNameOnScreen(const std::string& srcName)const noexcept
		{
			if (HasInputData())
				return JCUtil::Contain(srcName, inputHelper->result, caseSensitive);
			else
				return true;
		}
		bool JEditorSearchBar::CanSrcNameOnScreen(const std::wstring& srcName)const noexcept
		{
			if (HasInputData())
				return JCUtil::Contain(JCUtil::WstrToU8Str(srcName), inputHelper->result, caseSensitive);
			else
				return true;
		}
		bool JEditorSearchBar::HasInputData()const noexcept
		{
			return inputHelper->result.size() > 0;
		}
		bool JEditorSearchBar::IsUpdateInputData()const noexcept
		{
			return isUpdateInputData;
		}
	}
}