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
			double mSecondsPercount;
			double mDeltaTime;
			double mFixedTime;

			int64 mBaseTime;
			int64 mPausedTime;
			int64 mStopTime;
			int64 mPrevTime;
			int64 mCurrTime;
			bool mStopped;
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