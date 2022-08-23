#include"JEditorWidgetPosCalculator.h"

namespace JinEngine
{
	namespace Editor
	{
		void JEditorWidgetPosCalculator::Update(float maxWidth,
			float maxHeight,
			float width,
			float height,
			float positionX,
			float positionY,
			float xPadding,
			float yPadding,
			float border,
			float textWidth,
			float textHeight)noexcept
		{
			JEditorWidgetPosCalculator::maxWidth = maxWidth;
			JEditorWidgetPosCalculator::maxHeight = maxHeight;
			JEditorWidgetPosCalculator::width = width;
			JEditorWidgetPosCalculator::height = height;
			JEditorWidgetPosCalculator::positionX = positionX;
			JEditorWidgetPosCalculator::positionY = positionY;
			JEditorWidgetPosCalculator::xPadding = xPadding;
			JEditorWidgetPosCalculator::yPadding = yPadding;
			JEditorWidgetPosCalculator::border = border;
			JEditorWidgetPosCalculator::textWidth = textWidth;
			JEditorWidgetPosCalculator::textHeight = textHeight;
			xTextPadding = textWidth;
			yTextPadding = textHeight;
			//3มู - (...)
			maxValidTextCount = GetSameLineMaxTextCount() * 3 - 3;
		}

		void JEditorWidgetPosCalculator::Next()noexcept
		{
			float plus = width + xPadding + border + textWidth;
			positionX += plus;
			if (positionX + plus >= maxWidth)
			{
				positionX = xPadding + border;
				positionY += height + yPadding + border + (textHeight * 3) + (yTextPadding * 2);
			}
		}
		float JEditorWidgetPosCalculator::GetPositionX()noexcept
		{
			return positionX;
		}
		float JEditorWidgetPosCalculator::GetPositionY()noexcept
		{
			return positionY;
		}
		float JEditorWidgetPosCalculator::GetTextPositionX()noexcept
		{
			return positionX + xTextPadding;
		}
		float JEditorWidgetPosCalculator::GetTextPositionY(int line)noexcept
		{
			return positionY + height + yTextPadding + (line * textHeight);
		}
		int JEditorWidgetPosCalculator::GetSameLineMaxTextCount()noexcept
		{
			return (int)((width - xTextPadding * 2) / textWidth);
		}
		std::string JEditorWidgetPosCalculator::GetString(const std::string& str)noexcept
		{
			const int strSize = (int)str.size();
			if (strSize >= maxValidTextCount)
			{
				std::string newStr = str.substr(0, maxValidTextCount) + "...";
				return newStr;
			}
			else
				return str;
		}
	}
}