#include"JActivatedInterface.h"

namespace JinEngine
{
	namespace Core
	{
		bool JActivatedInterface::IsActivated()const noexcept
		{
			return isActivated;
		}
		bool JActivatedInterface::CanActivate() const noexcept
		{
			return true;
		}
		bool JActivatedInterface::CanDeActivate() const noexcept
		{
			return true;
		}
		void JActivatedInterface::Activate() noexcept
		{
			if (!IsActivated() && CanActivate())
				DoActivate();
		}
		void JActivatedInterface::DeActivate()noexcept
		{
			if (IsActivated() && CanDeActivate())
				DoDeActivate();
		}
		void JActivatedInterface::DoActivate() noexcept
		{
			isActivated = true;
		}
		void JActivatedInterface::DoDeActivate()noexcept
		{
			isActivated = false;
		}
	}
}