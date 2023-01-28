#include"JEditorLineCalculator.h"
namespace JinEngine
{
	namespace Editor
	{ 
		void JEditorTextLineCalculator::Update(const std::string& text, const JVector2<float>& size, const JVector2<float>& cursorPos, const bool useLocalCursor)
		{
			const JVector2<float> alphaSize = JImGuiImpl::GetAlphabetSize();
			JEditorTextLineCalculator::text = text;
			linePerAlpabet = size.x / alphaSize.x;
			lineLength = size.x;		 

			JEditorTextLineCalculator::size = size;
			JEditorTextLineCalculator::cursorPos = cursorPos;
			JEditorTextLineCalculator::useLocalCursor = useLocalCursor;
		}
		void JEditorTextLineCalculator::LeftAligned()
		{
			Aligned(ALIGNED_DIR::LEFT);
		}
		void JEditorTextLineCalculator::MiddleAligned()
		{
			Aligned(ALIGNED_DIR::MIDDLE);
		}
		void JEditorTextLineCalculator::RightAligned()
		{
			Aligned(ALIGNED_DIR::RIGHT);
		}
		void JEditorTextLineCalculator::Aligned(const ALIGNED_DIR dir)
		{
			const JVector2<float> alphaSize = JImGuiImpl::GetAlphabetSize();
			const JVector2<float> spaceSize = ImGui::CalcTextSize(" ");
			std::string srcText = text;
			std::string result;
			const float addtionalXFactor = dir == ALIGNED_DIR::LEFT ? 0.0f : (dir == ALIGNED_DIR::MIDDLE ? 0.5f : 1.0f);
			int lineCount = 0;

			while (!srcText.empty())
			{
				const uint subStrCount = CalTextLengthRange(srcText, lineLength);
				std::string subStr = srcText.substr(0, subStrCount);
				srcText = srcText.substr(subStrCount);

				const float nextYPos = ImGui::CalcTextSize(std::string(lineCount + 1, '\n').c_str()).y;
				if (nextYPos >= size.y)
				{
					subStr[subStrCount - 1] = '.';
					subStr[subStrCount - 2] = '.';
					subStr[subStrCount - 3] = '.'; 
					srcText.clear();
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
			JImGuiImpl::Text(result);
		}
		uint JEditorTextLineCalculator::CalTextLengthRange(const std::string& calText, const float length)const
		{
			const uint loopCount = (uint)calText.size();
			for (uint i = 0; i < loopCount; ++i)
			{
				if (ImGui::CalcTextSize(calText.substr(0, i + 1).c_str()).x >= length)
					return i;
			}
			return loopCount;
		}
		uint JEditorTextLineCalculator::CalTextAreaRange(const std::string& calText, const float area)const
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
	}
}