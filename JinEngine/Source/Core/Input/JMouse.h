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


#pragma once
#include"../JCoreEssential.h"
#include"../Math/JVector.h"
#include<bitset>

namespace JinEngine
{
	namespace Core
	{
		enum class J_MOUSE_BUTTON
		{
			LEFT,
			RIGHT,
			MIDDLE,
			COUNT
		};

		class JMouse
		{
		public:
			static constexpr uint left = (uint)J_MOUSE_BUTTON::LEFT;
			static constexpr uint right = (uint)J_MOUSE_BUTTON::RIGHT;
			static constexpr uint middle= (uint)J_MOUSE_BUTTON::MIDDLE;
		private:
			static constexpr uint buttonCount = (uint)J_MOUSE_BUTTON::COUNT;
		private:
			std::bitset<buttonCount> click;
			std::bitset<buttonCount> up;
			std::bitset<buttonCount> down;
			std::bitset<buttonCount> dragging;
		private:
			float wheel;
		private:
			JVector2<int> position;
		private:
			int lastDownIndex = invalidIndex;
		public:
			bool IsMouseClicked(const J_MOUSE_BUTTON btn)const noexcept;
			bool IsMouseUp(const J_MOUSE_BUTTON btn)const noexcept;
			bool IsMouseDown(const J_MOUSE_BUTTON btn)const noexcept;
			bool IsMouseDragging(const J_MOUSE_BUTTON btn)const noexcept;
		public:
			void Up(const J_MOUSE_BUTTON btn) noexcept;
			void Down(const J_MOUSE_BUTTON btn) noexcept;
			void Move(const JVector2<int>& newPos) noexcept;
		public:
			void Clear();
		};
	}
}