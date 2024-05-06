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
#include<time.h> 
#include"../JCoreEssential.h" 

namespace JinEngine
{
	class JMain;
	namespace Core
	{  
		class JGameTimer
		{
		private:
			friend class JMain;
		private: 
			double mSecondsPercount = 0;
			double mDeltaTime = 0;
			double mFixedTime = 0;

			int64 mBaseTime = 0;
			int64 mPausedTime = 0;
			int64 mStopTime = 0;
			int64 mPrevTime = 0;
			int64 mCurrTime = 0;
			bool mStopped = false;
		private:
			const size_t guid;
		public:
			JGameTimer();
			~JGameTimer();
		public: 
			float TotalTime()const noexcept;
			float DeltaTime()const noexcept;
		public:
			void Start() noexcept;
			void Stop() noexcept;
			void Reset() noexcept;
		public:
			bool IsStop()const noexcept;
		public:
			//Application update fps
			static float FramePerSecond()noexcept;
		private:  
			static void UpdateAllTimer();
		}; 
	}


	static struct JEngineTimer
	{
	public:
		static Core::JGameTimer& Data()
		{
			static Core::JGameTimer timer;
			return timer;
		}
	}JEngineTimer;

}