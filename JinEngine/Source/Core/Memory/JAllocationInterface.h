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
#include<sal.h> 
#include"JAllocationDesc.h"
#include"../JCoreEssential.h"  
 
namespace JinEngine
{
	namespace Core
	{
		struct JAllocationInfo
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
			virtual bool Initialize(JAllocationDesc&& newDesc) = 0;
			virtual void* Allocate(const size_t size) = 0;
			//virtual void* AllocateArray(const size_t size) = 0;
			virtual void DeAllocate(void* p) = 0;
			virtual void DeAllocate(void* p, const size_t size) = 0;
			virtual void ReleaseUnusePage() = 0;
			virtual void Release() = 0;
		public:
			virtual bool CanAllocate(const uint blockCount)const noexcept = 0;
		public:
			virtual size_t GetAlignedAllocBlockSize()const noexcept = 0;
			virtual JAllocationInfo GetInformation()const noexcept = 0;			
		public:
			static void CalculatePageFitAllocationData(_Inout_ size_t& blockSize,
				_Inout_ uint& blockCount,
				_Out_ size_t& pageSize,
				_Out_ uint& paegCount,
				_Out_ size_t& allocSize,
				const bool useBlockAlign,
				const bool fitAllocationGranularity = true);
		};			 
	}
}