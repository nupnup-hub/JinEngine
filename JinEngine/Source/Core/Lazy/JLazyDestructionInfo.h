#pragma once

namespace JinEngine
{
	namespace Core
	{
		class JTypeBase;
		struct JLazyDestructionInfo
		{
		public:
			using ExecuteDestroy = bool(*)(JTypeBase*);
			using CanDestroy = bool(*)(JTypeBase*);
			using NotifyExecuteLazy = void(*)(JTypeBase*);
			using NotifyCancelLazy = void(*)(JTypeBase*);
		public:
			float waitTime = 1;
		public:
			ExecuteDestroy executeDestroy = nullptr;
			CanDestroy canDestroy;
			NotifyExecuteLazy notifyExecuteLazy = nullptr;
			NotifyCancelLazy notifyCancelLazy = nullptr;
		public:
			JLazyDestructionInfo(const float waitTime,
				ExecuteDestroy executeDestroy, 
				CanDestroy canDestroy,
				NotifyExecuteLazy notifyExecuteLazy = nullptr,
				NotifyCancelLazy notifyCancelLazy = nullptr);
		};
	}
}