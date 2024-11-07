#include"JSync.h"

namespace JinEngine::Core
{
	void JCriticalSectionTool::Set(std::unique_ptr<JCriticalSectionSync>&& newSync)
	{
		CRITICAL_SECTION_MANAGE_TOOL tool = newSync->GetType();
		sync[(uint)tool].push_back(std::move(newSync));
	}
	void JCriticalSectionTool::EnterReader(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index)
	{
		if (sync[(uint)toolType].size() <= index)
			return;

		sync[(uint)toolType][index]->EnterReader();
	}
	void JCriticalSectionTool::EnterWriter(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index)
	{
		if (sync[(uint)toolType].size() <= index)
			return;

		sync[(uint)toolType][index]->EnterWriter();
	}
	void JCriticalSectionTool::LeaveReader(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index)
	{
		if (sync[(uint)toolType].size() <= index)
			return;

		sync[(uint)toolType][index]->LeaveReader();
	}
	void JCriticalSectionTool::LeaveWriter(const CRITICAL_SECTION_MANAGE_TOOL toolType, const uint index)
	{
		if (sync[(uint)toolType].size() <= index)
			return;

		sync[(uint)toolType][index]->LeaveWriter();
	}

#ifdef WINDOW 
	CriticalSection::CriticalSection(const uint spinLockCount)
	{
		/*
			- 초기화에 따라서 스핀락을 사용 후 커널 오브젝트를 이용하여 대기 상태로 만듬.(유저모드 -> 커널모드)
			- EnterCriticalSection, LeaveCriticalSection에 내부 동작은 원자적으로 수행됨.
			- EnterCriticalSection에 의해 대기상태가 된 스레드는 지정된 대기시간을 만료시 예외를 발생.(절대로 기아 상태가 되지 않음)
		*/
		if (spinLockCount > 0)
			InitializeCriticalSectionAndSpinCount(&criticalSection, spinLockCount);
		else
			InitializeCriticalSection(&criticalSection);
	}
	CriticalSection::~CriticalSection()
	{
		DeleteCriticalSection(&criticalSection);
	}
	CRITICAL_SECTION_MANAGE_TOOL CriticalSection::GetType()const noexcept
	{
		return CRITICAL_SECTION_MANAGE_TOOL::CRITICAL_SECTION;
	}
	void CriticalSection::EnterReader()
	{
		Enter();
	}
	void CriticalSection::EnterWriter()
	{
		Enter();
	}
	void CriticalSection::LeaveReader()
	{
		Leave();
	}
	void CriticalSection::LeaveWriter()
	{
		Leave();
	}
	void CriticalSection::Enter()
	{
		EnterCriticalSection(&criticalSection);
	}
	void CriticalSection::Leave()
	{
		LeaveCriticalSection(&criticalSection);
	}

	JSRW::JSRW()
	{
		/*
			- CRITICAL_SECTION과 유사하게 동작하나 스레드를 리더와 라이터로 구분.
			- 라이터는 값을 메모리에서 불러오고 수정한 뒤 다시 저장하고 다른 스레드들도 새롭게 메모리에서 값을
			- 불러오는 과정을 수행하며 리더는 이러한 과정이 없으므로 더 빠름.
			- 크리티컬 섹션과에 차이
				- TryEnter 함수 없음 ... 2024-10-29 Window 11에서 존재확인.
				- 단일 스레드에서 여러 번 락 불가능(크리티컬 섹션 소유 스레드가 Enter를 몇 번이든 호출가능하지만 그 횟수만큼 Leave를 수행해야함.)
				- Reader들은 동시에 접근 가능.
		*/
		InitializeSRWLock(&srwLock);
	}
	JSRW::~JSRW()
	{

	}
	CRITICAL_SECTION_MANAGE_TOOL JSRW::GetType()const noexcept
	{
		return CRITICAL_SECTION_MANAGE_TOOL::JSRW;
	}
	void JSRW::EnterReader()
	{
		AcquireSRWLockShared(&srwLock);
	}
	void JSRW::EnterWriter()
	{
		AcquireSRWLockExclusive(&srwLock);
	}
	void JSRW::LeaveReader()
	{
		ReleaseSRWLockShared(&srwLock);
	}
	void JSRW::LeaveWriter()
	{
		ReleaseSRWLockExclusive(&srwLock);
	}

