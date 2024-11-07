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

#pragma once 
#include"JThreadWork.h" 
#include"../Singleton/JSingletonHolder.h"
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		struct JThreadGroupOption
		{
		private:
			static constexpr size_t initSpinLockCount = 10000;
		public:
			size_t spinLockCount = initSpinLockCount;
		};
		class JThreadManager
		{
		private:
			friend class JCreateUsingNew<JThreadManager>;
		private:
			class JThreadManagerImpl;
		private:
			std::unique_ptr<JThreadManagerImpl> impl;
		public:
			JThreadManager();
			~JThreadManager();
		public:
			size_t CreateThreadGroup(const uint newThreadCount, const JThreadGroupOption option = JThreadGroupOption());
			size_t CreateThreadGroup(const uint newThreadCount, const JThreadGroupOption option, bool& isSuccess);
			bool DestroyThreadGroup(const size_t guid);
		public:
			void PushJob(const size_t guid, JobDesc&& desc);
			void PushJobPerThread(const size_t guid, std::vector<JobDesc>& desc);
			void PushJobPerThread(const size_t guid, std::vector<std::vector<JobDesc>>& desc, const uint perThreadJobCount);
			void PushJobPerThread(const size_t guid, JobDesc* desc);
			void PushJobPerThread(const size_t guid, JobDesc* desc, const uint perThreadJobCount);
		public:
			JTlsData* GetTlsData();
		}; 
	}

	using _JThreadManager = Core::JSingletonHolder<Core::JThreadManager>;
}