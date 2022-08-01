#pragma once
#include<string>

namespace JinEngine
{
	class EditorObjectPositionCalculator
	{
	private:
		float maxWidth;
		float maxHeight;

		float width;
		float height;
		float positionX;
		float positionY;

		float xPadding;
		float yPadding;
		float border;
		float xTextPadding;
		float yTextPadding;
		float textWidth;
		float textHeight;
		int maxValidTextCount;
	public:
		void Update(float maxWidth, 
			float maxHeight, 
			float width, 
			float height,
			float positionX,
			float positionY,
			float xPadding,
			float yPadding,
			float border,
			float textWidth,
			float textHeight)noexcept;
		void Next()noexcept;
		float GetPositionX()noexcept;
		float GetPositionY()noexcept;
		float GetTextPositionX()noexcept;
		float GetTextPositionY(int line)noexcept;
		int GetSameLineMaxTextCount()noexcept;
		std::string GetString(const std::string& str)noexcept;
	};
}