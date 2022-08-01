#pragma once 
#include"EditorDiagram.h"

namespace JinEngine
{
	EditorDiagram::EditorDiagram(float scrollingX, float scrollingY, float zoom)
		:scrollingX(scrollingX), scrollingY(scrollingY), zoom(zoom)
	{}
	EditorDiagram::~EditorDiagram() {}
	float EditorDiagram::GetScrollingX()const noexcept
	{
		return scrollingX;
	}
	float EditorDiagram::GetScrollingY()const noexcept
	{
		return scrollingY;
	}
	float EditorDiagram::GetZoomRate()const noexcept
	{
		return zoom;
	}

	void EditorDiagram::SetScrolling(const float newScrollingX, const float newScrollingY)noexcept
	{
		scrollingX = newScrollingX;
		scrollingY = newScrollingY;
	}
	void EditorDiagram::SetScrollingX(const float newScrollingX)noexcept
	{
		scrollingX = newScrollingX;
	}
	void EditorDiagram::SetScrollingT(const float newScrollingY)noexcept
	{
		scrollingY = newScrollingY;
	}
	void EditorDiagram::AddZoomRate(const float value)noexcept
	{
		zoom += value;
		if (zoom > maxZoomRate)
			zoom = maxZoomRate;
		if (zoom < minZoomRate)
			zoom = minZoomRate;
	}

}