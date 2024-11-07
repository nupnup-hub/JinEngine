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


#include"JThreadManager.h"  
#include"../Guid/JGuidCreator.h"
#include"../Func/Functor/JFunctor.h"   
 
namespace JinEngine::Core
{
	using ThreadHandle = uintptr_t;
	using ThreadID = uint;
	using TlsIndexMap = std::unordered_map<ThreadID, TlsIndex>;

	enum class J_THREAD_USE_CASE
	{
		HYBRID_WAIT,
		JOB_QUEUE,
		COUNT
	};

	static constexpr uint threadTypeCount = (uint)J_THREAD_USE_CASE::COUNT;
	 
	class ThreadPrivateData
	{
	public:
		uint index;
		ThreadHandle handle;
		ThreadID id;
	public:
		JThreadWorker worker;
	};
	class ThreadDataCache
	{
	public:
		TlsIndex tlsIndex;
	public:
		ThreadDataCache(const ThreadPrivateData& privateData)
			:tlsIndex(privateData.worker.GetTlsIndex())
		{}
	};
	using DataCacheMap = std::unordered_map<ThreadID, ThreadDataCache>;

	class ThreadGroupBase
	{
	public:
		const size_t guid;
	public:
		ThreadGroupBase(const size_t guid)
			:guid(guid)
		{}
	public:
		virtual ~ThreadGroupBase() = default;
	public:
		virtual void Initialize() = 0;
	public:
		virtual void PushJob(JobDesc&& desc) = 0;
		virtual void PushJobPerThread(std::vector<JobDesc>& desc) = 0;
		virtual void PushJobPerThread(std::vector<std::vector<JobDesc>>& desc, const uint perThreadJobCount) = 0;
		virtual void PushJobPerThread(JobDesc* desc) = 0;
		virtual void PushJobPerThread(JobDesc* desc, const uint perThreadJobCount) = 0;
	public:
		virtual void Terminate() = 0;
	public:
		virtual bool TryGetTlsIndex(const ThreadID id, _Inout_ TlsIndex& outIndex)const = 0;
	public:
		virtual bool IsValid()const noexcept = 0;
	};
	template<uint threadCount>
	class ThreadGroup : public ThreadGroupBase
	{
	private:
		ThreadPrivateData threadData[threadCount];
		JCriticalSectionTool syncTool;
	private:
		DataCacheMap* cacheMap = nullptr;
	public:
		ThreadGroup(const size_t guid, const JThreadGroupOption option, DataCacheMap* cacheMap)
			:ThreadGroupBase(guid), cacheMap(cacheMap)
		{
			for (uint i = 0; i < threadCount; ++i)
			{
				threadData[i].index = i;

				threadData[i].worker.info.threadIndex = i;

				threadData[i].worker.sync.toolSet = &syncTool;
				threadData[i].worker.sync.maxSpinLockCount = option.spinLockCount;

				threadData[i].worker.control.canWork = true;
				threadData[i].worker.control.isWait = false;
				threadData[i].worker.control.state = Constants::initializeStep;

				threadData[i].handle = _beginthreadex(NULL, 0, &JThreadWorker::Work, &threadData[i].worker, 0, &threadData[i].id);

				if (threadData[i].handle == NULL)
					threadData[i].worker.control.canWork = false;
			}
		}
		~ThreadGroup()
		{
			if (cacheMap == nullptr)
				return;

			for (uint i = 0; i < threadCount; ++i)
				cacheMap->erase(threadData[i].id);
		}
	public:
		void Initialize()final
		{
			while (!EntryThreadFunc(std::make_index_sequence<threadCount>()))
				;

			for (uint i = 0; i < threadCount; ++i)
				cacheMap->emplace(threadData[i].id, ThreadDataCache(threadData[i]));
		}
	public:
		void PushJob(JobDesc&& desc)final
		{
			uint index = 0;
			uint minJobCount = INT_MAX;
			for (uint i = 0; i < threadCount; ++i)
			{
				const uint jobCount = threadData[i].worker.TotalJobCount();
				if (jobCount == 0)
				{
					index = i;
					break;
				}

				if (jobCount < minJobCount)
				{
					index = i;
					minJobCount = jobCount;
				}
			}

			LockWait(index);
			threadData[index].worker.Add(std::move(desc));
			UnLockWait(index);
		}
		void PushJobPerThread(std::vector<JobDesc>& desc)final
		{
			//fit thread count
			if (desc.size() != threadCount)
				return;

			for (uint i = 0; i < threadCount; ++i)
			{
				LockWait(i);
				threadData[i].worker.Add(std::move(desc[i]));
				UnLockWait(i);
			}
		}
		void PushJobPerThread(std::vector<std::vector<JobDesc>>& desc, const uint perThreadJobCount)final
		{
			//fit thread count 
			for (uint i = 0; i < threadCount; ++i)
			{
				LockWait(i);
				for (uint j = 0; j < perThreadJobCount; ++j)
					threadData[i].worker.Add(std::move(desc[i][j]));
				UnLockWait(i);
			}
		}
		void PushJobPerThread(JobDesc* desc)
		{
			for (uint i = 0; i < threadCount; ++i)
			{
				LockWait(i);
				threadData[i].worker.Add(std::move(desc[i]));
				UnLockWait(i);
			}
		}
		void PushJobPerThread(JobDesc* desc, const uint perThreadJobCount)
		{
			//fit thread count 
			for (uint i = 0; i < threadCount; ++i)
			{
				LockWait(i);
				for (uint j = 0; j < perThreadJobCount; ++j)
					threadData[i].worker.Add(std::move(desc[i * perThreadJobCount + j]));
				UnLockWait(i);
			}
		}
	public:
		void Terminate()final
		{ 
			for (uint i = 0; i < threadCount; ++i)
			{
				auto& worker = threadData[i].worker;

				worker.Clear();
				worker.control.canWork.store(false, std::memory_order_release);
				worker.sync.maxSpinLockCount = 0;
			}

			while (!EntryKernelWait(std::make_index_sequence<threadCount>()))
				;
			 
			for (uint i = 0; i < threadCount; ++i)
			{
				auto& worker = threadData[i].worker;
				worker.sync.waitEvent.OnEvent();
			}

			std::vector<HANDLE> threadHandleVec(threadCount);
			for (uint i = 0; i < threadCount; ++i)
				threadHandleVec[i] = HANDLE(threadData[i].handle);
			 
			WaitForMultipleObjects(threadCount, threadHandleVec.data(), true, INFINITE);
			for (uint i = 0; i < threadCount; ++i)
				CloseHandle(threadHandleVec[i]); 
		}
	private:
		void LockWait(const uint index)
		{
			auto& worker = threadData[index].worker;
			worker.control.isWait.store(true, std::memory_order_release);
		}
		void UnLockWait(const uint index)
		{
			auto& worker = threadData[index].worker;
			if (worker.UseKernelWait())
				worker.sync.waitEvent.OnEvent();
			else
				worker.control.spinCount.store(0, std::memory_order_release);
			worker.control.isWait.store(false, std::memory_order_release);
		}
	private:
		template<size_t ...Is>
		bool EntryThreadFunc(std::index_sequence<Is...>)
		{
			return ((threadData[Is].worker.control.state == Constants::loopStep) && ...);
		}
		template<size_t ...Is>
		bool EntryKernelWait(std::index_sequence<Is...>)
		{
			return ((threadData[Is].worker.UseKernelWait()) && ...);
		}
	public:
		bool TryGetTlsIndex(const ThreadID id, _Inout_ TlsIndex& outIndex)const final
		{
			for (uint i = 0; i < threadCount; ++i)
			{
				if (threadData[i].id == id)
				{
					outIndex = threadData[i].worker.GetTlsIndex();
					return true;
				}
			}
			return false;
		}
	public:
		bool IsValid()const noexcept final
		{
			for (uint i = 0; i < threadCount; ++i)
			{
				if (threadData[i].handle == 0)
					return false;
			}
			return true;
		}
	};

