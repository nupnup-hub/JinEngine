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


#include"JStorageInterface.h"
#include<limits.h>

namespace JinEngine::Core
{
	void JStorageInterface::SetClearTrigger()noexcept
	{
		clearRequest = true;
	}
	void JStorageInterface::OffClearTrigger()noexcept
	{
		clearRequest = false;
	}
	bool JStorageInterface::HasClearRequest()const noexcept
	{
		return clearRequest;
	}
	  
	uint JStorageUpdateInterface::GetUpdateCount()const noexcept
	{
		return updateCount;
	}
	void JStorageUpdateInterface::SetWaitFrame(const uint frameCount)noexcept
	{
		waitFrame = frameCount;
	}
	void JStorageUpdateInterface::AddUpdateCount()noexcept
	{
		++updateCount;
		if (updateCount == UINT_MAX)
			updateCount = 1;
	}
	void JStorageUpdateInterface::MinusWaitFrame()noexcept
	{
		if (waitFrame > 0)
			--waitFrame;
	}
	bool JStorageUpdateInterface::HasWaitFrame()const noexcept
	{
		return waitFrame > 0;
	}

	void JVolatileStorageInterface::SetAliveTrigger()noexcept
	{
		canAlive = true;
	}
	void JVolatileStorageInterface::OffAliveTrigger()noexcept
	{
		canAlive = false;
	}
	bool JVolatileStorageInterface::CanAlive()const noexcept
	{
		return canAlive;
	}
}