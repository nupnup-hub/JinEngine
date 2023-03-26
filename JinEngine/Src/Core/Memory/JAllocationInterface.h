#pragma once
#include<sal.h>
#include"../JDataType.h"

namespace JinEngine
{
	namespace Core
	{
		class JAllocationInterface
		{ 
		public:
			virtual ~JAllocationInterface() = default;
		public:
			virtual bool Initialize(const uint blockCount, const size_t blockSize) = 0;
			virtual void* Allocate(const size_t size) = 0;
			//virtual void* AllocateArray(const size_t size) = 0;
			virtual void DeAllocate(void* p) = 0;
			virtual void DeAllocate(void* p, const size_t size) = 0;
			virtual void ReleaseUnusePage() = 0;
			virtual void Release() = 0;
		public:
			virtual bool CanAllocate(const uint blockCount)const noexcept = 0;
		public:
			static void CalculatePageFitAllocationData(const size_t blockSize,
				_Inout_ uint& blockCount,
				_Out_ size_t& pageSize,
				_Out_ uint& paegCount);
		};			 

		class JDefaultAlloc : public JAllocationInterface
		{
		private:
			int committedBlockCount = 0;
		protected: 
			void* DefaultAllocate(const size_t blockSize, const size_t reqSize);
			void DefaultDeAllocate(void* p);
			void DefaultDeAllocate(void* p, const size_t blockSize, const size_t reqSize);
		protected:
			bool IsOveredReservedCount()const noexcept;
			virtual bool IsDefaultAllocated(void* p)const noexcept = 0;
		};
	}
}