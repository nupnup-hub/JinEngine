#include"JEditorAlignCalculator.h" 
namespace JinEngine
{
	namespace Editor
	{
		void JEditorStaticAlignCalculator::Update(const JVector2<float>& canvasSize,
			const JVector2<float>& contentsSize,
			const JVector2<float>& padding,
			const JVector2<float>& spacing,
			const JVector2<float>& startPos)noexcept
		{
			JEditorStaticAlignCalculator::padding = padding;
			JEditorStaticAlignCalculator::spacing = spacing;
			JEditorStaticAlignCalculator::canvasSize = canvasSize;
			JEditorStaticAlignCalculator::contentsSize = contentsSize;

			float leftWidth = (canvasSize.x - (padding.x * 2));
			float contentsWidth = contentsSize.x;
			while (leftWidth >= contentsSize.x)
			{
				++columnCount;
				leftWidth -= contentsWidth;
				if (contentsWidth == contentsSize.x)
					contentsWidth += spacing.x;
			}

			columnIndex = rowIndex = 0;
			contentsStartCursor = startPos + padding;
		}
		float JEditorStaticAlignCalculator::GetCursorPosX()const noexcept
		{
			return contentsStartCursor.x + columnIndex * spacing.x + columnIndex * contentsSize.x;
		}
		float JEditorStaticAlignCalculator::GetCursorPosY()const noexcept
		{
			return contentsStartCursor.y + rowIndex * spacing.y + rowIndex * contentsSize.y;
		}
		JVector2<float> JEditorStaticAlignCalculator::GetCursorPos()const noexcept
		{
			return JVector2<float>(GetCursorPosX(), GetCursorPosY());
		}
		void JEditorStaticAlignCalculator::SetNextContentsPosition()noexcept
		{
			JGui::SetCursorPos(GetCursorPos());
			Next();
		}
		void JEditorStaticAlignCalculator::Next()noexcept
		{
			++columnIndex;
			if (columnIndex >= columnCount)
			{
				columnIndex = 0;
				++rowIndex;
			}
		}

		void JEditorTextAlignCalculator::Update(const std::string& text, const JVector2<float>& size, const bool useCompress)
		{
			const JVector2<float> alphaSize = JGui::GetAlphabetSize();
			JEditorTextAlignCalculator::text = JCUtil::EraseChar(JCUtil::EraseChar(text, '\n'), '\t');
			linePerAlpabet = static_cast<int>(size.x / alphaSize.x);
			lineLength = size.x;

			JEditorTextAlignCalculator::size = size;
			JEditorTextAlignCalculator::useCompress = useCompress;
			if (size.y == 0)
				JEditorTextAlignCalculator::useCompress = false;
		}
		std::string JEditorTextAlignCalculator::LeftAligned()
		{
			return Aligned(J_EDITOR_ALIGN_TYPE::LEFT);
		}
		std::string JEditorTextAlignCalculator::MiddleAligned()
		{
			return Aligned(J_EDITOR_ALIGN_TYPE::MID);
		}
		std::string JEditorTextAlignCalculator::RightAligned()
		{
			return Aligned(J_EDITOR_ALIGN_TYPE::RIGHT);
		}
		std::string JEditorTextAlignCalculator::Aligned(const J_EDITOR_ALIGN_TYPE dir)
		{
			const JVector2<float> alphaSize = JGui::GetAlphabetSize();
			const JVector2<float> spaceSize = JGui::CalTextSize(" ");
			if (lineLength <= alphaSize.x * 2.0f)
				return text;

			std::string srcText = text;
			std::string result = "";
			const float addtionalXFactor = dir == J_EDITOR_ALIGN_TYPE::LEFT ? 0.0f : (dir == J_EDITOR_ALIGN_TYPE::MID ? 0.5f : 1.0f);
			int lineCount = 0;
			 
			const float fontSize = JGui::GetFontSize();
			while (!srcText.empty())
			{
				uint subStrCount = CalTextLengthRange(srcText, lineLength); 
				std::string subStr = srcText.substr(0, subStrCount);
				srcText = srcText.substr(subStrCount);

				if (useCompress)
				{
					const float nextYEndPos = fontSize * (lineCount + 2);
					if (nextYEndPos >= size.y && !srcText.empty())
					{
						if (subStrCount > 0)
						{
							int swapPerioCount = subStrCount > 2 ? 3 : subStrCount;
							for (int i = 1; i <= swapPerioCount; ++i)
								subStr[subStrCount - i] = '.';

							int additionalPeriod = 3 - swapPerioCount;
							for (int i = 0; i < additionalPeriod; ++i)
								subStr.push_back('.');
						}
						srcText.clear();
					}
				}

				const float length = JGui::CalTextSize(subStr).x;
				const float additionalXPos = (lineLength - length) * addtionalXFactor;
				if (additionalXPos > 0)
				{
					int additionalSpace = static_cast<int>(additionalXPos / spaceSize.x);
					subStr.insert(0, std::string(additionalSpace, ' '));
				}

				result += subStr;
				if (!srcText.empty())
					result.push_back('\n');
				++lineCount;
			}
			return result;
		}
		uint JEditorTextAlignCalculator::CalTextLengthRange(const std::string& calText, const float length)const
		{
			const uint loopCount = (uint)calText.size();
			for (uint i = 0; i < loopCount; ++i)
			{
				if (JGui::CalTextSize(calText.substr(0, i + 1)).x >= length)
					return i;
			}
			return loopCount;
		}
		uint JEditorTextAlignCalculator::CalTextAreaRange(const std::string& calText, const float area)const
		{
			const uint loopCount = (uint)calText.size();
			for (uint i = 0; i < loopCount; ++i)
			{
				JVector2<float> size = JGui::CalTextSize(calText.substr(0, i + 1));
				if (size.x * size.y >= area)
					return i;
			}
			return loopCount;
		}

		void JEditorCursorPosCalculator::SetMiddline(const JVector2<float>& pos,
			const JVector2<float>& size,
			const JVector2<float>& constentsSize,
			const JVector2<float>& padding,
			const bool useWorldCursor)
		{
			JVector2<float> cursorPos = pos + JVector2<float>(0, (size.y - constentsSize.y) * 0.5f) + padding;
			if (useWorldCursor)
				JGui::SetCursorScreenPos(cursorPos);
			else
				JGui::SetCursorPos(cursorPos);
		}
	}
}