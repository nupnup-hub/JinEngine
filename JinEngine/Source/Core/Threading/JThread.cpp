#include"JThread.h"

namespace JinEngine
{
	namespace Core
	{
		JThreadInfo JThread::GetInfo()const noexcept
		{
			return info;
		}

		JThread::JThread(std::unique_ptr<JBindHandleBase>&& bind, const JThreadInfo& info)
			:bind(std::move(bind)), info(info)
		{}
	}
}