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


#include"JEditorSliderBar.h"
#include"../Gui/JGui.h"

namespace JinEngine::Editor
{
	JEditorSliderBar::JEditorSliderBar(float value, float minValue, float maxValue)
		:var(Core::JRestrictedRangeVar<float>(value, minValue, maxValue))
	{}
	bool JEditorSliderBar::Update(const std::string& label, const float formatDigit, const bool isRight, const bool displayText)
	{ 
		float width = JGui::GetSliderWidth();
		float restWidth = JGui::GetRestWindowContentsSize().x;
		if (width > restWidth)
			width = restWidth;

		if (isRight)
			JGui::SetCursorPosX(JGui::GetSliderRightAlignPosX(width));
		JGui::SetNextItemWidth(width);
		bool res = JGui::SliderFloat(label, var.GetPtr(), var.GetMin(), var.GetMax(), formatDigit, J_GUI_SLIDER_FLAG_ALWAYS_CLAMP | J_GUI_SLIDER_FLAG_NO_INPUT);
		if (displayText &&(JGui::IsLastItemActivated() || JGui::IsLastItemHovered()))
			JGui::Tooltip(var.Get(), formatDigit == 0 ? 1 : formatDigit);
		return res;
	}
	float JEditorSliderBar::GetValue()const noexcept
	{
		return var.Get();
	}
	void JEditorSliderBar::SetValue(float value)
	{
		var.Set(value);
	}
	void JEditorSliderBar::SetMinValue(float value)
	{
		var.SetMin(value);
	}
	void JEditorSliderBar::SetMaxValue(float value)
	{
		var.SetMax(value);
	}
}