	class JThreadManager::JThreadManagerImpl
	{
	private:
		std::unordered_map<size_t, std::unique_ptr<ThreadGroupBase>> groupMap;
		std::vector<ThreadGroupBase*> groupVec;
	private:
		DataCacheMap cacheMap;
	public:
		void Clear()
		{
			groupMap.clear();
			groupVec.clear();
		}
	public:
		size_t CreateThreadGroup(const uint newThreadCount, const JThreadGroupOption option, bool& isSuccess)
		{
			const size_t guid = Core::MakeGuid();
			isSuccess = false;

			if (newThreadCount == 1)
				groupVec.push_back(groupMap.emplace(guid, std::make_unique<ThreadGroup<1>>(guid, option, &cacheMap)).first->second.get());
			else if (newThreadCount == 2)
				groupVec.push_back(groupMap.emplace(guid, std::make_unique<ThreadGroup<2>>(guid, option, &cacheMap)).first->second.get());
			else if (newThreadCount == 4)
				groupVec.push_back(groupMap.emplace(guid, std::make_unique<ThreadGroup<4>>(guid, option, &cacheMap)).first->second.get());
			else if (newThreadCount == 8)
				groupVec.push_back(groupMap.emplace(guid, std::make_unique<ThreadGroup<8>>(guid, option, &cacheMap)).first->second.get());
			else if (newThreadCount == 16)
				groupVec.push_back(groupMap.emplace(guid, std::make_unique<ThreadGroup<16>>(guid, option, &cacheMap)).first->second.get());
			else if (newThreadCount == 32)
				groupVec.push_back(groupMap.emplace(guid, std::make_unique<ThreadGroup<32>>(guid, option, &cacheMap)).first->second.get());
			else
				return 0;

			if (!groupVec[groupVec.size() - 1]->IsValid())
			{
				DestroyThreadGroup(guid);
				isSuccess = false;
				return 0;
			}
			else
			{
				groupVec[groupVec.size() - 1]->Initialize();
				isSuccess = true;
				return guid;
			}
		}
		bool DestroyThreadGroup(const size_t guid)
		{
			auto group = GetThreadGroup(guid);
			if (group == nullptr)
				return false;

			uint index = 0;
			for (uint i = 0; i < groupVec.size(); ++i)
			{
				if (groupVec[i]->guid == guid)
				{
					index = i;
					break;
				}
			}
			groupVec[index]->Terminate();

			groupVec.erase(groupVec.begin() + index);
			groupMap.erase(guid);
			return true;
		}
	public:
		void PushJob(const size_t guid, JobDesc&& desc)
		{
			//Push one job 
			auto group = GetThreadGroup(guid);
			if (group == nullptr)
				return;

			group->PushJob(std::move(desc));
		}
		void PushJobPerThread(const size_t guid, std::vector<JobDesc>& desc)
		{
			//Push one job per thread
			auto group = GetThreadGroup(guid);
			if (group == nullptr)
				return;

			group->PushJobPerThread(desc);
		}
		void PushJobPerThread(const size_t guid, std::vector<std::vector<JobDesc>>& desc, const uint perThreadJobCount)
		{
			//Push n job per thread
			auto group = GetThreadGroup(guid);
			if (group == nullptr)
				return;

			group->PushJobPerThread(desc, perThreadJobCount);
		}
		void PushJobPerThread(const size_t guid, JobDesc* desc)
		{
			auto group = GetThreadGroup(guid);
			if (group == nullptr)
				return;

			group->PushJobPerThread(desc);
		}
		void PushJobPerThread(const size_t guid, JobDesc* desc, const uint perThreadJobCount)
		{
			auto group = GetThreadGroup(guid);
			if (group == nullptr)
				return;

			group->PushJobPerThread(desc, perThreadJobCount);
		}
	public:
		JTlsData* GetTlsData()
		{
			auto data = cacheMap.find(GetCurrentThreadId());
			return data != cacheMap.end() ? static_cast<JTlsData*>(TlsGetValue(data->second.tlsIndex)) : nullptr;
		}
	private:
		ThreadGroupBase* GetThreadGroup(const size_t guid)
		{
			auto data = groupMap.find(guid);
			return data == groupMap.end() ? nullptr : data->second.get();
		}
	};

