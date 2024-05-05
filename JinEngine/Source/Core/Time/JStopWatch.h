#pragma once 
#include"../JCoreEssential.h"
#include <chrono>
#include <ctime>  

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
			size_t prevTime = 0;	//nano
		private:
			std::chrono::steady_clock::time_point time;  
		private:
			std::vector<JStopWatchResult> resultVec;
		private:
			bool isStop = false;
		public:
			void Start()noexcept;  
			void Stop()noexcept; 
			void Reset()noexcept;
			void Record()noexcept;
			void Clear()noexcept;
		public:
			int GetRecordCount()const noexcept;
			JStopWatchResult GetRecord(int index)const noexcept;
			std::vector<JStopWatchResult> GetRecordVec()const noexcept;
		public:
			float GetElapsedSecondTime()const noexcept;
			float GetElapsedMilliTime()const noexcept;
			float GetElapsedMicroTime()const noexcept;
			float GetElapsedNanoTime()const noexcept;
		};
	}
}