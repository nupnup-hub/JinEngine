#pragma once
#include<time.h>
#include<memory>
#include"../JDataType.h"
#include"../Singleton/JSingletonHolder.h"

namespace JinEngine
{
	namespace Application
	{
		class JApplication;
	}

	namespace Core
	{
		template<typename T>class JCreateUsingNew;
		class JGameTimerImpl
		{
		private:
			friend class Application::JApplication; 
			template<typename T>friend class JCreateUsingNew;
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
		public: 
			float TotalTime()const noexcept;
			float DeltaTime()const noexcept;
		private:
			JGameTimerImpl();
			~JGameTimerImpl();
			void Start() noexcept;
			void Stop() noexcept;
			void Reset() noexcept;
			void Tick() noexcept;
		};
	}
	using JGameTimer = Core::JSingletonHolder<Core::JGameTimerImpl>;
}