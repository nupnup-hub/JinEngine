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