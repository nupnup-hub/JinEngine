#pragma once
#include"../Func/Functor/JFunctor.h"
#include"JThreadInfo.h"

namespace JinEngine
{
	namespace Core
	{
		class JThreadManager;
		class JThread
		{
		private:
			friend class JThreadManager;
		public:
			std::unique_ptr<JBindHandleBase> bind;
			JThreadInfo info;
		public:
			JThreadInfo GetInfo()const noexcept;
		public:
			JThread() = default;
			JThread(std::unique_ptr<JBindHandleBase>&& bind, const JThreadInfo& info);
		};
	}
}