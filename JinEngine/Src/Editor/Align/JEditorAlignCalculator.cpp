#include"JEditorAlignCalculator.h" 

namespace JinEngine
{
	namespace Editor
	{
		void JEditorTextAlignCalculator::Update(const std::string& text, const JVector2<float>& size, const bool useCompress)
		{
			const JVector2<float> alphaSize = JImGuiImpl::GetAlphabetSize();
			JEditorTextAlignCalculator::text = JCUtil::EraseChar(text, '\n');
			linePerAlpabet = size.x / alphaSize.x;
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
			const JVector2<float> alphaSize = JImGuiImpl::GetAlphabetSize();
			const JVector2<float> spaceSize = ImGui::CalcTextSize(" ");
			std::string srcText = text;
			std::string result;
			const float addtionalXFactor = dir == J_EDITOR_ALIGN_TYPE::LEFT ? 0.0f : (dir == J_EDITOR_ALIGN_TYPE::MID ? 0.5f : 1.0f);
			int lineCount = 0;

			ImGuiContext& context = *GImGui;
			while (!srcText.empty())
			{
				const uint subStrCount = CalTextLengthRange(srcText, lineLength);
				std::string subStr = srcText.substr(0, subStrCount);
				srcText = srcText.substr(subStrCount);

				if (useCompress)
				{
					const float nextYEndPos = context.FontSize * (lineCount + 2);
					if (nextYEndPos >= size.y && !srcText.empty())
					{
						subStr[subStrCount - 1] = '.';
						subStr[subStrCount - 2] = '.';
						subStr[subStrCount - 3] = '.';
						srcText.clear();
					}
				}

				const float length = ImGui::CalcTextSize(subStr.c_str()).x;
				const float additionalXPos = (lineLength - length) * addtionalXFactor;
				if (additionalXPos > 0)
				{
					int additionalSpace = additionalXPos / spaceSize.x;
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
				if (ImGui::CalcTextSize(calText.substr(0, i + 1).c_str()).x >= length)
					return i;
			}
			return loopCount;
		}
		uint JEditorTextAlignCalculator::CalTextAreaRange(const std::string& calText, const float area)const
		{
			const uint loopCount = (uint)calText.size();
			for (uint i = 0; i < loopCount; ++i)
			{
				JVector2<float> size = ImGui::CalcTextSize(calText.substr(0, i + 1).c_str());
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
				ImGui::SetCursorScreenPos(cursorPos);
			else
				ImGui::SetCursorPos(cursorPos);
		}
	}
}