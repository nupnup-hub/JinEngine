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
#include"../Func/Functor/JFunctor.h" 

namespace JinEngine
{
	namespace Core
	{  
		enum class J_ALLOCATION_TYPE
		{
			DEFAULT,
			VIRTUAL,
			HEAP,	//Unuse
			CUSTOM
		};
		 
		struct JAllocationDesc
		{ 
		public:
			static constexpr size_t initDataCount = 64;
		public:
			using ReceiverPtr = void*;
			using ReAllocatedPtr = void*;
			using MemIndex = int;
		public:
			using PreAllocatedPtr = void*;
		public:
			using NotifyReAllocF = JSFunctorType<void, ReceiverPtr, ReAllocatedPtr, MemIndex>;
			using NotifyReAllocB = JBindHandle<NotifyReAllocF::Functor, ReceiverPtr, EmptyParam, EmptyParam>;
		public:
			using NotifyReAllocDebugF = JSFunctorType<void, ReceiverPtr, PreAllocatedPtr, ReAllocatedPtr, MemIndex>;
			using NotifyReAllocDebugB = JBindHandle<NotifyReAllocDebugF::Functor, ReceiverPtr, EmptyParam, EmptyParam, EmptyParam>;
		public:
			std::string name; //Debug
		public:
			size_t dataCount = initDataCount;
			size_t dataSize = 0;				//original data size
			size_t alignDataSize = 0;			//automacally calulcated in allocation class
		public:
			std::unique_ptr<NotifyReAllocB> notifyReAllocB;
			std::unique_ptr<NotifyReAllocDebugB> notifyDebugB;
		public:
			J_ALLOCATION_TYPE allocationType = J_ALLOCATION_TYPE::VIRTUAL;
		public:
			int compactionThreshold = 25;
			int memoryCompactBorder = 2;		//execute memory compact condition
			float extendFactor = 2;
		public:
			bool useDataAlign = true;
			bool fitAllocationGranularity = true;
		public:
			bool useMemoryCompaction = true;
			bool useIncrementalMemoryCompaction = true;
		public:
			bool canReAlloc = true;
		};
	}
}