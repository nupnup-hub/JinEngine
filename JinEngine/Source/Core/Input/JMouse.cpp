/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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