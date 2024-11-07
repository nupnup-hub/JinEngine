#pragma once
#include"../JCoreEssential.h"
#include"JThreadType.h"

#define WINDOW
#ifdef WINDOW
#include<Windows.h>
#else
#endif

#include<vector>
namespace JinEngine
{
	namespace Core
	{
		enum class CRITICAL_SECTION_MANAGE_TOOL
		{
			CRITICAL_SECTION,
			JSRW,
			MUTEX,
			SEMAPHORE,
			COUNT
		};
		class JCriticalSectionSync
		{
		public:
			virtual ~JCriticalSectionSync() = default;
		public:
			virtual CRITICAL_SECTION_MANAGE_TOOL GetType()const noexcept = 0;
		public:
			virtual void EnterReader() = 0;
			virtual void EnterWriter() = 0;
			virtual void LeaveReader() = 0;
			virtual void LeaveWriter() = 0;
		};
		class JCriticalSectionTool
		{
		private:
			using SyncVec = std::vector<std::unique_ptr<JCriticalSectionSync>>;
		private:
			SyncVec sync[(uint)CRITICAL_SECTION_MANAGE_TOOL::COUNT];
		public:
			void Set(std::unique_ptr<JCriticalSectionSync>&& newSync);
		public:
			void EnterReader(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index);
			void EnterWriter(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index);
			void LeaveReader(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index);
			void LeaveWriter(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index);
		};
		template<uint atomicCount>
		class JAtomicBooleanSet
		{
		private:
			Core::AtomicBoolen trigger[atomicCount];
		public:
			Core::AtomicBoolen* GetPtr(const uint index)
			{
				return &trigger[index];
			}
		public:
			void Set(const uint index, const std::memory_order order = std::memory_order_release)
			{
				trigger[index].store(true, order);
			}
			void Off(const uint index, const std::memory_order order = std::memory_order_release)
			{
				trigger[index].store(false, order);
			}
			void OffWithCount(const uint count, const std::memory_order order = std::memory_order_release)
			{
				for (uint i = 0; i < count; ++i)
					trigger[i].store(false, order);
			}
			void Wait(const uint st, const uint count)
			{
				bool isDone = false;
				while (!isDone)
				{
					isDone = true;
					for (uint i = st; i < count; ++i)
					{
						isDone &= trigger[i];
						if (!isDone)
							break;
					}
				}
			}
		public:
			void AllOff(const std::memory_order order = std::memory_order_release)
			{
				Off(order, std::make_index_sequence<atomicCount>());
			}
			void AllWait()
			{
				while (!Wait(std::make_index_sequence<atomicCount>()))
					;
			}
		private:
			template<size_t ...Is>
			bool Wait(std::index_sequence< Is...>)
			{
				return (trigger[Is] && ...);
			}
			template<size_t ...Is>
			void Off(const std::memory_order order, std::index_sequence<Is...>)
			{
				(trigger[Is].store(false, order), ...);
			}
		};

#ifdef WINDOW   
		//Manage criticalSection access for prevent data race
		class CriticalSection final : public JCriticalSectionSync
		{
		private:
			CRITICAL_SECTION criticalSection;
		public:
			//if spinLockCount > 0 use spin lock
			CriticalSection(const uint spinLockCount = 0);
			~CriticalSection();
		public:
			CRITICAL_SECTION_MANAGE_TOOL GetType()const noexcept final;
		public:
			void EnterReader() final;
			void EnterWriter() final;
			void LeaveReader() final;
			void LeaveWriter() final;
		private:
			void Enter();
			void Leave();
		};
		class JSRW final : public JCriticalSectionSync
		{
		private:
			SRWLOCK srwLock;
		public:
			JSRW();
			~JSRW();
		public:
			CRITICAL_SECTION_MANAGE_TOOL GetType()const noexcept final;
		public:
			void EnterReader() final;
			void EnterWriter() final;
			void LeaveReader() final;
			void LeaveWriter() final;
		};
		class JMutex final : public JCriticalSectionSync
		{
		private:
			HANDLE mutex = NULL;
		public:
			JMutex();
			~JMutex();
		public:
			CRITICAL_SECTION_MANAGE_TOOL GetType()const noexcept final;
		public:
			void EnterReader() final;
			void EnterWriter() final;
			void LeaveReader() final;
			void LeaveWriter() final;
		private:
			void Enter();
			void Leave();
		};
		class JSemaphore final : public JCriticalSectionSync
		{
		private:
			HANDLE semaphore = NULL;
		public:
			JSemaphore(const uint initCount, const uint maxCount);
			~JSemaphore();
		public:
			CRITICAL_SECTION_MANAGE_TOOL GetType()const noexcept final;
		public:
			void EnterReader() final;
			void EnterWriter() final;
			void LeaveReader() final;
			void LeaveWriter() final;
		private:
			void Enter();
			void Leave();
		};

		//Sync
		class JKernelMultiEvent;
		class JKernelEvent
		{
		private:
			friend class JKernelMultiEvent;
		private:
			HANDLE handle;
		public:
			JKernelEvent(const bool useAuto = true);
			~JKernelEvent();
		public:
			void Wait();
		public:
			void OnEvent();
			void OffEvent();
		};
		class JKernelMultiEvent
		{
		private:
			std::vector<JKernelEvent> eventVec;
			std::vector<HANDLE> handleVec;
		public:
			JKernelMultiEvent(const uint count, const bool useAuto);
		public:
			uint GetCount()const noexcept;
			JKernelEvent* GetEvent(const uint index);
		public:
			void Wait();
		};
#else
#endif 
	}
}