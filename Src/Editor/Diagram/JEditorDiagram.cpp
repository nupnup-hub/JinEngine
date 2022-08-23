#pragma once 
#include"JEditorDiagram.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorDiagram::JEditorDiagram(float scrollingX, float scrollingY, float zoom)
			:scrollingX(scrollingX), scrollingY(scrollingY), zoom(zoom)
		{}
		JEditorDiagram::~JEditorDiagram() {}
		float JEditorDiagram::GetScrollingX()const noexcept
		{
			return scrollingX;
		}
		float JEditorDiagram::GetScrollingY()const noexcept
		{
			return scrollingY;
		}
		float JEditorDiagram::GetZoomRate()const noexcept
		{
			return zoom;
		}

		void JEditorDiagram::SetScrolling(const float newScrollingX, const float newScrollingY)noexcept
		{
			scrollingX = newScrollingX;
			scrollingY = newScrollingY;
		}
		void JEditorDiagram::SetScrollingX(const float newScrollingX)noexcept
		{
			scrollingX = newScrollingX;
		}
		void JEditorDiagram::SetScrollingT(const float newScrollingY)noexcept
		{
			scrollingY = newScrollingY;
		}
		void JEditorDiagram::AddZoomRate(const float value)noexcept
		{
			zoom += value;
			if (zoom > maxZoomRate)
				zoom = maxZoomRate;
			if (zoom < minZoomRate)
				zoom = minZoomRate;
		}
	}
}