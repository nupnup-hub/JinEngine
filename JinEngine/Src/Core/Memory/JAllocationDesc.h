#pragma once 
#include"../Func/Functor/JFunctor.h"
#include<memory>

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
			static constexpr size_t initDataCount = 32;
		public:
			using ReceiverPtr = void*;
			using ReAllocatedPtr = void*;
			using MemIndex = int;
		public:
			using NotifyReAllocF = JSFunctorType<void, ReceiverPtr, ReAllocatedPtr, MemIndex>;
			using NotifyReAllocB = JBindHandle<NotifyReAllocF::Functor, ReceiverPtr, EmptyParam, EmptyParam>;
		public:
			size_t dataCount = initDataCount;
			size_t dataSize = 0;				//original data size
			size_t alignDataSize = 0;			//automacally calulcated in allocation class
		public:
			std::unique_ptr<NotifyReAllocB> notifyReAllocB;
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