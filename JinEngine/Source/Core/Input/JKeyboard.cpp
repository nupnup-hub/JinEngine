#include"JKeyboard.h" 

namespace JinEngine
{
	namespace Core
	{ 
		bool JKeyboard::IsKeyPress(const J_KEYCODE keycode)const noexcept
		{
			return pressKey[(int)keycode];
		}
		bool JKeyboard::IsKeyDown(const J_KEYCODE keycode)const noexcept
		{
			return downKey[(int)keycode];
		}
		bool JKeyboard::IsKeyUp(const J_KEYCODE keycode)const noexcept
		{
			return upKey[(int)keycode];
		}

		void JKeyboard::PressKey(const ushort key)noexcept
		{
			if (pressKey[key])
				downKey[key] = false;
			else
				downKey[key] = true;

			pressKey[key] = true;
		}
		void JKeyboard::UpKey(const ushort key)noexcept
		{
			upKey[key] = true;
			pressKey[key] = false;
			downKey[key] = false; 
		} 
	}
}
