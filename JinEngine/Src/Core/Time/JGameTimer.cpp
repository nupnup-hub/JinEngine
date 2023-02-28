//***************************************************************************************
// JGameTimer.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************
#include "JGameTimer.h"
#include"../Guid/GuidCreator.h"
#include <windows.h>
#include<string>
#include <chrono>  
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		static struct TimerCashVecData
		{
		private:
			using TimerCashVec = std::vector<JGameTimer*>;
		public:
			static TimerCashVec& Data()
			{
				static TimerCashVec vec;
				return vec;
			}
		}TimerCashVecData;

		JGameTimer::JGameTimer()
			: mSecondsPercount(0.0), mDeltaTime(0.0), mBaseTime(0),
			mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false), guid(MakeGuid())
		{
			TimerCashVecData::Data().push_back(this);
			__int64 countsPerSec;
			QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
			mSecondsPercount = 1.0 / (double)countsPerSec;
		}
		JGameTimer::~JGameTimer()
		{
			const uint count = (uint)TimerCashVecData::Data().size();
			for (uint i = 0; i < count; ++i)
			{
				if (TimerCashVecData::Data()[i]->guid == guid)
				{
					TimerCashVecData::Data().erase(TimerCashVecData::Data().begin() + i);
					break;
				}
			}
		}
		// Returns the total time elapsed since Reset() was called, NOT counting any
		// time when the clock is stopped.
		float JGameTimer::TotalTime()const noexcept
		{
			// If we are stopped, do not count the time that has passed since we stopped.
			// Moreover, if we previously already had a pause, the distance 
			// mStopTime - mBaseTime includes paused time, which we do not want to count.
			// To correct this, we can subtract the paused time from mStopTime:  
			//
			//                     |<--paused time-->|
			// ----*---------------*-----------------*------------*------------*------> time
			//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

			if (mStopped)
			{
				return (float)(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPercount);
			}

			// The distance mCurrTime - mBaseTime includes paused time,
			// which we do not want to count.  To correct this, we can subtract 
			// the paused time from mCurrTime:  
			//
			//  (mCurrTime - mPausedTime) - mBaseTime 
			//
			//                     |<--paused time-->|
			// ----*---------------*-----------------*------------*------> time
			//  mBaseTime       mStopTime        startTime     mCurrTime

			else
			{
				return (float)(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPercount);
			}
		}
		float JGameTimer::DeltaTime()const noexcept
		{
			return (float)mDeltaTime;
		}
		void JGameTimer::Start() noexcept
		{
			int64 startTime;
			QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


			// Accumulate the time elapsed between stop and start pairs.
			//
			//                     |<-------d------->|
			// ----*---------------*-----------------*------------> time
			//  mBaseTime       mStopTime        startTime     

			if (mStopped)
			{
				mPausedTime += (startTime - mStopTime);

				mPrevTime = startTime;
				mStopTime = 0;
				mStopped = false;
			}
		}

		void JGameTimer::Stop() noexcept
		{
			if (!mStopped)
			{
				int64 currTime;
				QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

				mStopTime = currTime;
				mStopped = true;
			}
		}
		void JGameTimer::Reset() noexcept
		{
			__int64 currTime;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

			mCurrTime = currTime;
			mBaseTime = currTime;
			mPrevTime = currTime;
			mStopTime = 0;
			mStopped = false;
		}
		bool JGameTimer::IsStop()const noexcept
		{
			return mStopped;
		}
		void JGameTimer::TickAllTimer()
		{
			auto tickLam = [](JGameTimer* timer)
			{
				if (timer->mStopped)
				{
					timer->mDeltaTime = 0.0;
					return;
				}
				int64 currTime;
				QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
				timer->mCurrTime = currTime;
				// Time difference between this frame and the previous.
				timer->mDeltaTime = (timer->mCurrTime - timer->mPrevTime) * timer->mSecondsPercount;
				// Prepare for next frame.
				timer->mPrevTime = timer->mCurrTime;

				// Force nonnegative.  The DXSDK's CDXUTGameTimer mentions that if the 
				// libcessor goes into a power save mode or we get shuffled to another
				// libcessor, then mDeltaTime can be negative.
				if (timer->mDeltaTime < 0.0f)
					timer->mDeltaTime = 0.0f;
			};

			const uint count = (uint)TimerCashVecData::Data().size();
			for (uint i = 0; i < count; ++i)
				tickLam(TimerCashVecData::Data()[i]);
		}
	}
}