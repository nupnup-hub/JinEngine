#pragma once
#include<sal.h>
#include"../JDataType.h" 
#include"JAllocationDesc.h"

namespace JinEngine
{
	namespace Core
	{
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
			static void CalculatePageFitAllocationData(_Inout_ size_t& blockSize,
				const uint blockCount,
				_Out_ size_t& pageSize,
				_Out_ uint& paegCount,
				const bool useBlockAlign);
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