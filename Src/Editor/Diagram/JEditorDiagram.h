#pragma once 
#include"../../Core/JDataType.h"
#include"../../Utility/JVector.h"

namespace JinEngine
{
	namespace Editor
	{
		struct JEditorDiagram
		{
		private:
			float scrollingX;
			float scrollingY;
			float zoom;

			static constexpr uint maxZoomRate = 2;
			static constexpr uint minZoomRate = 1;
		public:
			JEditorDiagram(float scrollingX = 0, float scrollingY = 0, float zoom = minZoomRate);
			~JEditorDiagram();

			JEditorDiagram(const JEditorDiagram& rhs) = delete;
			JEditorDiagram& operator=(const JEditorDiagram& rhs) = delete;

			float GetScrollingX()const noexcept;
			float GetScrollingY()const noexcept;
			float GetZoomRate()const noexcept;

			void SetScrolling(const float newScrollingX, const float newScrollingY)noexcept;
			void SetScrollingX(const float newScrollingX)noexcept;
			void SetScrollingT(const float newScrollingY)noexcept;
			void AddZoomRate(const float value)noexcept;
		};
	}
}