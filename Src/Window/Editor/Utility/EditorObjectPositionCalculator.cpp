#pragma once
#include"EditorObjectPositionCalculator.h"

namespace JinEngine
{
	void EditorObjectPositionCalculator::Update(float maxWidth, 
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
		EditorObjectPositionCalculator::maxWidth = maxWidth;
		EditorObjectPositionCalculator::maxHeight = maxHeight;
		EditorObjectPositionCalculator::width = width;
		EditorObjectPositionCalculator::height = height;
		EditorObjectPositionCalculator::positionX = positionX;
		EditorObjectPositionCalculator::positionY = positionY;
		EditorObjectPositionCalculator::xPadding = xPadding;
		EditorObjectPositionCalculator::yPadding = yPadding;
		EditorObjectPositionCalculator::border = border;
		EditorObjectPositionCalculator::textWidth = textWidth;
		EditorObjectPositionCalculator::textHeight = textHeight;
		xTextPadding = textWidth;
		yTextPadding = textHeight;
		//3มู - (...)
		maxValidTextCount = GetSameLineMaxTextCount() * 3 - 3;
	}

	void EditorObjectPositionCalculator::Next()noexcept
	{
		float plus = width + xPadding + border + textWidth;
		positionX += plus;
		if (positionX + plus >= maxWidth)
		{
			positionX = xPadding + border;
			positionY += height + yPadding + border + (textHeight * 3) + (yTextPadding * 2);
		}
	}
	float EditorObjectPositionCalculator::GetPositionX()noexcept
	{
		return positionX;
	}
	float EditorObjectPositionCalculator::GetPositionY()noexcept
	{
		return positionY;
	}
	float EditorObjectPositionCalculator::GetTextPositionX()noexcept
	{
		return positionX + xTextPadding;
	}
	float EditorObjectPositionCalculator::GetTextPositionY(int line)noexcept
	{
		return positionY + height + yTextPadding + (line * textHeight);
	}
	int EditorObjectPositionCalculator::GetSameLineMaxTextCount()noexcept
	{
		return (int)((width - xTextPadding * 2) / textWidth);
	}
	std::string EditorObjectPositionCalculator::GetString(const std::string& str)noexcept
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