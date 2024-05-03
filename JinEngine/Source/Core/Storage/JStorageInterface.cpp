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