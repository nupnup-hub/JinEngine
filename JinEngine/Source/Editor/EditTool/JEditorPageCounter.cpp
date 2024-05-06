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


#include"JEditorPageCounter.h"
#include"../Gui/JGui.h"

namespace JinEngine::Editor
{ 
	bool JEditorPageCounter::DisplayUI(const std::string& uniqueLabel)
	{
		bool isSelectedNewPageIndex = false;
		const uint maxPageIndex = GetMaxPageIndex();
		const JVector2F size(JGui::GetAlphabetSize().x * 2.0f, JGui::GetAlphabetSize().y);
		
		for (uint i = 0; i <= maxPageIndex; ++i)
		{ 
			if (pageIndex == i)
				JGui::PushTreeNodeColorSet(false, true, true);
			bool isSelected = JGui::Selectable(std::to_string(i + 1) + "##" + uniqueLabel, pageIndex == i, J_GUI_SELECTABLE_FLAG_NONE | J_GUI_SELECTABLE_SELECT_ON_CLICK, size);
			if (i != maxPageIndex)
				JGui::SameLine();
			if (pageIndex == i)
				JGui::PopTreeNodeColorSet(true, true);

			if (isSelected)
			{
				if (pageIndex != i)
					isSelectedNewPageIndex = true;
				pageIndex = i;
			}
		}
		if(isSelectedNewPageIndex)
			UpdateData();
		return isSelectedNewPageIndex;
	}
	uint JEditorPageCounter::GetPageInedx()const noexcept
	{
		return pageIndex;
	}
	uint JEditorPageCounter::GetMaxPageIndex()const noexcept
	{
		return count / pagePerCount;
	}
	void JEditorPageCounter::SetPageIndex(uint value)
	{
		pageIndex = value;
		UpdateData();
	}
	void JEditorPageCounter::SetPagePerCount(uint value)
	{
		pagePerCount = value;
		UpdateData();
	}
	void JEditorPageCounter::BeginCounting()
	{
		count = 0;
	}
	void JEditorPageCounter::EndCounting()
	{

	}
	void JEditorPageCounter::Count()
	{
		++count;
	}
	bool JEditorPageCounter::IsValidIndex()
	{
		return pageStartIndex <= count && count <= pageEndIndex;
	}
	void JEditorPageCounter::UpdateData()
	{
		pageStartIndex = pageIndex * pagePerCount;
		pageEndIndex = pageStartIndex + pagePerCount - 1;
	}
}