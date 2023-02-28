#pragma once
#include<time.h>
#include<memory>
#include"../JDataType.h" 

namespace JinEngine
{
	namespace Application
	{
		class JApplication;
	}

	namespace Core
	{ 
		class JGameTimer
		{
		private:
			friend class Application::JApplication;  
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
		private: 
			static void TickAllTimer();
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
	};

}