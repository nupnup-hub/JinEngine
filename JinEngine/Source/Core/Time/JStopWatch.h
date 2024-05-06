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