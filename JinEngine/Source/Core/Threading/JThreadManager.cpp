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
#include"JThreadManagerPrivate.h"
#include"../Func//Functor/JFunctor.h"  
//#include"../Log/JLogMacro.h"
#include<mutex>
#include<condition_variable>

#if defined(_WIN32) || defined(_WIN64)
#include<windows.h>
#define USE_WINDOW
#endif
 
namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			using PriorityNumber = uint;
			static constexpr PriorityNumber firstPriority = 0; 
			static constexpr PriorityNumber lastPriority = 1;
			static constexpr PriorityNumber priorityRange = lastPriority + 1;

			//static constexpr int reservedThreadCount = Graphic::Constants::gMaxFrameThread;
			//static bool canLoopThread = true;
		}
		namespace
		{
			static int GetOrderPriority(const J_THREAD_USE_CASE_TYPE type)noexcept
			{
				if (type == J_THREAD_USE_CASE_TYPE::ENGINE_TASK_SYNC)
					return firstPriority; 
				else if (type == J_THREAD_USE_CASE_TYPE::COMMON)
					return lastPriority;
				else
					return invalidIndex;
			}
			static bool HasReservedSpace(const PriorityNumber priority)
			{
				if (priority == firstPriority)
					return true; 
				else
					return 0;
			}
		}
		namespace
		{
#ifdef USE_WINDOW
			class JThreadHandle
			{
			private: 
				std::unique_ptr<std::thread> thread;
				HANDLE handle;
				bool isWait = false;
				bool canLoop = true;
				//static std::vector<Data> data; 
			public:
				void Initialize()
				{ 
					handle = CreateEvent(NULL, FALSE, FALSE, NULL);
				}
				void Clear()
				{
					CloseHandle(handle);
					thread = nullptr;
				}
			public:
				void SetLoopTrigger(const bool value)
				{
					canLoop = value;
				} 
			public:
				bool IsJoinable()const noexcept
				{
					return thread != nullptr && thread->joinable();
				}
				bool IsWait()const noexcept
				{
					return isWait;
				}
				bool HasThread()const noexcept
				{
					return thread != nullptr;
				}
				bool CanLoop()const noexcept
				{
					return canLoop;
				}
			public:
				template<typename ...Param>
				void CreateThread(void(*ptr)(Param...), Param&&... param)
				{
					thread = std::make_unique<std::thread>(ptr, std::forward<Param>(param)...);
				}
			public:
				void Join()
				{
					if (thread == nullptr)
						return;

					thread->join();
				}
				void Wait()
				{
					if (thread == nullptr)
						return;

					ResetEvent(handle);
					isWait = true;
					WaitForSingleObject(handle, INFINITE);
				} 
				void WakeUpThread()
				{
					if (thread == nullptr)
						return;

					isWait = false;
					SetEvent(handle);
				}
			}; 
#else
			class JThreadHandle
			{
			private:
				std::unique_ptr<std::thread> thread;  
				std::unique_ptr<std::mutex> m;
				std::unique_ptr<std::condition_variable> cv;
				bool isWait = false;
				bool canLoop = true;
				bool allowSetNewThread = false;   
			public:
				void Initialize()
				{ 
					m = std::make_unique<std::mutex>();
					cv = std::make_unique<std::condition_variable>();
				}
				void Clear()
				{  
					thread = nullptr;
					m = nullptr;
					cv = nullptr;
				}
			public:
				void SetLoopTrigger(const bool value)
				{
					canLoop = value;
				}
			public:
				bool IsJoinable()const noexcept
				{
					return thread != nullptr && thread->joinable();
				}
				bool IsWait()const noexcept
				{
					return isWait;
				}
				bool CanWork()const noexcept
				{
					return !isWait;
				}
				bool CanLoop()const noexcept
				{
					return canLoop;
				}
			public:
				template<typename ...Param>
				void CreateThread(void(*ptr)(Param...), Param&&... param)
				{
					thread = std::make_unique<std::thread>(ptr, std::forward<Param>(param)...);
				}
			public:
				void Join()
				{
					if (thread == nullptr)
						return;

					thread->join();
				}
				void Wait()
				{
					JFunctor<bool> canWorkF(&JThreadHandle::CanWork, this);
					//생성시 lock
					std::unique_lock<std::mutex> lk(*m);
					isWait = true;
					//진입시 unlock wait 끝난후 lock
					cv->wait(lk, Bind(canWorkF));
				}
				void WakeUpThread()
				{
					isWait = false;
					std::lock_guard<std::mutex> lk(*m);
					cv->notify_one();
				}
			}; 
#endif
		}

		class JThreadManager::JThreadManagerImpl
		{
		public:
			std::vector<std::unique_ptr<JThread>> waitT[priorityRange];
			std::vector<std::unique_ptr<JThread>> workT;
			std::vector<std::unique_ptr<JThreadHandle>> handle;
			//std::vector<std::unique_ptr<std::thread>> thread;
		public:
			uint reservedSpace[priorityRange];
			uint workingPoolStartIndex[priorityRange];
		public:
			uint threadCount = 0;
			uint reservedThreadCount = 0;
		public:
			void Initialize(uint newReservedThreadCount)
			{
				const uint capa = std::thread::hardware_concurrency();
				newReservedThreadCount = newReservedThreadCount < capa ? newReservedThreadCount : capa;

				reservedThreadCount = newReservedThreadCount;
				//threadCount = newReservedThreadCount;

				int newReservedSpace[priorityRange];
				int newReservedSum = 0;
			 
				//stuff reserve space count
				for (int i = 0; i < priorityRange; ++i)
				{
					newReservedSpace[i] = 0;
					if (HasReservedSpace(i))
					{
						if (newReservedSum + reservedThreadCount < capa)
						{
							newReservedSpace[i] = reservedThreadCount;
							newReservedSum += reservedThreadCount;
						}
						else
						{
							if (capa - newReservedSum <= 2)
								newReservedSpace[i] = 0;
							else
							{
								newReservedSpace[i] = reservedThreadCount / 2;
								newReservedSum += reservedThreadCount / 2;
							}
						}
					}
				} 
				for (int i = 0; i < priorityRange; ++i)
					ExtendCapacity(i, newReservedSpace[i]);
			}
			void Clear()
			{ 
				for (uint i = 0; i < priorityRange; ++i)
					ReduceCapacity(i, GetWorkingPoolCount(i));
  
				for (uint i = 0; i < handle.size(); ++i)
					handle[i]->Clear();

				handle.clear(); 
				workT.clear();
				for (auto& data : waitT)
					data.clear(); 
			}
		public:
			int GetReservedSpaceCount(const PriorityNumber priority)const noexcept
			{
				return reservedSpace[priority];
			}
			int GetWorkingPoolStartIndex(const PriorityNumber priority)const noexcept
			{
				return workingPoolStartIndex[priority];
			}   
			int GetWorkingPoolIndex(const PriorityNumber priority, const uint localIndex)const noexcept
			{
				return GetWorkingPoolStartIndex(priority) + localIndex;
			}
			int GetWorkingPoolCount(const PriorityNumber priority)const noexcept
			{
				return priority == lastPriority ? (uint)handle.size() - workingPoolStartIndex[priority] :
					workingPoolStartIndex[priority + 1] - workingPoolStartIndex[priority];
			}
		public:
			//Push priority area
			void ExtendCapacity(const PriorityNumber priority, const uint count)
			{
				if (count == 0)
					return;
				 
				const uint existCount = GetWorkingPoolCount(priority);
				const uint stIndex = GetWorkingPoolStartIndex(priority) + existCount;
 
				std::vector< std::unique_ptr<JThreadHandle>> newHandle(count);
				handle.insert(handle.begin() + stIndex, std::move_iterator(newHandle.begin()), std::move_iterator(newHandle.end()));
				for (uint i = 0; i < count; ++i)
				{
					handle[i + stIndex] = std::make_unique<JThreadHandle>();
					handle[i + stIndex]->Initialize();
				}
  
				std::vector<std::unique_ptr<JThread>> newWorkT(count);		 
				workT.insert(workT.begin() + stIndex, std::move_iterator(newWorkT.begin()), std::move_iterator(newWorkT.end()));
			   
				threadCount += count;
				for (uint i = 0; i < count; ++i)
				{
					const uint index = stIndex + i;
					workT[index] = std::make_unique<JThread>();	
					*workT[index]->index = index;
					BeginThread(index);
				}

				for (uint i = stIndex + count; i < workT.size(); ++i)
					*workT[i]->index = i;

				if (HasReservedSpace(priority))
					reservedSpace[priority] += count;

				for (PriorityNumber i = priority + 1; i < priorityRange; ++i)
					workingPoolStartIndex[i] += count;
				
				//J_LOG_PRINT_OUT("ExtendCapacity", "push: " + std::to_string(count) + 
				//	" St: " + std::to_string(stIndex) + 
				//	" preCount: " + std::to_string(existCount) + 
				//	" newCount: " + std::to_string(threadCount));
			}
			//Pop priority area
			void ReduceCapacity(const PriorityNumber priority, uint count)
			{
				const uint existCount = GetWorkingPoolCount(priority);
				count = count > existCount ? existCount : count;
				if (count == 0)
					return;
 
				const int stIndex = GetWorkingPoolIndex(priority, existCount - count);
				for (uint i = 0; i < count; ++i)
					EndThread(stIndex + i);

				for (uint i = stIndex + count; i < workT.size(); ++i)
					*workT[i]->index -= count;

				threadCount -= count;		  
				workT.erase(workT.begin() + stIndex, workT.begin() + stIndex + count);
				//workT.resize(threadCount);

				for (uint i = stIndex; i < count; ++i)
					handle[i]->Clear();
				handle.erase(handle.begin() + stIndex, handle.begin() + stIndex + count);
				handle.shrink_to_fit();

				if (HasReservedSpace(priority))
					reservedSpace[priority] -= count;

				for (PriorityNumber i = priority + 1; i < priorityRange; ++i)
					workingPoolStartIndex[i] -= count;
				 
				//J_LOG_PRINT_OUT("ReduceCapacity", "pop: " + std::to_string(count) +
				//	" St: " + std::to_string(stIndex) +
				//	" preCount: " + std::to_string(existCount) +
				//	" newCount: " + std::to_string(threadCount));
			}
		public:
			void WaitUntilThreadEnd(const JThreadUserHandle& userHandle)
			{
				//OutputDebugStringA(("WaitUntilThreadEnd Index St: " + std::to_string(userHandle.GetIndex()) + " " + std::to_string(userHandle.GetGuid()) + " \n").c_str());
				const int index = userHandle.GetIndex(); 
				if (index == invalidIndex)
					return;
				
				while (!handle[index]->IsWait())
					; 
				//OutputDebugStringA(("WaitUntilThreadEnd Index Ed: " + std::to_string(index) + " " + std::to_string(userHandle.GetGuid()) + " \n").c_str());
			}
		public:
			JThreadUserHandle CreateThread(std::unique_ptr<JThread>&& newThread)
			{ 
				const PriorityNumber priorityNumber = GetOrderPriority(newThread->info.useCase);
				const uint woringSetStIndex = GetWorkingPoolStartIndex(priorityNumber);  
				const JThreadUserHandle userHandle(newThread->info.guid, newThread->index);
				  
				const int index = FindWaitThread(woringSetStIndex);
				if (index != invalidIndex)
					WakeUpThread(std::move(newThread), index);
				else
					WaitThread(std::move(newThread), priorityNumber);
				return userHandle;
			}
		public:
			void Update()
			{
				for (int i = 0; i < priorityRange; ++i)
				{
					const uint woringSetStIndex = GetWorkingPoolStartIndex(i);
					for (int j = 0; j < waitT[i].size(); ++j)
					{
						const int index = FindWaitThread(woringSetStIndex);
						if (index == invalidIndex)
							return;

						MoveToWorkQueue(index, i, j);  
						--j;
					}
				}
			}
		private:
			void BeginThread(uint index)
			{
				auto threadFLam = [](JThreadManagerImpl* impl, const uint index)
				{
					auto handle = impl->handle[index].get();
					while (handle->CanLoop())
					{ 
						handle->Wait();
						auto workT = impl->workT[index].get();
						if (workT->bind != nullptr)
						{ 
							//할당된 작업 수행.
							workT->bind->InvokeCompletelyBind();
							if (workT->info.notifyF != nullptr)
								(*workT->info.notifyF)(workT->info.guid);
							if (workT->info.callBindOnce)
								workT->bind = nullptr; 
						}  
					}
				};

				void(*ptr)(JThreadManagerImpl*, const uint) = threadFLam;

				auto curHandle = handle[index].get();
				curHandle->SetLoopTrigger(true);
				curHandle->CreateThread(ptr, this, std::move(index));
			}
			void EndThread(const uint index)
			{
				auto curHandle = handle[index].get();
				curHandle->SetLoopTrigger(false);
				if (curHandle->IsJoinable())
				{
					if (curHandle->IsWait())
						curHandle->WakeUpThread();
					curHandle->Join();
				}
			} 
			void WakeUpThread(std::unique_ptr<JThread>&& newThread, const uint workQueueIndex)
			{
				//OutputDebugStringA(("WakeUpThread St: " + std::to_string(workQueueIndex) + "\n").c_str());
				workT[workQueueIndex] = std::move(newThread);
				*workT[workQueueIndex]->index = workQueueIndex; 
				//OutputDebugStringA(("WakeUpThread Mid: " + std::to_string(workQueueIndex) + "\n").c_str());
				handle[workQueueIndex]->WakeUpThread(); 
				//OutputDebugStringA(("WakeUpThread Ed: " + std::to_string(workQueueIndex) + "\n").c_str());
			}
			void WaitThread(std::unique_ptr<JThread>&& newThread, const uint waitQueueIndex)
			{
				*newThread->index = invalidIndex;
				waitT[waitQueueIndex].push_back(std::move(newThread));
				//OutputDebugStringA(("WaitThread Index: " + std::to_string(waitQueueIndex) + "\n").c_str());
			}
			void MoveToWorkQueue(const uint workQueueIndex, const uint waitQueueIndex, const uint theadIndex)
			{
				WakeUpThread(std::move(waitT[waitQueueIndex][theadIndex]), workQueueIndex);
				waitT[waitQueueIndex].erase(waitT[waitQueueIndex].begin() + theadIndex);
			}
			//work thread중 wait 상태인 thread search
			int FindWaitThread(const uint woringSetStIndex)const noexcept
			{
				for (uint i = woringSetStIndex; i < threadCount; ++i)
				{
					if (handle[i]->IsWait())
						return i;
				}
				return invalidIndex;
			}
		};
	
		uint JThreadManager::GetReservedSpaceCount(const J_THREAD_USE_CASE_TYPE type)
		{
			return impl->reservedSpace[(int)type];
		}
		void JThreadManager::ExtendCommonThreadCapacity(const uint count)
		{
			impl->ExtendCapacity(lastPriority, count);
		}
		void JThreadManager::ReduceCommonThreadCapacity(const uint count)
		{
			impl->ReduceCapacity(lastPriority, count);
		}
		void JThreadManager::WaitUntilThreadEnd(const JThreadUserHandle& userHandle)
		{
			impl->WaitUntilThreadEnd(userHandle);
		}
		JThreadUserHandle JThreadManager::CreateThread(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind)
		{
			return DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::COMMON, std::move(bind));
		}
		JThreadUserHandle JThreadManager::DoCreateThread(const JThreadInitInfo& initInfo, const J_THREAD_USE_CASE_TYPE useCase, std::unique_ptr<JBindHandleBase>&& bind)
		{
			return impl->CreateThread(std::make_unique<JThread>(std::move(bind), JThreadInfo(initInfo, useCase)));
		}
		JThreadManager::JThreadManager()
			:impl(std::make_unique<JThreadManagerImpl>())
		{}
		JThreadManager::~JThreadManager()
		{
			impl.reset();
		}

		using MainAccess = JThreadManagerPrivate::MainAccess;
		using GraphicInterface = JThreadManagerPrivate::GraphicInterface;

		void MainAccess::Initialize(const uint reservedThreadCount)
		{
			_JThreadManager::Instance().impl->Initialize(reservedThreadCount);
		}
		void MainAccess::Clear()
		{
			_JThreadManager::Instance().impl->Clear();
		}
		void MainAccess::Update()
		{
			_JThreadManager::Instance().impl->Update();
		} 

		JThreadUserHandle GraphicInterface::SetUpdateThreadTask(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind)
		{
			return _JThreadManager::Instance().DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::ENGINE_TASK_SYNC, std::move(bind));
		}
		JThreadUserHandle GraphicInterface::SetDrawThreadTask(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind)
		{
			return _JThreadManager::Instance().DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::ENGINE_TASK_SYNC, std::move(bind));
		}
	}
}