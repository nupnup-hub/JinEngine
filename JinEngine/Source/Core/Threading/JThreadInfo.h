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
#include"JThreadType.h"  
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		using ThreadEndNotifyPtr = void(*)(const size_t);
		struct JThreadInitInfo
		{
		public:
			ThreadEndNotifyPtr* notifyF;
			//WorkT가 wait상태일시 bind된 func를 교체관련 여부 trigger 관련된 기능들이 미구현 상태이므로 값은 수정불가
			const bool callBindOnce = true;
		public:
			JThreadInitInfo(ThreadEndNotifyPtr* notifyF = nullptr);
		};
		struct JThreadInfo
		{
		public:
			size_t guid;
			ThreadEndNotifyPtr* notifyF = nullptr;
		public:
			J_THREAD_USE_CASE_TYPE useCase = J_THREAD_USE_CASE_TYPE::COMMON;
			J_THREAD_STATE_TYPE state = J_THREAD_STATE_TYPE::WAIT;
		public:
			bool callBindOnce = true;
		public:
			JThreadInfo() = default;
			JThreadInfo(const JThreadInitInfo& initInfo, const J_THREAD_USE_CASE_TYPE useCase);
		};
	}
}