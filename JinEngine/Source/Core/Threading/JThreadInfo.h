#pragma once
#include"JThreadType.h" 
#include<string>

namespace JinEngine
{
	namespace Core
	{
		using ThreadEndNotifyPtr = void(*)(const size_t);
		struct JThreadInitInfo
		{
		public:
			ThreadEndNotifyPtr* notifyF;
			const bool callBindOnce = true;		//관련된 기능들이 미구현 상태이므로 값은 수정불가
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