#include"JEditorWidgetPosCalculator.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../Utility/JCommonUtility.h"

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
		void JEditorWidgetPosCalculator::Update(float maxWidth, float maxHeight, float width, float height)noexcept
		{
			JEditorWidgetPosCalculator::maxWidth = maxWidth;
			JEditorWidgetPosCalculator::maxHeight = maxHeight;
			JEditorWidgetPosCalculator::width = width;
			JEditorWidgetPosCalculator::height = height;

			ImVec2 nowCursor = JImGuiImpl::GetCursorPos();
			ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;
			float frameBorder = ImGui::GetStyle().FrameBorderSize;
			ImVec2 txtSize = JImGuiImpl::GetTextSize();

			JEditorWidgetPosCalculator::positionX = nowCursor.x;
			JEditorWidgetPosCalculator::positionY = nowCursor.y;
			JEditorWidgetPosCalculator::xPadding = itemSpacing.x;
			JEditorWidgetPosCalculator::yPadding = itemSpacing.y;
			JEditorWidgetPosCalculator::border = frameBorder;
			JEditorWidgetPosCalculator::textWidth = txtSize.x;
			JEditorWidgetPosCalculator::textHeight = txtSize.y;

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
		void JEditorWidgetPosCalculator::TitleText(const std::wstring& name)
		{
			std::string oriName = JCUtil::WstrToU8Str(name);
			std::string subName;
			int maxTextCount = GetSameLineMaxTextCount();
			int lineCount = 0;

			ImGui::SetCursorPos(ImVec2(GetTextPositionX(), GetTextPositionY(0)));
			while (oriName.size() > maxTextCount)
			{
				++lineCount;
				subName = oriName.substr(maxTextCount, oriName.size());
				JImGuiImpl::Text(oriName.substr(0, maxTextCount));
				ImGui::SetCursorPos(ImVec2(GetTextPositionX(), GetTextPositionY(lineCount)));
				oriName = subName;
			}
			JImGuiImpl::Text(oriName); 
		}
	}
}