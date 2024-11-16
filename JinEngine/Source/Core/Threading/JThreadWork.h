#pragma once 
#include"JThreadJobDesc.h"
#include"JThreadLocalStorageData.h" 
#include"../Func/Functor/JFunctor.h"
#include<process.h> 
#include<strsafe.h>
#include<vector> 
#include<queue> 

namespace JinEngine
{
	namespace Core
	{
		using ThreadWorkFunc = _beginthreadex_proc_type;
		using ThreadError = int;
		namespace Constants
		{
			static constexpr AtomicInt::value_type initializeStep = 0;
			static constexpr AtomicInt::value_type loopStep = 1;
			static constexpr AtomicInt::value_type endStep = 2;  
		}
		namespace ThreadErrorCode
		{
			static constexpr ThreadError none = 0;
			static constexpr ThreadError tlsOutOfIndex = 1;
		}
		 
		/**
		* Worker thread data
		* Main thread(manage thread)와 해당하는 worker thread가 공유하는 Class
		* thread 상태 정의와 동기화에 사용.
		*/
		class JThreadWorker
		{
		public:
			using Queue = std::queue<JobDesc>;
			using AtomicQueue = std::atomic<Queue*>;
		public:
			struct Information
			{
			public:
				//Thread index per group
				size_t threadIndex = 0;
				ThreadError errorCode = ThreadErrorCode::none;
			};
			struct SyncManage
			{
			public:
				JKernelEvent waitEvent;
				JCriticalSectionTool* toolSet = nullptr;
				size_t maxSpinLockCount = 0;
			};
		public:
			struct ControlData
			{
				//Has possibility to change variable in thread
			public:
				AtomicQueue queue;
			public:
				AtomicBoolen canWork = true;
				AtomicBoolen isWait = false;
			public:
				AtomicInt state = Constants::initializeStep;
				AtomicUint64 spinCount = 0;
			};
		public:
			Information info;
			SyncManage sync;
			ControlData control;
		private:
			Queue jobQueue;
		private:
			TlsIndex tlsIndex = 0;
		public:
			JThreadWorker();
		public:
			/*
			* @ return true => proceed ExecuteJob, fasle => try to entry wait
			*/
			bool Wait();
			void ExecuteJob();
		public:
			void Add(JobDesc&& desc);
			void Clear();
		public:
			uint TotalJobCount()const noexcept;
		public:
			TlsIndex GetTlsIndex()const noexcept;
		public:
			bool UseKernelWait()const noexcept;
		public:
			static unsigned __stdcall Work(void* param);
		};
	}
}