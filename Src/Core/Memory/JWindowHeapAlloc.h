#pragma once
#include<Windows.h>  

namespace JinEngine
{
	namespace Core
	{
		//수정필요
		class JWindowHeapAlloc
		{
		private:
			HANDLE heapHandle;
		public:
			void CreateHeap(size_t heapSize, size_t maxSize);
			void DeleteHeap();
			void* AllocHeap(size_t size);
			void FreeHeap(void* p);
			void PrintHeapInfo();
		};

	}
}
 