#pragma once 
#include"../Platform/JHardwareInfo.h"
#include<Windows.h> 
#include<assert.h> 

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		class WindowVirtualAlloc
		{
		private:
			/// Pointer to array of allocated blocks.
			typedef T* Data;
			Data pData;
			size_t blockSize;
			size_t committedBlockCount;
			size_t reservedBlockCount;
			size_t committedPageCount;
			size_t pageSize;
			unsigned int unusedPageLimit;
		public:
			bool Init(const size_t reservedBlockCount, const size_t blockSize, const unsigned int unusedPageLimit = 0)
			{
				WindowVirtualAlloc::blockSize = blockSize;
				pData = (Data)VirtualAlloc(NULL, reservedBlockCount * blockSize, MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
				if (pData != NULL)
				{
					committedBlockCount = 0;
					WindowVirtualAlloc::reservedBlockCount = reservedBlockCount;
					committedPageCount = 0;
					pageSize = JHardwareInfo::Instance()->GetCpuInfo().pageSize;
					WindowVirtualAlloc::unusedPageLimit = unusedPageLimit;
					return true;
				}
				else
					return false;
			}
			void* Allocate(size_t reqSize)
			{
				if (committedBlockCount == reservedBlockCount)
					return NULL;

				size_t availableSize = (reservedBlockCount - committedBlockCount) * blockSize;
				if (availableSize < reqSize)
					return NULL;

				if (reqSize % blockSize != 0)
					reqSize = (reqSize / blockSize) * blockSize + blockSize;

				const size_t reqMaxByte = committedBlockCount * blockSize + reqSize;
				const size_t nowPageMaxByte = pageSize * committedPageCount;

				if (reqMaxByte > nowPageMaxByte)
				{
					void* res = NULL;
					if (VirtualAlloc(&pData[committedBlockCount], reqSize, MEM_COMMIT, PAGE_READWRITE) != NULL)
					{
						res = &pData[committedBlockCount];
						const size_t gap = reqMaxByte - nowPageMaxByte;
						committedBlockCount += (reqSize / blockSize);
						if (gap % pageSize == 0)
							committedPageCount += (gap / pageSize);
						else
							committedPageCount += (gap / pageSize) + 1;
					}
					return res;
				}
				else
				{
					void* res = &pData[committedBlockCount];
					committedBlockCount += (reqSize / blockSize);
					return res;
				}
			}
			void Deallocate(void* p, size_t reqSize)
			{
				assert(p >= pData);
				const size_t gap = (pageSize * committedPageCount) - (committedBlockCount * blockSize);
				const unsigned int sparePageCount = (unsigned int)(gap / pageSize);
				if (sparePageCount > unusedPageLimit)
				{
					VirtualFree(pData + (committedPageCount - sparePageCount) * pageSize, sparePageCount * pageSize, MEM_RELEASE);
					committedPageCount -= sparePageCount;
				}
				const unsigned int blockCount = (int)(reqSize / blockSize);
				if (committedBlockCount <= blockCount)
					committedBlockCount = 0;
				else
					committedBlockCount -= blockCount;
				//p = nullptr;
			}
			void ReleaseUnusePage()
			{
				const size_t gap = (pageSize * committedPageCount) - (committedBlockCount * blockSize);
				const unsigned int sparePageCount = gap / pageSize;
				if (sparePageCount > unusedPageLimit)
				{
					VirtualFree(pData + (committedPageCount - sparePageCount) * pageSize, sparePageCount * pageSize, MEM_RELEASE);
					committedPageCount -= sparePageCount;
				}
			}
			void Release()
			{
				VirtualFree(pData, 0, MEM_RELEASE);
			}
		};
	}
}
/*
* 
		cout << "Rest: " << committedBlockCount * blockSize << " " << committedPageCount * pageSize << endl;
		cout << "De: " << (committedBlockCount * blockSize) << " " << (pageSize * committedPageCount) << endl;
		cout << "Rest: " << committedBlockCount * blockSize << " " << committedPageCount * pageSize << endl;

			cout << "IF: " << reqMaxByte << " " << nowPageMaxByte << endl;
					cout << "Just" << endl;
					cout << "Ano" << endl;
					cout << "B: " << res << " " << reqSize << " " << committedBlockCount << " " << committedPageCount << " " << pageSize * committedPageCount << endl;
//cout<<"B: " << res << " " << &pData[committedBlockCount] << " " << reqSize << " " << committedBlockCount << endl;
//cout<<"B: " << res << " " << &pData[committedBlockCount] << " " << reqSize << " " << committedBlockCount << endl;
			cout << pageSize << " " << committedPageCount << " " << committedBlockCount << " " << blockSize << endl;
			cout << gap << " " << sparePageCount << endl;
			cout << committedPageCount * pageSize << endl;
			cout << pData + (committedPageCount - sparePageCount) * pageSize << endl;
			cout << committedPageCount * pageSize << endl;
*/