#include"JDebugTimer.h"

namespace JinEngine
{
	namespace Core
	{
		std::chrono::steady_clock::time_point JDebugTimer::startTime;
		std::chrono::steady_clock::time_point JDebugTimer::endTime;
		std::unordered_map<size_t, JDebugTimeResult> JDebugTimer::timeResult;

		void JDebugTimer::StartGameTimer()noexcept
		{
			startTime = std::chrono::high_resolution_clock::now();
		}
		void JDebugTimer::StopGameTimer()noexcept
		{
			endTime = std::chrono::high_resolution_clock::now();
		}
		void JDebugTimer::RecordTime(const size_t guid)noexcept
		{
			auto data = timeResult.find(guid);
			if (data == timeResult.end())
			{
				timeResult.emplace(guid, JDebugTimeResult());
				data = timeResult.find(guid);
			}
			data->second.secondTime = GetElapsedSecondTime();
			data->second.msTime = GetElapsedMilliTime();
			data->second.nanoTime = GetElapsedNanoTime();
		}
		JDebugTimeResult JDebugTimer::GetTimeResult(const size_t guid)noexcept
		{
			return timeResult.find(guid)->second;
		}
		float JDebugTimer::GetElapsedSecondTime()noexcept
		{
			return (float)std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
		}
		float JDebugTimer::GetElapsedMilliTime()noexcept
		{
			return (float)std::chrono::duration<double, std::milli>(endTime - startTime).count();
		}
		float JDebugTimer::GetElapsedNanoTime()noexcept
		{
			return (float)std::chrono::duration<double, std::nano>(endTime - startTime).count();
		}
	}
}