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