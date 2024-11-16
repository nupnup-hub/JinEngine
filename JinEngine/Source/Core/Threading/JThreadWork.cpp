#include"JThreadWork.h"
#include"JThreadLocalStorageData.h" 

namespace JinEngine::Core
{
	/*
	why use atomic
		- C++ 의 모든 객체들은 수정 순서(modification order) 라는 것을 정의할 수 있으며 시간에 따른 값의 변화이며 
		  서로 다른 스레드는 현재 다른 값을 관찰할 수 있으며(최종적으로 같은 값)
		  오직 동기화많이 해당 시간대에 모든 스레드가 같은 값을 보게 할 수 있음.
		- 컴파일러가 명령어를 재배치 할 때, 다른 쓰레드들을 고려하지 않는다는 점.
		- 최적화 수행시 값을 캐시해놓고 거기서 read를 수행할 시 다른 스레드에 변경사항을 감지 할 수 없음(volatile과 문제 공유)
		- atomic은 원자적 연산과 메모리 재배치 순서를 강제할 수 있는 기능으로 올바른 결과를 이끌어 낼 수 있음.
		- 예상되는 대기시간에 따라 사용자 모드와 커널 모드에 대기를 구분해서 사용해야 함. 
	*/

	JThreadWorker::JThreadWorker()
	{
		control.queue = &jobQueue;
	}
	bool JThreadWorker::Wait()
	{ 
		//std::cout << std::to_string(info.uniqueID) + ": Wait\n";
		control.spinCount = 0;

		bool isEmpty = control.queue.load(std::memory_order_acquire)->size() == 0;
		if (!isEmpty)
			return true;

		while (isEmpty || control.isWait.load(std::memory_order_acquire))
		{
			while (control.isWait.load(std::memory_order_acquire))
				;

			if (control.spinCount.fetch_add(1) >= sync.maxSpinLockCount)
			{ 
				//Hybrid wait
				//Spin lock wait -> kernel wait   
				sync.waitEvent.Wait();
				control.spinCount = 0;
				if (!control.canWork)
					break;
			}
			isEmpty = control.queue.load(std::memory_order_acquire)->size() == 0;
		}
		return false;
	}
	void JThreadWorker::ExecuteJob()
	{ 
		auto queue = control.queue.load(std::memory_order_acquire);
		const JobDesc& desc = queue->front();
		desc.func->InvokeCompletelyBind();

		if (desc.notifyAtomic != nullptr)
			desc.notifyAtomic->store(true, std::memory_order_acquire);
		if (desc.notifyEvent != nullptr)
			desc.notifyEvent->OnEvent();

		queue->pop();
	}
	void JThreadWorker::Add(JobDesc&& desc)
	{ 
		control.queue.load(std::memory_order_acquire)->push(std::move(desc));
	}
	void JThreadWorker::Clear()
	{
		auto queue = control.queue.load(std::memory_order_acquire);
		while (!queue->empty())
			queue->pop();
	}
	uint JThreadWorker::TotalJobCount()const noexcept
	{
		return jobQueue.size();
	}
	TlsIndex JThreadWorker::GetTlsIndex()const noexcept
	{
		return tlsIndex;
	}
	bool JThreadWorker::UseKernelWait()const noexcept
	{
		if (sync.maxSpinLockCount > 0)
			return control.spinCount.load(std::memory_order_acquire) >= (sync.maxSpinLockCount);
		else
			return true;
	}
	unsigned __stdcall JThreadWorker::Work(void* param)
	{
		JThreadWorker* worker = static_cast<JThreadWorker*>(param);
		JTlsData tlsData;
		tlsData.threadIndex = worker->info.threadIndex;
		tlsData.syncTool = worker->sync.toolSet;

		worker->tlsIndex = TlsAlloc();
		if (worker->tlsIndex == TLS_OUT_OF_INDEXES)
		{ 
			worker->info.errorCode = ThreadErrorCode::tlsOutOfIndex;
			return 0;
		}
		TlsSetValue(worker->tlsIndex, &tlsData);

		std::atomic_thread_fence(std::memory_order_release);

		worker->control.state.store(Constants::loopStep, std::memory_order::memory_order_seq_cst);
		while (worker->control.canWork)
		{
			if (!worker->Wait())
				continue;
			worker->ExecuteJob();
		}

		TlsFree(worker->tlsIndex);
		worker->control.state.store(Constants::endStep, std::memory_order::memory_order_release);
		return 0;
	}

}