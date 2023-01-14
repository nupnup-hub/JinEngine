#include"JActivatedInterface.h"

namespace JinEngine
{
	namespace Core
	{
		bool JActivatedInterface::IsActivated()const noexcept
		{
			return isActivated;
		}
		void JActivatedInterface::Activate() noexcept
		{
			if (!isActivated)
				DoActivate();
		}
		void JActivatedInterface::DeActivate()noexcept
		{
			if (isActivated)
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