#pragma once
#include"JAllocationInterface.h"
#include<Windows.h>  

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		class JWindowHeapAlloc : public JDefaultAlloc
		{
		private:
			using DataPointer = T*; 
		private:
			HANDLE heapHandle; 
			size_t blockSize;
		private:
			size_t committedBlockCount = 0; 
			size_t reservedBlockCount = 0;
		public:
			bool Initialize(const uint blockCount, const size_t blockByteSize)final
			{
				heapHandle = HeapCreate(0, 0, blockCount * blockByteSize);
				if (heapHandle != NULL)
				{
					blockSize = blockByteSize;
					reservedBlockCount = blockCount;
					committedBlockCount = 0;
					return true;
				}
				else
					return false;
			}
			void* Allocate(const size_t size)final
			{
				if (size < blockSize)
					return nullptr;

				const size_t blockCount = size / blockSize;
				if (!CanAllocate(blockCount))
					return DefaultAllocate(blockSize, size);

				void* res = HeapAlloc(heapHandle, 0, size);
				if (res == nullptr)
					return nullptr;

				committedBlockCount += blockCount;
				return res;
			}
			void DeAllocate(void* p)final
			{
				if (IsOveredReservedCount() && IsDefaultAllocated(p))
					DefaultDeAllocate(p);
				else
				{
					if (HeapFree(heapHandle, 0, p) == NULL)
					{ 
						assert("InValid pointer in heap");
						return;
					}
					--committedBlockCount;
				}				 
			}
			void DeAllocate(void* p, const size_t size) final 
			{
				if (IsOveredReservedCount() && IsDefaultAllocated(p))
					DefaultDeAllocate(p, blockSize, size);
				else
				{
					if (HeapFree(heapHandle, 0, p) == NULL)
					{
						assert("InValid pointer in heap");
						return;
					}
					committedBlockCount -= (size / blockSize);
				}				 
			}
			void ReleaseUnusePage()final
			{

			}
			void Release()final
			{
				HeapDestroy(heapHandle);
				heapHandle = NULL;
				blockSize = committedBlockCount = reservedBlockCount = 0;
			}
		public:
			bool CanAllocate(const uint blockCount)const noexcept final
			{
				return committedBlockCount + blockCount <= reservedBlockCount;
			}
		protected:
			bool IsDefaultAllocated(void* p)const noexcept final
			{
				return HeapSize(heapHandle, 0, p) > 0;
			}
		};

		/*
		void JWindowHeapAlloc::CreateHeap(size_t heapSize, size_t maxSize)
		{
			heapHandle = HeapCreate(0, 0, 0);
		}
		void JWindowHeapAlloc::DeleteHeap()
		{
			HeapDestroy(heapHandle);
		}
		void* JWindowHeapAlloc::AllocHeap(size_t size)
		{
			void* res = HeapAlloc(heapHandle, 0, size);
			return res;
		}
		void JWindowHeapAlloc::FreeHeap(void* p)
		{
			if (p != nullptr)
				HeapFree(heapHandle, 0, p);
		}
		void JWindowHeapAlloc::PrintHeapInfo()
		{
			DWORD heapIndex;
			DWORD heapCount = 0;
			PHANDLE heaps = NULL;
			while (TRUE)
			{
				DWORD actualHeapCount = GetProcessHeaps(heapCount, heaps);
				if (actualHeapCount <= heapCount)
				{
					break;
				}
				heapCount = actualHeapCount;
				free(heaps);
				heaps = (HANDLE*)malloc(heapCount * sizeof(HANDLE));
				if (heaps == NULL)
				{
					printf("Unable to allocate memory for list of heaps\n");
					return;
				}
			}
			for (heapIndex = 0; heapIndex < heapCount; heapIndex++)
			{
				printf("Heap ID: %d\n", (DWORD)(ULONG_PTR)heaps[heapIndex]);
				PROCESS_HEAP_ENTRY entry;
				entry.lpData = NULL;
				while (HeapWalk(heaps[heapIndex], &entry))
				{
					// Heap32First and Heap32Next ignore entries
					// with the PROCESS_HEAP_REGION flag
					if (!(entry.wFlags & PROCESS_HEAP_REGION))
						printf("Block size: %d .. ptr: %d  .. index: %d \n", entry.cbData + entry.cbOverhead, entry.lpData, entry.iRegionIndex);
				}
			}
		}
		*/
	}
}
 