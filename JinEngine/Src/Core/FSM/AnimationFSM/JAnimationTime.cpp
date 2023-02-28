#pragma once
#include"JAnimationTime.h"

namespace JinEngine
{
	namespace Core
	{
		void JAnimationTime::Clear()
		{
			timePos = 0;
			startTime = 0;
			endTime = 0;
			normalizedTime = 0;
			nextUpdateTime = 0;
		}
		bool JAnimationTime::IsOverEndTime()const noexcept
		{
			return normalizedTime >= 1.0f;
		}
	}
}