#pragma once
#include"JAllocationInterface.h"
#include<Windows.h>  

namespace JinEngine
{
	namespace Core
	{ 
		//unuse
		class JHeapAlloc : public JDefaultAlloc
		{
		private:
			using DataPointer = BYTE*; 
		private:
			HANDLE heapHandle; 
			JAllocationDesc desc; 
		private:
			size_t committedBlockCount = 0; 
			size_t reservedBlockCount = 0;
		public:
			bool Initialize(JAllocationDesc newDesc)final;
			void* Allocate(const size_t size)final;
			void DeAllocate(void* p)final;
			void DeAllocate(void* p, const size_t size) final;
			void ReleaseUnusePage()final;
			void Release()final;
		public:
			bool CanAllocate(const uint blockCount)const noexcept final;
		protected:
			bool IsDefaultAllocated(void* p)const noexcept final;
		};
	}
}
 