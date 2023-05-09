#include"JAllocationInterface.h"
#include"../Platform/JHardwareInfo.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		void JAllocationInterface::CalculatePageFitAllocationData(_Inout_ size_t& blockSize,
			_Inout_ uint& blockCount,
			_Out_ size_t& pageSize,
			_Out_ uint& paegCount,
			_Out_ size_t& allocSize,
			const bool useBlockAlign,
			const bool fitAllocationGranularity)
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

			size_t totalByte = blockCount * blockSize;
			if (fitAllocationGranularity)
			{
				size_t granularityCount = 0;

				if (totalByte % hAllocationGranularity)
					granularityCount = (totalByte / hAllocationGranularity) + 1;
				else
					granularityCount = (totalByte / hAllocationGranularity);
				totalByte = granularityCount * hAllocationGranularity;
				paegCount = (totalByte / pageSize);
				blockCount = ((paegCount * pageSize) / blockSize);

				allocSize = totalByte;
			}
			else
			{
				if (totalByte % pageSize)
					paegCount = (totalByte / pageSize) + 1;
				else
					paegCount = (totalByte / pageSize);
				allocSize = paegCount * pageSize;
			}
		}
	}
}