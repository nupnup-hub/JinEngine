#include"JLazyDestructionInfo.h"

namespace JinEngine
{
	namespace Core
	{
		JLazyDestructionInfo::JLazyDestructionInfo(const float waitTime,
			ExecuteDestroy executeDestroy,
			CanDestroy canDestroy,
			NotifyExecuteLazy notifyExecuteLazy,
			NotifyCancelLazy notifyCancelLazy)
			:waitTime(waitTime),
			executeDestroy(executeDestroy), 
			canDestroy(canDestroy),
			notifyExecuteLazy(notifyExecuteLazy),
			notifyCancelLazy(notifyCancelLazy)
		{}
	}
}