#include"JStopWatch.h"

namespace JinEngine
{
	namespace Core
	{ 
		void JStopWatch::Start()noexcept
		{
			startTime = std::chrono::high_resolution_clock::now();
		}
		void JStopWatch::Stop()noexcept
		{
			endTime = std::chrono::high_resolution_clock::now();
		}
		void JStopWatch::Record()noexcept
		{	 
			JStopWatchResult newResult;
			newResult.secondTime = GetElapsedSecondTime();
			newResult.msTime = GetElapsedMilliTime();
			newResult.nanoTime = GetElapsedNanoTime();
			resultVec.push_back(newResult);
		}
		void JStopWatch::Clear()noexcept
		{
			resultVec.clear();
		}
		int JStopWatch::GetRecordCount()const noexcept
		{
			return (int)resultVec.size();
		}
		JStopWatchResult JStopWatch::GetRecord(int index)const noexcept
		{
			return resultVec[index];
		} 
		std::vector<JStopWatchResult> JStopWatch::GetRecordVec()const noexcept
		{
			return resultVec;
		}
		float JStopWatch::GetElapsedSecondTime()const noexcept
		{
			return (float)std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
		}
		float JStopWatch::GetElapsedMilliTime()const noexcept
		{
			return (float)std::chrono::duration<double, std::milli>(endTime - startTime).count();
		}
		float JStopWatch::GetElapsedMicroTime()const noexcept
		{
			return (float)std::chrono::duration<double, std::micro>(endTime - startTime).count();
		}
		float JStopWatch::GetElapsedNanoTime()const noexcept
		{
			return (float)std::chrono::duration<double, std::nano>(endTime - startTime).count();
		}
	}
}