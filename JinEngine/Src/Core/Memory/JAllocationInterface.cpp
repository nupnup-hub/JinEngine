#include"JAllocationInterface.h"
#include"../Platform/JHardwareInfo.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		void JAllocationInterface::CalculatePageFitAllocationData(_Inout_ size_t& blockSize,
			const uint blockCount,
			_Out_ size_t& pageSize,
			_Out_ uint& paegCount,
			const bool useBlockAlign)
		{ 
			using CpuInfo = Core::JHardwareInfoImpl::CpuInfo;
			const CpuInfo cpuInfo = JHardwareInfo::Instance().GetCpuInfo();

			const size_t hPageSize = cpuInfo.pageSize;
			const size_t hAllocationGranularity = cpuInfo.allocationGranularity;

			if (useBlockAlign && blockSize % MEMORY_ALLOCATION_ALIGNMENT)
				blockSize = (blockSize / MEMORY_ALLOCATION_ALIGNMENT + 1) * MEMORY_ALLOCATION_ALIGNMENT;

			pageSize = hPageSize;
			//add page size until pageSize is four times bigger than blockByteSize
			while (pageSize < blockSize)
				pageSize += hPageSize;

			const size_t totalBlockByte = blockCount * blockSize;
			if (totalBlockByte % pageSize)
				paegCount = (totalBlockByte / pageSize) + 1;
			else
				paegCount = (totalBlockByte / pageSize);
			/* don't use
			*
			size_t totalReserveByte = (paegCount * pageSize);
			size_t allocPageBoundary = hAllocationGranularity;

			if (totalReserveByte > allocPageBoundary)
			{
				//add 64kb until totalReserveByte is smaller than allocPageBoundary
				while (totalReserveByte > allocPageBoundary)
					allocPageBoundary += hAllocationGranularity;
				paegCount += ((allocPageBoundary - totalReserveByte) / pageSize);
				blockCount = (paegCount * pageSize) / blockSize;
			}
			*/
		}

		void* JDefaultAlloc::DefaultAllocate(const size_t blockSize, const size_t reqSize)
		{  
			if (void* ptr = std::malloc(reqSize))
			{
				committedBlockCount += (reqSize / blockSize);
				return ptr;
			}

			throw std::bad_alloc{};
		}
		void JDefaultAlloc::DefaultDeAllocate(void* p)
		{
			std::free(p);
			--committedBlockCount;
		}
		void JDefaultAlloc::DefaultDeAllocate(void* p, const size_t blockSize, const size_t reqSize)
		{
			std::free(p);
			committedBlockCount -= (reqSize / blockSize);
		}
		bool JDefaultAlloc::IsOveredReservedCount()const noexcept
		{
			return committedBlockCount > 0;
		}
	}
}