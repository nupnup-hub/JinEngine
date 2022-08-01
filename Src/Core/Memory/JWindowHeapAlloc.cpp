#include"JWindowHeapAlloc.h" 
#include<stdio.h>

namespace JinEngine
{
	namespace Core
	{
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
	}
}
 