	/*
	당연하지만
	Wait... 호출을 통한 대기함수에서 세마포어 리소스카운트나 JMutex 스레드 ID 변경, 반복카운트 업데이트 같은
	작업은 모두 원자적으로 수행됨
	*/

	JMutex::JMutex()
	{
		/*
			- 사용 카운트, 스레드 ID, 반복 카운트를 저장할 수 있는 공간을 소유.
				- 스레드 ID: 소유하고 있는 스레드 ID ... 0 이면 어떤 스레드에도 소유되지 않음.
				- 반복 카운터: 소유하고 있는 스레드가 몇 회나 반복적으로 소유하고자 했는지의 대한 횟수.
			- 크리티컬 섹션과 동작방식이 동일하나 뮤텍스는 커널 오브젝트.
				- 더 느리지만 다른 프로세스의 스레드에서 접근 가능.
				- 시간제한 지정 가능.

			- 반복카운터가 0이 되면 스레드 값도 0이 되고 시그널 상태가 됨.
			- 논시그널 상태에서 소유 스레드가 재차 대기함수를 호출하면 반복카운터를 증가시키고 바로 빠져나옴.
				- 대기 함수 호출한 횟수만큼 ReleaseMutex를 호출해 반복카운터를 0 으로 만들어야 다른 스레드가 대기상태에서 빠져나올 수 있음.

			- 다른 커널 오브젝트와 다르게 스레드 소유권의 개념이 있음.
				- 스레드ID가 바로 그것이며 성공적으로 대기에 성공한 스레드의 ID를 기록.
				- 이러한 개념덕분에 논시그널 상태에서 스레드가 다시 소유할 수 있음.
				- ReleaseMutex시 ID 검사후 같지 않으면 감소시키지 않음.
		*/
		mutex = CreateMutexEx(NULL, NULL, 0, MUTEX_ALL_ACCESS);
	}
	JMutex::~JMutex()
	{
		CloseHandle(mutex);
	}
	CRITICAL_SECTION_MANAGE_TOOL JMutex::GetType()const noexcept
	{
		return CRITICAL_SECTION_MANAGE_TOOL::MUTEX;
	}
	void JMutex::EnterReader()
	{
		Enter();
	}
	void JMutex::EnterWriter()
	{
		Enter();
	}
	void JMutex::LeaveReader()
	{
		Leave();
	}
	void JMutex::LeaveWriter()
	{
		Leave();
	}
	void JMutex::Enter()
	{
		/*
			- JMutex는 소유되면 singal
			- 소유되지 않으면 non-signal
			- 소유한 thread가 여러번 성공적 대기를 수행할 수 있으며 그만큼 반복카운터를 Release 호출로 상쇄해야
			  대기중인 다른 스레드가 스케쥴 가능상태가 될 수있음.
		*/
		WaitForSingleObject(mutex, INFINITE);
	}
	void JMutex::Leave()
	{
		ReleaseMutex(mutex);
	}

