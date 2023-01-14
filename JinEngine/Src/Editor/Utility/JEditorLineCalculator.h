#pragma once
#include"../../Utility/JCommonUtility.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		template<int columnCount>
		class JEditorLineCalculator
		{
		private:
			float contentsWidth[columnCount];
			float contentsStart[columnCount];
			int nowIndex = 0;
		public:
			JEditorLineCalculator(const float(&width)[columnCount])
			{
				for(uint i = 0; i < columnCount; ++i)
					contentsWidth[i] = width[i];
				contentsStart[0] = 0; 
			}
		public:
			void LabelOnScreen(const std::string& str)
			{ 
				JImGuiImpl::Text(str);
				if (nowIndex + 1 < columnCount)
				{
					ImGui::SameLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + contentsWidth[nowIndex]);
					contentsStart[nowIndex + 1] = ImGui::GetCursorPosX();
				}
				Next();
			}
		public:
			float GetItemRangeMax()
			{
				return contentsWidth[nowIndex];
			}
			void SetNextContentsPosition()
			{
				if(nowIndex > 0)
					ImGui::SameLine();
				ImGui::SetCursorPosX(contentsStart[nowIndex]);
				Next();
			}
		public:
			void Next()
			{
				++nowIndex;
				if (nowIndex >= columnCount)
					nowIndex = 0;
			}
		};
	}
}