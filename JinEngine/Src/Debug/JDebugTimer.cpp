#include"JDebugTimer.h"

namespace JinEngine
{
	namespace Core
	{
		std::chrono::steady_clock::time_point JDebugTimer::startTime;
		std::chrono::steady_clock::time_point JDebugTimer::endTime;

		void JDebugTimer::StartGameTimer()noexcept
		{
			startTime = std::chrono::high_resolution_clock::now();
		}
		void JDebugTimer::StopGameTimer()noexcept
		{
			endTime = std::chrono::high_resolution_clock::now();
		}
		float JDebugTimer::GetElapsedNanoTime()noexcept
		{
			return (float)std::chrono::duration<double, std::nano>(endTime - startTime).count();
		}
		float JDebugTimer::GetElapsedMilliTime()noexcept
		{
			return (float)std::chrono::duration<double, std::milli>(endTime - startTime).count();
		}
		float JDebugTimer::GetElapsedSecondTime()noexcept
		{
			return (float)std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
		}
	}
}