	JSemaphore::JSemaphore(const uint initCount, const uint maxCount)
	{
		semaphore = CreateSemaphoreEx(NULL, initCount, maxCount, NULL, 0, SEMAPHORE_ALL_ACCESS);
	}
	JSemaphore::~JSemaphore()
	{
		CloseHandle(semaphore);
	}
	CRITICAL_SECTION_MANAGE_TOOL JSemaphore::GetType()const noexcept
	{
		return CRITICAL_SECTION_MANAGE_TOOL::SEMAPHORE;
	}
	void JSemaphore::EnterReader()
	{
		Enter();
	}
	void JSemaphore::EnterWriter()
	{
		Enter();
	}
	void JSemaphore::LeaveReader()
	{
		Leave();
	}
	void JSemaphore::LeaveWriter()
	{
		Leave();
	}
	void JSemaphore::Enter()
	{
		/*
			- 리소스의 개수를 고려하는 상황에서 사용.
			- 0 <= n(현재 리소스 카운트) < max(최대 리소스 카운트)
			- n = 0 이면 논 시그널(대기로 전환).
			- 0 < n 시그널(바로 사용 가능).
			- 사용시 리소스 카운트 감소.
			- 현재 리소스 카운트 값을 Get하는 건 불가능(2019)
		*/
		WaitForSingleObject(semaphore, INFINITE);
	}
	void JSemaphore::Leave()
	{
		ReleaseSemaphore(semaphore, 1, NULL);
	}

	JKernelEvent::JKernelEvent(const bool useAuto)
	{
		/*
			- 수동 리셋 이벤트
				- 시그널 시 이벤트를 기다리는 모든 스레드들이 동시에 스케쥴 가능 상태.
				- ex) 여러 스레드에서 동시에 read
			- 자동 리셋 이벤트
				- 시그널 시 이벤트를 기다리는 모든 스레드들 중 하나만 스케쥴 가능 상태.(내부 알고리즘에 의해 선택됨)
				- 성공적 대기의 부가적인 영향 가능. (시그널 -> 논 시그널 변경(return 전) -> 대기 중인 스레드에게 알림)
				- ex) 여려 스레드에서 순차적으로 write하고 signal
		*/
		handle = CreateEventEx(NULL, NULL, useAuto ? 0 : CREATE_EVENT_MANUAL_RESET, INFINITE);
	}
	JKernelEvent::~JKernelEvent()
	{
		CloseHandle(handle);
	}
	void JKernelEvent::Wait()
	{
		WaitForSingleObject(handle, INFINITE);
	}
	void JKernelEvent::OnEvent()
	{
		SetEvent(handle);
	}
	void JKernelEvent::OffEvent()
	{
		ResetEvent(handle);
	}

	JKernelMultiEvent::JKernelMultiEvent(const uint count, const bool useAuto)
	{
		handleVec.resize(count);
		for (uint i = 0; i < count; ++i)
		{
			eventVec.push_back(JKernelEvent(useAuto));
			handleVec[i] = eventVec[i].handle;
		}
	}
	uint JKernelMultiEvent::GetCount()const noexcept
	{
		return uint(eventVec.size());
	}
	JKernelEvent* JKernelMultiEvent::GetEvent(const uint index)
	{
		return index < eventVec.size() ? &eventVec[index] : nullptr;
	}
	void JKernelMultiEvent::Wait()
	{
		WaitForMultipleObjects(eventVec.size(), handleVec.data(), true, INFINITE);
	}
	/*
			struct SyncTool
		{
		private:
			std::unique_ptr<CriticalSection> criticalSection;
			std::unique_ptr<JSRW> srw;
		private:
			std::unique_ptr<JMutex> mutex;
			std::unique_ptr<JSemaphore> semaphore;
		private:
			const SYNC_TYPE type;
			bool isActivated = false;
		public:
			SyncTool(const SYNC_TYPE type)
				:criticalSection(CriticalSection(1000)), semaphore(JSemaphore(4, 4))
			{}
		public:

		public:
			void Enter()
			{
				if (!isActivated)
					return;

				if (index == 0)
					criticalSection.Enter();
				else if (index == 1)
					srw.EnterReader();
				else if (index == 2)
					srw.EnterWriter();
				else if (index == 3)
					mutex.Enter();
				else if (index == 4)
					semaphore.Enter();
			}
			void Leave()
			{
				if (!isActivated)
					return;

				if (index == 0)
					criticalSection.Leave();
				else if (index == 1)
					srw.LeaveReader();
				else if (index == 2)
					srw.LeaveWriter();
				else if (index == 3)
					mutex.Leave();
				else if (index == 4)
					semaphore.Leave();
			}
		};
	*/
#else
#endif


}