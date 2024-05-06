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

namespace JinEngine
{
	namespace Core
	{
		enum class J_KEYCODE
		{
			MOUSE_LEFT = 0x01,
			MOUSE_RIGHT,
			CONTROL_BREAK,
			MOUSE_MIDDLE,
			MOUSE_X1,
			MOUSE_X2,
			BACKSPACE = 0x08,
			TAB,
			CLEAR = 0x0C,
			ENTER,
			SHIFT = 0x10,
			CONTROL,
			ALT,
			PAUSE,
			CAPITAL,
			ESC = 0x1b,
			SPACE = 0x20,
			PAGE_UP,
			PAGE_DOWN,
			END,
			HOME,
			LEFT_ARROW,
			UP_ARROW,
			RIGHT_ARROW,
			DOWN_ARROW,
			SELECT,
			PRINT,
			EXECUTE,
			SCAPSHOT,
			INSERT,
			DELETE_,
			HELP,
			ALPHA_0,
			ALPHA1,
			ALPHA2,
			ALPHA3,
			ALPHA4,
			ALPHA5,
			ALPHA6,
			ALPHA7,
			ALPHA8,
			ALPHA9,
			A = 0x41,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			N,
			M,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			LEFT_WINDOW,
			RIGHT_WINDOW,
			APP,
			PAD_0 = 0x60,
			PAD_1,
			PAD_2,
			PAD_3,
			PAD_4,
			PAD_5,
			PAD_6,
			PAD_7,
			PAD_8,
			PAD_9,
			PAD_MUL = 0X6A,
			PAD_ADD,
			PAD_SEPARATOR,
			PAD_SUBTRACT,
			PAD_DECIMAL,
			PAD_DEVIDE,
			F1 = 0x70,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			NUMBER_LOCK = 0x90,
			SCROLL,
			LEFT_SHIFT = 0xa0,
			RIGHT_SHIFT,
			LEFT_CONTROL,
			RIGHT_CONTROL,
			LEFT_MENU,
			RIGHT_MENU
		};

		class JKeyboard
		{ 
		private:
			static constexpr size_t keyNumber = 254;
			std::bitset<keyNumber> pressKey;
			std::bitset<keyNumber> downKey;
			std::bitset<keyNumber> upKey; 
		public:
			bool IsKeyPress(const J_KEYCODE keycode)const noexcept;
			bool IsKeyDown(const J_KEYCODE keycode)const noexcept;
			bool IsKeyUp(const J_KEYCODE keycode)const noexcept;
		private:
			void PressKey(const ushort key) noexcept;
			void UpKey(const ushort key) noexcept;
		};
	}
}