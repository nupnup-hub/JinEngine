#include"JInputManager.h" 

namespace JinEngine
{
	namespace Window
	{
		std::bitset<JInputManager::keyNumber> JInputManager::pressKey;
		std::bitset<JInputManager::keyNumber> JInputManager::downKey;
		std::bitset<JInputManager::keyNumber> JInputManager::upKey;

		bool JInputManager::GetKeyPress(J_KEYCODE keycode)noexcept
		{
			return pressKey[(int)keycode];
		}
		bool JInputManager::GetKeyDown(J_KEYCODE keycode)noexcept
		{
			return downKey[(int)keycode];
		}
		bool JInputManager::GetKeyUp(J_KEYCODE keycode)noexcept
		{
			return upKey[(int)keycode];
		}

		void JInputManager::PressKey(int key)noexcept
		{
			if (pressKey[key])
				downKey[key] = false;
			else
				downKey[key] = true;

			pressKey[key] = true;
		}
		void JInputManager::UpKey(int key)noexcept
		{
			upKey[key] = true;
			pressKey[key] = false;
			downKey[key] = false;
			upkeyIndex = key;
		}
		void JInputManager::ReleaseUpKey()noexcept
		{
			upKey[upkeyIndex] = false;
			upkeyIndex = -1;
		}
		bool JInputManager::HasUpkey() noexcept
		{
			return upkeyIndex >= 0;
		}
	}
}
