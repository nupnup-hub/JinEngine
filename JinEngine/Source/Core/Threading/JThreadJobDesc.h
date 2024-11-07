#pragma once
#include"JSync.h"
#include"JThreadType.h" 
#include"../Func/Functor/JFunctor.h" 

namespace JinEngine
{
	namespace Core
	{ 
		struct JobDesc
		{
		public:
			std::unique_ptr<JBindHandleBase> func = nullptr;
		public:
			AtomicBoolen* notifyAtomic = nullptr;
			JKernelEvent* notifyEvent = nullptr;
		}; 
	}
}