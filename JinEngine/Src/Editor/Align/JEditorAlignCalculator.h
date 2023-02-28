#pragma once 
#include"JEditorAlignType.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"  

namespace JinEngine
{
	namespace Editor
	{
		//Use local cursor 
		class JEditorStaticAlignCalculator
		{
		private:
			JVector2<float> canvasSize;
			JVector2<float> contentsSize;
			JVector2<float> contentsStartCursor;
		private:
			JVector2<float> padding;
			JVector2<float> spacing;
		private:
			uint columnCount = 0;
			uint columnIndex = 0;
			uint rowIndex = 0;
		public:
			void Update(const JVector2<float>& newCanvasSize,
				const JVector2<float>& newContentsSize,
				const JVector2<float>& padding,
				const JVector2<float>& spacing,
				const JVector2<float>& startPos)noexcept;
		public:
			float GetCursorPosX()const noexcept;
			float GetCursorPosY()const noexcept;
			JVector2<float> GetCursorPos()const noexcept;
			void SetNextContentsPosition()noexcept;
		private:
			void Next()noexcept;
		};

		//Use local cursor
		template<int innerCount>
		class JEditorDynamicAlignCalculator
		{
		private:
			JVector2<float> canvasSize;
			JVector2<float> contentsSize;
			JVector2<float> contentsStartCursor;
		private:
			JVector2<float> contentsPadding;
			JVector2<float> contentsSpacing;
		private:
			JVector2<float> innerPosOffset[innerCount];
			JVector2<float> innerSize[innerCount];
			J_EDITOR_INNER_ALGIN_TYPE innerAlignType;
		private:
			uint columnCount = 0;
			uint columnIndex = 0;
			uint rowIndex = 0;
			uint innerIndex = 0;
		public: 
			void Update(const JVector2<float>& canvasSize,
				const JVector2<float>& contentsSize,
				const JVector2<float>& contentsPadding,
				const JVector2<float>& contentsSpacing,
				const JVector2<float>(&innerSize)[innerCount],
				const J_EDITOR_INNER_ALGIN_TYPE innerAlignType,
				const JVector2<float>& startPos)noexcept
			{
				JEditorDynamicAlignCalculator::canvasSize = canvasSize;
				JEditorDynamicAlignCalculator::contentsSize = contentsSize;
				JEditorDynamicAlignCalculator::contentsPadding = contentsPadding;
				JEditorDynamicAlignCalculator::contentsSpacing = contentsSpacing;

				for (uint i = 0; i < innerCount; ++i)
					JEditorDynamicAlignCalculator::innerSize[i] = innerSize[i];			 
				JEditorDynamicAlignCalculator::innerAlignType = innerAlignType;

				JVector2<float> sum = 0;
				if (innerAlignType == J_EDITOR_INNER_ALGIN_TYPE::ROW)
				{
					for (uint i = 0; i < innerCount; ++i)
					{
						innerPosOffset[i] = sum ;
						sum += JVector2<float>(innerSize[i].x, 0);
					}
				}
				else
				{
					for (uint i = 0; i < innerCount; ++i)
					{
						innerPosOffset[i] = sum;
						sum += JVector2<float>(0, innerSize[i].y);
					}

				}

				float leftWidth = (canvasSize.x - (contentsPadding.x * 2));
				float contentsWidth = contentsSize.x;
				while (leftWidth >= contentsSize.x)
				{
					++columnCount;
					leftWidth -= contentsWidth;
					if (contentsWidth == contentsSize.x)
						contentsWidth += contentsSpacing.x;
				}

				columnIndex = rowIndex = innerIndex = 0;
				contentsStartCursor = startPos + contentsPadding;
			}
			void Update(const JVector2<float>& canvasSize,
				const JVector2<float>& contentsSize,
				const JVector2<float>& contentsPadding,
				const JVector2<float>& contentsSpacing,
				const JVector2<float>(&innerSize)[innerCount],
				const JVector2<float>(&innerPosition)[innerCount],
				const J_EDITOR_INNER_ALGIN_TYPE innerAlignType,
				const JVector2<float>& startPos)noexcept
			{
				JEditorDynamicAlignCalculator::canvasSize = canvasSize;
				JEditorDynamicAlignCalculator::contentsSize = contentsSize;
				JEditorDynamicAlignCalculator::contentsPadding = contentsPadding;
				JEditorDynamicAlignCalculator::contentsSpacing = contentsSpacing;

				for (uint i = 0; i < innerCount; ++i)
					JEditorDynamicAlignCalculator::innerSize[i] = innerSize[i];			 
				JEditorDynamicAlignCalculator::innerAlignType = innerAlignType;

				JVector2<float> sum = 0;
				if (innerAlignType == J_EDITOR_INNER_ALGIN_TYPE::ROW)
				{
					for (uint i = 0; i < innerCount; ++i)
					{
						innerPosOffset[i] = sum + innerPosition[i];
						sum += JVector2<float>(innerSize[i].x, 0);
					}
				}
				else
				{
					for (uint i = 0; i < innerCount; ++i)
					{
						innerPosOffset[i] = sum + innerPosition[i];
						sum += JVector2<float>(0, innerSize[i].y);
					}

				}

				float leftWidth = (canvasSize.x - (contentsPadding.x * 2));
				float contentsWidth = contentsSize.x;
				while (leftWidth >= contentsSize.x)
				{
					++columnCount;
					leftWidth -= contentsWidth;
					if (contentsWidth == contentsSize.x)
						contentsWidth += contentsSpacing.x;
				}

				columnIndex = rowIndex = innerIndex = 0;
				contentsStartCursor = startPos + contentsPadding;
			}
			void Update(const JVector2<float>& canvasSize,
				const JVector2<float>& contentsPadding,
				const JVector2<float>& contentsSpacing,
				const uint columnPerCount,
				const uint rowPerCount,
				const JVector2<float>(&innerSizeRate)[innerCount],
				const JVector2<float>(&innerPositionRate)[innerCount],
				const J_EDITOR_INNER_ALGIN_TYPE innerAlignType,
				const JVector2<float>& startPos)
			{
				JEditorDynamicAlignCalculator::canvasSize = canvasSize;
				JEditorDynamicAlignCalculator::contentsPadding = contentsPadding;
				JEditorDynamicAlignCalculator::contentsSpacing = contentsSpacing;

				const float contentWidth = ((canvasSize.x - contentsPadding.x * 2) - ((columnPerCount - 1) * contentsSpacing.x)) / columnPerCount;
				const float contentHeight = ((canvasSize.y - contentsPadding.y * 2) - ((rowPerCount - 1) * contentsSpacing.y)) / rowPerCount;
				contentsSize = JVector2<float>(contentWidth, contentHeight);

				for (uint i = 0; i < innerCount; ++i)
					innerSize[i] = innerSizeRate[i] * contentsSize.y; 
				JEditorDynamicAlignCalculator::innerAlignType = innerAlignType;

				JVector2<float> sum = 0;
				if (innerAlignType == J_EDITOR_INNER_ALGIN_TYPE::ROW)
				{
					for (uint i = 0; i < innerCount; ++i)
					{
						innerPosOffset[i] = sum + innerPositionRate[i] * contentsSize;
						sum += JVector2<float>(innerSize[i].x, 0);
					}
				}
				else
				{
					for (uint i = 0; i < innerCount; ++i)
					{
						innerPosOffset[i] = sum + innerPositionRate[i] * contentsSize;
						sum += JVector2<float>(0, innerSize[i].y);
					}
				}
				columnCount = columnPerCount;
				columnIndex = rowIndex = innerIndex = 0;
				contentsStartCursor = startPos + contentsPadding;
			}
		public:
			JVector2<float> GetTotalContentsSize()const noexcept
			{
				return contentsSize;
			}
			//Has order dependency
			JVector2<float> GetInnerContentsSize()const noexcept
			{
				return innerSize[innerIndex];
			}
			float GetCursorPosX()noexcept
			{
				return contentsStartCursor.x + columnIndex * contentsSpacing.x + columnIndex * contentsSize.x + innerPosOffset[innerIndex].x;
			}
			float GetCursorPosY()noexcept
			{
				return contentsStartCursor.y + rowIndex * contentsSpacing.y + rowIndex * contentsSize.y + innerPosOffset[innerIndex].y;
			}
			//Has order dependency
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
				++innerIndex;
				if (innerIndex >= innerCount)
				{
					innerIndex = 0;
					++columnIndex;
					if (columnIndex >= columnCount)
					{
						columnIndex = 0;
						++rowIndex;
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