	JThreadManager::JThreadManager()
		:impl(std::make_unique<JThreadManagerImpl>())
	{}
	JThreadManager::~JThreadManager()
	{
		impl = nullptr;
	}
	size_t JThreadManager::CreateThreadGroup(const uint newThreadCount, const JThreadGroupOption option)
	{
		bool temp = false;
		return impl->CreateThreadGroup(newThreadCount, option, temp);
	}
	size_t JThreadManager::CreateThreadGroup(const uint newThreadCount, const JThreadGroupOption option, bool& isSuccess)
	{
		return impl->CreateThreadGroup(newThreadCount, option, isSuccess);
	}
	bool JThreadManager::DestroyThreadGroup(const size_t guid)
	{
		return impl->DestroyThreadGroup(guid);
	}
	void JThreadManager::PushJob(const size_t guid, JobDesc&& desc)
	{
		impl->PushJob(guid, std::move(desc));
	}
	void JThreadManager::PushJobPerThread(const size_t guid, std::vector<JobDesc>& desc)
	{
		impl->PushJobPerThread(guid, desc);
	}
	void JThreadManager::PushJobPerThread(const size_t guid, std::vector<std::vector<JobDesc>>& desc, const uint perThreadJobCount)
	{
		impl->PushJobPerThread(guid, desc, perThreadJobCount);
	}
	void JThreadManager::PushJobPerThread(const size_t guid, JobDesc* desc)
	{
		impl->PushJobPerThread(guid, desc);
	}
	void JThreadManager::PushJobPerThread(const size_t guid, JobDesc* desc, const uint perThreadJobCount)
	{
		impl->PushJobPerThread(guid, desc, perThreadJobCount);
	}
	JTlsData* JThreadManager::GetTlsData()
	{
		return impl->GetTlsData();
	}
}
