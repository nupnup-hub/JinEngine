#pragma once 
#include <chrono>
#include <ctime> 

namespace JinEngine
{
	namespace Core
	{
		class JDebugTimer
		{
		private:
			static 	std::chrono::steady_clock::time_point startTime;
			static 	std::chrono::steady_clock::time_point endTime;

		public:
			static void StartGameTimer()noexcept;
			static void StopGameTimer()noexcept;
			static float GetElapsedNanoTime()noexcept;
			static float GetElapsedMilliTime()noexcept;
			static float GetElapsedSecondTime()noexcept;
		};
	}
}