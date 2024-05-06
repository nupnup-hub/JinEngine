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
			using CpuInfo = Core::JHardwareInfo::CpuInfo;
			const CpuInfo cpuInfo = JHardwareInfo::GetCpuInfo();

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