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
#include<mutex>
#include<condition_variable>

#if defined(_WIN32) || defined(_WIN64)
#include<windows.h>
#endif

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			using PriorityNumber = int;
			static constexpr PriorityNumber firstPriority = 0;
			static constexpr PriorityNumber secondPriority = 1;
			static constexpr PriorityNumber lastPriority = 3;
			static constexpr PriorityNumber priorityRange = lastPriority + 1;

			//static constexpr int reservedThreadCount = Graphic::Constants::gMaxFrameThread;
			static bool canLoopThread = true;
		}
		namespace
		{
			static int GetOrderPriority(const J_THREAD_USE_CASE_TYPE type)noexcept
			{
				if (type == J_THREAD_USE_CASE_TYPE::GRAPHIC_DRAW)
					return firstPriority;
				else if (type == J_THREAD_USE_CASE_TYPE::GRAPHIC_UPDATE)
					return secondPriority;
				else if (type == J_THREAD_USE_CASE_TYPE::COMMON)
					return lastPriority;
				else
					return invalidIndex;
			}
			static bool HasReservedSpace(const int priority)
			{
				if (priority == firstPriority)
					return true;
				else if (priority == secondPriority)
					return true;
				else
					return 0;
			}
		}
		namespace
		{
#if defined(_WIN32) || defined(_WIN64)
			class JThreadHandle
			{
			public:
				struct Data
				{
				public:
					HANDLE handle;
					bool isWait = false;
				};
				static std::vector<Data> data;
			public:
				static void Initialize(const int threadCount)
				{
					data.resize(threadCount);
					for (int i = 0; i < threadCount; ++i)
						data[i].handle = CreateEvent(NULL, FALSE, FALSE, NULL);
				}
				static void Clear()
				{
					const int count = (int)data.size();
					for (int i = 0; i < count; ++i)
						CloseHandle(data[i].handle);
					data.clear();
				}
			public:
				static void Wait(const int index)
				{
					ResetEvent(data[index].handle);
					data[index].isWait = true;
					WaitForSingleObject(data[index].handle, INFINITE);
				}
				static void WakeUpThread(const int index)
				{
					data[index].isWait = false;
					SetEvent(data[index].handle);
				}
			public:
				static bool IsWait(const int index)
				{
					return data[index].isWait;
				}
			};
			std::vector<JThreadHandle::Data> JThreadHandle::data;
#else
			class JThreadHandle
			{
			public:
				struct Data
				{
				public:
					std::mutex m;
					std::condition_variable cv;
					bool isWait = false;
					bool allowSetNewThread = false;
				};
			public:
				static std::vector<std::unique_ptr<Data>> data;
				static std::unique_ptr<JFunctor<bool, const int>> canWorkF;
			public:
				static void Initialize(const int threadCount)
				{
					data.resize(threadCount);
					for (int i = 0; i < threadCount; ++i)
						data[i] = std::make_unique<Data>();
					canWorkF = std::make_unique<JFunctor<bool, const int>>(&CanWork);
				}
				static void Clear()
				{
					data.clear();
					canWorkF.reset();
				}
			public:
				static void Wait(const int index)
				{
					//생성시 lock
					std::unique_lock<std::mutex> lk(data[index]->m);
					data[index]->isWait = true;
					//진입시 unlock wait 끝난후 lock
					data[index]->cv.wait(lk, Bind(*canWorkF, index));
				}
				static void WakeUpThread(const int index)
				{
					data[index]->isWait = false;
					std::lock_guard<std::mutex> lk(data[index]->m);
					data[index]->cv.notify_one();
				}
				static bool IsWait(const int index)
				{
					return data[index]->isWait;
				}
				static bool CanWork(const int index)
				{
					return !data[index]->isWait;
				}
			};
			std::vector<std::unique_ptr<JThreadHandle::Data>> JThreadHandle::data;
			std::unique_ptr<JFunctor<bool, const int>> JThreadHandle::canWorkF;
#endif
		}

		class JThreadManager::JThreadManagerImpl
		{
		public:
			std::vector<std::unique_ptr<JThread>> waitT[priorityRange];
			std::vector<std::unique_ptr<JThread>> workT;
			std::vector<std::unique_ptr<std::thread>> thread;
		public:
			int reservedSpace[priorityRange];
			int workingPoolStartIndex[priorityRange];
		public:
			int threadCount = 0;
			uint reservedThreadCount = 0;
		public:
			void Initialize(const uint newReservedThreadCount)
			{
				reservedThreadCount = newReservedThreadCount;
				canLoopThread = true;
				threadCount = std::thread::hardware_concurrency();
				JThreadHandle::Initialize(threadCount);
				workT.resize(threadCount);
				thread.resize(threadCount);

				int reservedSum = 0;
				//stuff reserve space count
				for (int i = 0; i < priorityRange; ++i)
				{
					if (!HasReservedSpace(i))
						reservedSpace[i] = 0;
					else
					{
						if (reservedSum + reservedThreadCount < threadCount)
						{
							reservedSpace[i] = reservedThreadCount;
							reservedSum += reservedThreadCount;
						}
						else
						{
							if (threadCount - reservedSum <= 2)
								reservedSpace[i] = 0;
							else
							{
								reservedSpace[i] = reservedThreadCount / 2;
								reservedSum += reservedThreadCount / 2;
							}
						}
					}
				}
				reservedSum = 0;
				//stuff workingPoolStartIndex
				for (int i = 0; i < priorityRange; ++i)
				{
					workingPoolStartIndex[i] = reservedSum;
					reservedSum += reservedSpace[i];
				}

				for (int i = 0; i < threadCount; ++i)
				{
					workT[i] = std::make_unique<JThread>();
					BeginThread(i);
				}
			}
			void Clear()
			{
				canLoopThread = false;
				for (int i = 0; i < threadCount; ++i)
				{
					if (thread[i] != nullptr && thread[i]->joinable())
					{
						if (JThreadHandle::IsWait(i))
							JThreadHandle::WakeUpThread(i);
						thread[i]->join();
					}
				}
				thread.clear();
				workT.clear();
				for (auto& data : waitT)
					data.clear();
				JThreadHandle::Clear();
			}
		public:
			int GetReservedSpaceCount(const int priority)noexcept
			{
				return reservedSpace[priority];
			}
			int GetWorkingPoolStartIndex(const int priority)noexcept
			{
				return workingPoolStartIndex[priority];
			}
		public:
			size_t CreateThread(std::unique_ptr<JThread>&& newThread)
			{
				const int priorityNumber = GetOrderPriority(newThread->info.useCase);
				const int woringSetStIndex = GetWorkingPoolStartIndex(priorityNumber);
				const size_t guid = newThread->info.guid;

				const int index = FindWaitThread(woringSetStIndex);
				if (index != -1)
				{
					workT[index] = std::move(newThread);
					JThreadHandle::WakeUpThread(index);
				}
				else
					waitT[priorityNumber].push_back(std::move(newThread));
				return guid;
			}
		public:
			void Update()
			{
				for (int i = 0; i < priorityRange; ++i)
				{
					const int woringSetStIndex = GetWorkingPoolStartIndex(i);
					int count = (int)waitT[i].size();

					for (int j = 0; j < count; ++j)
					{
						const int index = FindWaitThread(woringSetStIndex);
						if (index == -1)
							return;

						workT[index] = std::move(waitT[i][j]);
						JThreadHandle::WakeUpThread(index);

						waitT[i].erase(waitT[i].begin() + j);
						count = (int)waitT[i].size();
						--j;
					}
				}
			}
		private:
			void BeginThread(const int index)
			{
				auto threadFLam = [](JThreadManagerImpl* impl, const int index)
				{
					while (canLoopThread)
					{
						JThreadHandle::Wait(index);
						auto& thread = impl->workT[index];
						if (thread->bind != nullptr)
						{
							thread->bind->InvokeCompletelyBind();
							if (thread->info.notifyF != nullptr)
								(*thread->info.notifyF)(thread->info.guid);
							if (thread->info.callBindOnce)
								thread->bind = nullptr;
						}
					}
				};

				thread[index] = std::make_unique<std::thread>(threadFLam, this, index);
			}
			//work thread중 wait 상태인 thread search
			int FindWaitThread(const int woringSetStIndex)const noexcept
			{
				for (int i = woringSetStIndex; i < threadCount; ++i)
				{
					if (JThreadHandle::IsWait(i))
						return i;
				}
				return -1;
			}
		};
	
		uint JThreadManager::GetReservedSpaceCount(const J_THREAD_USE_CASE_TYPE type)
		{
			return impl->reservedSpace[(int)type];
		}
		size_t JThreadManager::CreateThread(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind)
		{
			return DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::COMMON, std::move(bind));
		}
		size_t JThreadManager::DoCreateThread(const JThreadInitInfo& initInfo, const J_THREAD_USE_CASE_TYPE useCase, std::unique_ptr<JBindHandleBase>&& bind)
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

		size_t GraphicInterface::CreateUpdateThread(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind)
		{
			return _JThreadManager::Instance().DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::GRAPHIC_UPDATE, std::move(bind));
		}
		size_t GraphicInterface::CreateDrawThread(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind)
		{
			return _JThreadManager::Instance().DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::GRAPHIC_DRAW, std::move(bind));
		}
	}
}