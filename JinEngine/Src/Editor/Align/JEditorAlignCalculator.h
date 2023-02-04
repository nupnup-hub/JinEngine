#pragma once 
#include"JEditorAlignType.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"


namespace JinEngine
{
	namespace Editor
	{
		//Use local cursor
		template<int columnCount>
		class JEditorStaticAlignCalculator
		{
		private:
			float contentsWidth[columnCount];
			float contentsStart[columnCount];
			int nowIndex = 0;
		public:
			JEditorStaticAlignCalculator(const float(&width)[columnCount])
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
		private:
			void Next()
			{
				++nowIndex;
				if (nowIndex >= columnCount)
					nowIndex = 0;
			}
		};

		//Use local cursor
		template<int innerContentsCount>
		class JEditorDynamicAlignCalculator
		{
		private:
			JVector2<float> padding = JVector2<float>(0, 0);
			JVector2<float> spacing = JVector2<float>(0, 0);
		private:
			JVector2<float> canvasSize; 
			JVector2<float> contentsSize;
			JVector2<float> innerPadding[innerContentsCount];
			JVector2<float> contentsStartCursor;
		private:
			float innerHeight[innerContentsCount];
			uint innerHeightOffset[innerContentsCount];
			uint columnMaxCount = 0;
			uint columnCount = 0;
			uint rowCount = 0;
		private:
			uint innerRowCount = 0;
		public:
			JVector2<float> GetTotalContentsSize()const noexcept
			{
				return contentsSize;
			}
			JVector2<float> GetNowContentsSize(const bool applyPadding = true)const noexcept
			{
				JVector2<float> validSize = contentsSize;
				validSize.y = innerHeight[innerRowCount];
				if(applyPadding)
					validSize = validSize - (innerPadding[innerRowCount] * 2);
				 
				return validSize;
			}
		public:
			void Update(const JVector2<float>& newCanvasSize,
				const JVector2<float>& newContentsSize,
				const JVector2<float>& contentsPadding,
				const JVector2<float>& contentsSpacing,
				const float(&iHeight)[innerContentsCount],
				const JVector2<float>(&iPadding)[innerContentsCount],
				const JVector2<float>& startPos)noexcept
			{
				padding = contentsPadding;
				spacing = contentsSpacing;
				canvasSize = newCanvasSize;
				contentsSize = newContentsSize;
				 
				for (uint i = 0; i < innerContentsCount; ++i)
				{
					innerHeight[i] = iHeight[i];
					innerPadding[i] = iPadding[i]; 
				}

				uint sum = 0;
				for (uint i = 0; i < innerContentsCount; ++i)
				{
					innerHeightOffset[i] = sum;
					sum += innerHeight[i];
				}

				float leftWidth = (canvasSize.x - (padding.x * 2));
				float contentsWidth = contentsSize.x;
				while (leftWidth >= contentsSize.x)
				{
					++columnMaxCount;
					leftWidth -= contentsWidth;
					if (contentsWidth == contentsSize.x)
						contentsWidth += spacing.x;
				}
				 
				columnCount = rowCount = innerRowCount = 0;
				contentsStartCursor = startPos + padding;
			}
			void Update(const JVector2<float>& newCanvasSize,
				const JVector2<float>& contentsPadding,
				const JVector2<float>& contentsSpacing,
				const uint columnPerCount,
				const uint rowPerCount,
				const float(&iHeightRate)[innerContentsCount],
				const JVector2<float>(&iPaddingRate)[innerContentsCount],
				const JVector2<float>& startPos)
			{
				padding = contentsPadding;
				spacing = contentsSpacing;
				canvasSize = newCanvasSize;

				const float contentWidth = ((canvasSize.x - padding.x * 2) - ((columnPerCount - 1) * contentsSpacing.x)) / columnPerCount;
				const float contentHeight = ((canvasSize.y - padding.y * 2) - ((rowPerCount - 1) * contentsSpacing.y)) / rowPerCount;
				contentsSize = JVector2<float>(contentWidth, contentHeight);
 
				for (uint i = 0; i < innerContentsCount; ++i)
				{
					innerHeight[i] = iHeightRate[i] * contentsSize.y;
					innerPadding[i] = iPaddingRate[i] * contentsSize;
				}

				uint sum = 0;
				for (uint i = 0; i < innerContentsCount; ++i)
				{
					innerHeightOffset[i] = sum;
					sum += innerHeight[i];
				}
				columnMaxCount = columnPerCount;
				columnCount = rowCount = innerRowCount = 0; 
 
				contentsStartCursor = startPos + padding;
			}

		public:
			float GetCursorPosX()noexcept
			{
				return contentsStartCursor.x + columnCount * spacing.x + columnCount * contentsSize.x + innerPadding[innerRowCount].x;
			}
			float GetCursorPosY()noexcept
			{
				return contentsStartCursor.y + rowCount * spacing.y + rowCount * contentsSize.y + innerPadding[innerRowCount].y + innerHeightOffset[innerRowCount];
			}
			JVector2<float> GetCursorPos()noexcept
			{
				return JVector2<float>(GetCursorPosX(), GetCursorPosY());
			}
			void SetNextContentsPosition()
			{
				ImGui::SetCursorPos(GetCursorPos());
				Next();
			}
		private:
			void Next()
			{
				++innerRowCount;
				if (innerRowCount >= innerContentsCount)
				{
					innerRowCount = 0;
					++columnCount;
					if (columnCount >= columnMaxCount)
					{
						columnCount = 0;
						++rowCount;
					}
				}				
			}
		};

		class JEditorTextAlignCalculator
		{
		private:
			std::string text;
			JVector2<float> size;
			bool useCompress = true;
			int linePerAlpabet = 0;
			float lineLength = 0;  
		public:
			void Update(const std::string& text, const JVector2<float>& size, const bool useCompress);
		public:
			std::string LeftAligned();
			std::string MiddleAligned();
			std::string RightAligned();
		public:
			std::string Aligned(const J_EDITOR_ALIGN_TYPE dir);
		private:
			uint CalTextLengthRange(const std::string& calText, const float length)const;
			uint CalTextAreaRange(const std::string& calText, const float area)const;
		};
		 
		class JEditorCursorPosCalculator
		{
		public: 
			//left align
			void SetMiddline(const JVector2<float>& pos,
				const JVector2<float>& size,
				const JVector2<float>& constentsSize,
				const JVector2<float>& padding,
				const bool useWorldCursor);
		};
	}
}