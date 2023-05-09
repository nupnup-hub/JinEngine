#include"JHeapAlloc.h" 
#include<assert.h>

namespace JinEngine
{
	namespace Core
	{ 
		bool JHeapAlloc::Initialize(JAllocationDesc&& newDesc)
		{
			desc = std::move(newDesc);
			heapHandle = HeapCreate(0, 0, desc.dataCount * desc.dataSize);
			if (heapHandle != NULL)
			{ 
				reservedBlockCount = desc.dataCount;
				committedBlockCount = 0;
				return true;
			}
			else
				return false;
		}
		void* JHeapAlloc::Allocate(const size_t size)
		{
			if (size < desc.dataSize)
				return nullptr;

			const size_t blockCount = size / desc.dataSize;
			if (!CanAllocate(blockCount))
				return nullptr;

			void* res = HeapAlloc(heapHandle, 0, size);
			if (res == nullptr)
				return nullptr;

			committedBlockCount += blockCount;
			return res;
		}
		void JHeapAlloc::DeAllocate(void* p)
		{
			if (HeapFree(heapHandle, 0, p) == NULL)
			{
				assert("InValid pointer in heap");
				return;
			}
			--committedBlockCount;
		}
		void JHeapAlloc::DeAllocate(void* p, const size_t size)
		{
			if (HeapFree(heapHandle, 0, p) == NULL)
			{
				assert("InValid pointer in heap");
				return;
			}
			committedBlockCount -= (size / desc.dataSize);
		}
		void JHeapAlloc::ReleaseUnusePage()
		{

		}
		void JHeapAlloc::Release()
		{
			HeapDestroy(heapHandle);
			heapHandle = NULL;
			committedBlockCount = reservedBlockCount = 0;
		} 
		bool JHeapAlloc::CanAllocate(const uint blockCount)const noexcept
		{
			return committedBlockCount + blockCount <= reservedBlockCount;
		}
		JAllocationInfo JHeapAlloc::GetInformation()const noexcept
		{
			return JAllocationInfo{};
		}
		/*
		void JHeapAlloc::CreateHeap(size_t heapSize, size_t maxSize)
		{
			heapHandle = HeapCreate(0, 0, 0);
		}
		void JHeapAlloc::DeleteHeap()
		{
			HeapDestroy(heapHandle);
		}
		void* JHeapAlloc::AllocHeap(size_t size)
		{
			void* res = HeapAlloc(heapHandle, 0, size);
			return res;
		}
		void JHeapAlloc::FreeHeap(void* p)
		{
			if (p != nullptr)
				HeapFree(heapHandle, 0, p);
		}
		void JHeapAlloc::PrintHeapInfo()
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
