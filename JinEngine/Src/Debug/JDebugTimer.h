#pragma once 
#include <chrono>
#include <ctime> 
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		struct JDebugTimeResult
		{
		public:
			float secondTime = 0;
			float msTime = 0;
			float nanoTime = 0;
		};
		class JDebugTimer
		{
		private:
			static std::chrono::steady_clock::time_point startTime;
			static std::chrono::steady_clock::time_point endTime;
		private:
			static std::unordered_map<size_t, JDebugTimeResult> timeResult;
		public:
			static void StartGameTimer()noexcept;
			static void StopGameTimer()noexcept;
			static void RecordTime(const size_t guid)noexcept;
			static JDebugTimeResult GetTimeResult(const size_t guid)noexcept;
			static float GetElapsedSecondTime()noexcept;
			static float GetElapsedMilliTime()noexcept;
			static float GetElapsedNanoTime()noexcept;
		};
	}
}