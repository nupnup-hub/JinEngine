#include"JMouse.h"

namespace JinEngine::Core
{
	bool JMouse::IsMouseClicked(const J_MOUSE_BUTTON btn)const noexcept
	{
		return click[(uint)btn];
	}
	bool JMouse::IsMouseUp(const J_MOUSE_BUTTON btn)const noexcept
	{
		return up[(uint)btn];
	}
	bool JMouse::IsMouseDown(const J_MOUSE_BUTTON btn)const noexcept
	{
		return down[(uint)btn];
	}
	bool JMouse::IsMouseDragging(const J_MOUSE_BUTTON btn)const noexcept
	{
		return dragging[(uint)btn];
	}
	void JMouse::Down(const J_MOUSE_BUTTON btn) noexcept
	{
		if (lastDownIndex != invalidIndex && lastDownIndex == (uint)btn)
			dragging[lastDownIndex] = true;

		down[(uint)btn] = true;
		lastDownIndex = (uint)btn;
	}
	void JMouse::Up(const J_MOUSE_BUTTON btn) noexcept
	{
		up[(uint)btn] = true;
		click[(uint)btn] = true;
		if (lastDownIndex != invalidIndex && lastDownIndex == (uint)btn)
			lastDownIndex = invalidIndex;
	}
	void JMouse::Move(const JVector2<int>& newPos) noexcept
	{
		position = newPos;
	}
	void JMouse::Clear()
	{
		up.reset();
		click.reset();
		down.reset();
		dragging.reset();
	}
}