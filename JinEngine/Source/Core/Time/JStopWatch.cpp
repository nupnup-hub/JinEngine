/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JStopWatch.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			static constexpr float ns2sec = 0.000000001f;
			static constexpr float ns2milli = 0.000001f;
			static constexpr float ns2micro = 0.001f;

			template<typename Unit>
			float ToFloatTime(const std::chrono::steady_clock::time_point& tp)
			{ 
				return (float)std::chrono::duration_cast<Unit>(tp.time_since_epoch()).count();
			}
			template<typename Unit>
			float ToFloatTime(const std::chrono::steady_clock::time_point& st, const std::chrono::steady_clock::time_point& ed)
			{
				return (float)std::chrono::duration_cast<Unit>(ed - st).count();
			}
		}
		void JStopWatch::Start()noexcept
		{
			isStop = false;
			time = std::chrono::high_resolution_clock::now();
		} 
		void JStopWatch::Stop()noexcept
		{  
			prevTime += (float)std::chrono::duration<double, std::nano>(std::chrono::high_resolution_clock::now() - time).count();
			isStop = true;
		}
		void JStopWatch::Reset()noexcept
		{ 
			prevTime = 0;
			isStop = false;
			time = std::chrono::high_resolution_clock::now();
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
			if (isStop)
				return prevTime * ns2sec;
			else
				return ToFloatTime<std::chrono::seconds>(time, std::chrono::high_resolution_clock::now()) + prevTime * ns2sec;
		}
		float JStopWatch::GetElapsedMilliTime()const noexcept
		{
			if (isStop)
				return prevTime * ns2milli;
			else
				return ToFloatTime<std::chrono::milliseconds>(time, std::chrono::high_resolution_clock::now()) + prevTime * ns2milli;
		}
		float JStopWatch::GetElapsedMicroTime()const noexcept
		{
			if (isStop)
				return prevTime * ns2micro;
			else
				return ToFloatTime<std::chrono::microseconds>(time, std::chrono::high_resolution_clock::now()) + prevTime * ns2micro;
		}
		float JStopWatch::GetElapsedNanoTime()const noexcept
		{
			if (isStop)
				return prevTime;
			else
				return ToFloatTime<std::chrono::nanoseconds>(time, std::chrono::high_resolution_clock::now()) + prevTime;
		}
	}
}