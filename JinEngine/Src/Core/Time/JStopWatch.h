#pragma once 
#include <chrono>
#include <ctime> 
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		struct JStopWatchResult
		{
		public:
			float secondTime = 0;
			float msTime = 0;
			float nanoTime = 0;
		};
		class JStopWatch
		{
		private:
			std::chrono::steady_clock::time_point startTime;
			std::chrono::steady_clock::time_point endTime;
		private:
			std::vector<JStopWatchResult> resultVec;
		public:
			void Start()noexcept;
			void Stop()noexcept;
			void Record()noexcept;
			void Clear()noexcept;
		public:
			int GetRecordCount()const noexcept;
			JStopWatchResult GetRecord(int index)const noexcept;
			std::vector<JStopWatchResult> GetRecordVec()const noexcept;
		public:
			float GetElapsedSecondTime()const noexcept;
			float GetElapsedMilliTime()const noexcept;
			float GetElapsedNanoTime()const noexcept;
		};
	}
}