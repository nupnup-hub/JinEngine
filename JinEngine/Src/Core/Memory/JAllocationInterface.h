#pragma once
#include<sal.h>
#include"../JDataType.h" 
#include"JAllocationDesc.h"

namespace JinEngine
{
	namespace Core
	{
		struct JAllocInfo
		{
		public:
			size_t totalReserveSize = 0;
			size_t totalCommittedSize = 0;
		public:
			size_t reservePageCount = 0;
			size_t committedPageCount = 0;
		public:
			size_t reserveBlockCount = 0;
			size_t committedBlockCount = 0;
		public:
			size_t useBlockCount = 0;
		public:
			size_t oriBlockSize = 0;
			size_t allocBlockSize = 0;
		public:
			J_ALLOCATION_TYPE allocType = J_ALLOCATION_TYPE::DEFAULT;
		};

		class JAllocationInterface
		{ 
		public:
			virtual ~JAllocationInterface() = default;
		public:
			virtual bool Initialize(JAllocationDesc newDesc) = 0;
			virtual void* Allocate(const size_t size) = 0;
			//virtual void* AllocateArray(const size_t size) = 0;
			virtual void DeAllocate(void* p) = 0;
			virtual void DeAllocate(void* p, const size_t size) = 0;
			virtual void ReleaseUnusePage() = 0;
			virtual void Release() = 0;
		public:
			virtual bool CanAllocate(const uint blockCount)const noexcept = 0;
		public:
			virtual JAllocInfo GetInformation()const noexcept = 0;
		public:
			static void CalculatePageFitAllocationData(_Inout_ size_t& blockSize,
				const uint blockCount,
				_Out_ size_t& pageSize,
				_Out_ uint& paegCount,
				const bool useBlockAlign);
		};			 
	}
}