#pragma once 
#include"../../../Core/JDataType.h"
#include"../../../Utility/Vector.h"

namespace JinEngine
{
	struct EditorDiagram
	{
	private:
		float scrollingX;
		float scrollingY;
		float zoom;
		  
		static constexpr uint maxZoomRate = 2;
		static constexpr uint minZoomRate = 1;
	public:
		EditorDiagram(float scrollingX = 0, float scrollingY = 0, float zoom = minZoomRate);
		~EditorDiagram();

		EditorDiagram(const EditorDiagram& rhs) = delete;
		EditorDiagram& operator=(const EditorDiagram& rhs) = delete;

		float GetScrollingX()const noexcept;
		float GetScrollingY()const noexcept;
		float GetZoomRate()const noexcept;

		void SetScrolling(const float newScrollingX, const float newScrollingY)noexcept;
		void SetScrollingX(const float newScrollingX)noexcept;
		void SetScrollingT(const float newScrollingY)noexcept;
		void AddZoomRate(const float value)noexcept;
	};
}