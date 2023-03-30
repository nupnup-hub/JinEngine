#pragma once 

namespace JinEngine
{
	namespace Core
	{  
		enum class J_ALLOCATION_TYPE
		{
			DEFAULT,
			VIRTUAL,
			HEAP,
			CUSTOM
		};

		struct JAllocationDesc
		{ 
		public:
			size_t dataCount;
			size_t dataSize;				//original data size
			size_t alignDataSize;			//automacally calulcated in allocation class
		public:
			J_ALLOCATION_TYPE allocationType;
		public:
			int compactionThreshold = 25;
			int memoryCompactBorder = 2;		//execute memory compact condition
		public:
			bool useDataAlign = true;
		public:
			bool useMemoryCompaction = true;
			bool useIncrementalMemoryCompaction = true;
		};
	}
}