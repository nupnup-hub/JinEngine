#include"JOccBase.h"
#include"../../Core/Time/JGameTimer.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		void JOccBase::UpdateTimer()noexcept
		{
			const float totalTime = JGameTimer::Instance().TotalTime();
			if (totalTime > lastUpdateTime + updateFrequency)
			{
				canOcclusionCullingStart = true;
				lastUpdateTime = totalTime;
			}
			else
				canOcclusionCullingStart = false;
		}
		bool JOccBase::CanReadOcclusionResult()const noexcept
		{
			return canReadCullingResult;
		}
		bool JOccBase::CanCullingStart()const noexcept
		{
			return canOcclusionCullingStart;
		}
		void JOccBase::SetUpdateFrequency(const float newUpdateFrequency)noexcept
		{
			updateFrequency = newUpdateFrequency;
		}
		void JOccBase::SetReadResultTrigger(bool value)noexcept
		{
			canReadCullingResult = value;
		}
	}
}