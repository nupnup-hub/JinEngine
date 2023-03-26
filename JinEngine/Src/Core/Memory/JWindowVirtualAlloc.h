#pragma once 
#include"JAllocationInterface.h"
#include"../Platform/JHardwareInfo.h"
#include<Windows.h> 
#include<assert.h> 

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		class JWindowVirtualAlloc : public JDefaultAlloc
		{
		private:
			using DataPointer = T*; 
		private:
			struct PageInfo
			{
			public:
				uint pageIndex;
				PageInfo* next = nullptr;
			};
			struct BlockInfo
			{
			public:
				uint pageIndex;
				uint blockIndex;
				BlockInfo* next = nullptr;
			public:
				BlockInfo(const uint pageIndex, const uint blockIndex)
					:pageIndex(pageIndex), blockIndex(blockIndex)
				{}
			};
		private:
			/// Pointer to array of allocated blocks.
			DataPointer pData = nullptr;
			bool* isUsePage = nullptr;
			bool* isUseBlock = nullptr;
		private:
			PageInfo* allocablePaegHead = nullptr;
			BlockInfo* allocableBlockHead = nullptr;
		private:
			size_t blockSize = 0;
			size_t pageSize = 0;
			uint committedPageCount = 0;
			uint useBlockCount = 0;
			uint reservedBlockCount = 0;
			uint reservedPageCount = 0;
		private:
			uint lastAllocPageIndex = 0;	//store last page index  caution! it isn't store realloc page index
		public:
			bool Initialize(const uint blockCount, const size_t blockByteSize)final
			{
				reservedBlockCount = blockCount;
				blockSize = blockByteSize; 
				CalculatePageFitAllocationData(blockSize, reservedBlockCount, pageSize, reservedPageCount); 

				pData = (DataPointer)VirtualAlloc(NULL, CalPageSize(reservedPageCount), MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
				if (pData != NULL)
				{
					committedPageCount = useBlockCount = lastAllocPageIndex = 0;
					isUsePage = new bool[reservedPageCount];
					for (uint i = 0; i < reservedPageCount; ++i)
						isUsePage[i] = false;

					isUseBlock = new bool[reservedBlockCount];
					for (uint i = 0; i < reservedBlockCount; ++i)
						isUseBlock[i] = false;

					allocablePaegHead = new PageInfo();
					allocablePaegHead->pageIndex = 0;
					return true;
				}
				else
					return false;
			}
			void* Allocate(const size_t reqSize)final
			{
				if (reqSize < blockSize)
					return nullptr;

				const uint reqBlockCount = reqSize / blockSize;
				if (!CanAllocate(reqBlockCount))
					return DefaultAllocate(blockSize, reqSize);

				DataPointer startP = allocableBlockHead != nullptr ? CommitEmtpyBlock() : CommitEmptyPage();
				for (uint i = 1; i < reqBlockCount; ++i)
				{
					if (allocableBlockHead != nullptr)
						CommitEmtpyBlock();
					else
						CommitEmptyPage();
				}
				return startP;
			}
			void DeAllocate(void* p)final
			{
				if (IsOveredReservedCount() && IsDefaultAllocated(p))
					DefaultDeAllocate(p);
				else
				{
					assert(p >= pData);
					UnUseBlock(GetPageIndex(p), GetBlockIndex(p));
				}
			}
			void DeAllocate(void* p, const size_t size)final
			{
				if (IsOveredReservedCount() && IsDefaultAllocated(p))
					DefaultDeAllocate(p, blockSize, size);
				else
				{
					assert(p >= pData);
					const uint blockCount = size / blockSize;
					for (uint i = 0; i < blockCount; ++i)
					{
						UnUseBlock(GetPageIndex(static_cast<DataPointer>(p) + i),
							GetBlockIndex(static_cast<DataPointer>(p) + i));
					}
				}
			}
			void DeallocateAll()
			{
				for (uint i = 0; i <= lastAllocPageIndex; ++i)
				{
					uint blockSt;
					uint blockEd;
					CalPageInnerBlockIndex(i, blockSt, blockEd);
					for (uint j = blockSt; j <= blockEd; ++j)
					{
						if (isUseBlock[j])
							UnUseBlock(i, j);
					}
				}
				//ReleaseUnusePage();
			}
			void ReleaseUnusePage()final
			{
				for (int i = lastAllocPageIndex; i >= 0; --i)
				{
					if (!isUsePage[i])
						continue;

					uint blockSt;
					uint blockEd;
					CalPageInnerBlockIndex(i, blockSt, blockEd);

					bool isEmpty = true;
					for (uint j = blockSt; j <= blockEd; ++j)
					{
						if (isUseBlock[j])
						{
							isEmpty = false;
							break;
						}
					}

					if (isEmpty)
					{
						if (IsOverlapPage(i) && isUseBlock[blockEd + 1])
							continue;

						BlockInfo** info = &allocableBlockHead;
						while (*info != nullptr)
						{
							if ((*info)->pageIndex == i)
								DeleteInfoPointer(info);
							else
								info = &(*info)->next;
						}
						VirtualFree((PVOID)((std::intptr_t)&pData[0] + CalPageSize(i)), pageSize, MEM_DECOMMIT);
						UnUsePage(i);
					}
				}
			}
			void Release()final
			{
				VirtualFree(pData, 0, MEM_RELEASE);
				delete[] isUsePage;
				delete[] isUseBlock;

				DeleteAllInfoPointer(&allocablePaegHead);
				DeleteAllInfoPointer(&allocableBlockHead);
				pData = nullptr;
				isUsePage = nullptr;
				isUseBlock = nullptr;

				blockSize = pageSize = committedPageCount = useBlockCount = reservedBlockCount = reservedPageCount = 0;
				lastAllocPageIndex = 0;
			}
		public:
			bool CanAllocate(const uint blockCount)const noexcept final
			{
				return useBlockCount + blockCount <= reservedBlockCount;
			}
		protected:
			bool IsDefaultAllocated(void* p)const noexcept final
			{
				return p < pData || &pData[reservedBlockCount -1] < p;
			}
		private:
			bool IsOverlapPage(const uint pageIndex)
			{
				if (pageIndex + 1 >= reservedPageCount)
					return false;

				return CalPageSize(pageIndex) % blockSize > 0;
			}
		private:
			uint GetBlockIndex(void* p)const noexcept
			{
				return  static_cast<DataPointer>(p) - pData;
			}
			uint GetPageIndex(void* p)const noexcept
			{
				uint pageIndex = 0;
				uint blockIndex = static_cast<DataPointer>(p) - pData;
				if (blockIndex > 0)
				{
					pageIndex = CalBlockSize(blockIndex) / pageSize;
					//overlap
					if (pageSize - (CalBlockSize(blockIndex) % pageSize) < blockSize)
						++pageIndex;
				}
				return pageIndex;
			}
		private:
			inline size_t CalPageSize(const uint pageIndex)const noexcept
			{
				return pageSize * (size_t)pageIndex;
			}
			inline size_t CalBlockSize(const uint blockIndex)const noexcept
			{
				return blockSize * (size_t)blockIndex;
			}
			//Unuse
			void CalPageInnerBlockIndex(const uint pageIndex, _Out_ uint& st, _Out_ uint& ed)const noexcept
			{
				if (pageIndex > 0)
				{
					st = CalPageSize(pageIndex) / blockSize;
					ed = st + ((pageSize + (CalPageSize(pageIndex) % blockSize)) / blockSize - 1);
					//st + (pageSize +  preRest) / blcokSize - 1
				}
				else
				{
					st = 0;
					ed = (pageSize / blockSize - 1);
				}
			}
		private:
			template<typename T>
			void DeleteInfoPointer(T** infoP)
			{
				T* temp = *infoP;
				*infoP = (*infoP)->next;
				delete temp;
			}
			template<typename T>
			void DeleteAllInfoPointer(T** infoP)
			{
				T* tar = *infoP;
				while (tar != nullptr)
				{
					T* next = tar->next;
					delete tar;
					tar = next;
				}
				*infoP = nullptr;
			}
		private:
			DataPointer UsePage(const uint pageIndex)noexcept
			{
				++committedPageCount;
				isUsePage[pageIndex] = true;

				if (lastAllocPageIndex < pageIndex)
					lastAllocPageIndex = pageIndex;

				uint blockSt;
				uint blockEd;
				CalPageInnerBlockIndex(pageIndex, blockSt, blockEd);

				BlockInfo* newInfo = new BlockInfo(pageIndex, blockSt);
				allocableBlockHead = newInfo;
				for (uint i = blockSt + 1; i <= blockEd; ++i)
				{
					newInfo->next = new BlockInfo(pageIndex, i);
					newInfo = newInfo->next;
				}

				if (allocablePaegHead->next == nullptr && pageIndex + 1 < reservedPageCount)
				{
					allocablePaegHead->next = new PageInfo();
					allocablePaegHead->next->pageIndex = pageIndex + 1;
				}
				DeleteInfoPointer(&allocablePaegHead);
				UseBlock(pageIndex, blockSt);
				return &pData[blockSt];
			}
			void UnUsePage(const uint pageIndex)noexcept
			{
				--committedPageCount;
				isUsePage[pageIndex] = false;

				PageInfo* newPageInfo = new PageInfo();
				newPageInfo->pageIndex = pageIndex;
				newPageInfo->next = allocablePaegHead;
				allocablePaegHead = newPageInfo;
			}
			DataPointer UseBlock(const uint pageIndex, const uint blockIndex)noexcept
			{
				++useBlockCount;
				isUseBlock[blockIndex] = true;
				DeleteInfoPointer(&allocableBlockHead);
				return &pData[blockIndex];
			}
			void UnUseBlock(const uint pageIndex, const uint blockIndex)noexcept
			{
				--useBlockCount;
				isUseBlock[blockIndex] = false;

				BlockInfo* newBlockInfo = new BlockInfo(pageIndex, blockIndex);
				newBlockInfo->next = allocableBlockHead;
				allocableBlockHead = newBlockInfo;
			}
		private:
			DataPointer CommitEmtpyBlock()noexcept
			{
				if (allocableBlockHead == nullptr)
					return nullptr;

				return UseBlock(allocableBlockHead->pageIndex, allocableBlockHead->blockIndex);
			}
			DataPointer CommitEmptyPage()noexcept
			{
				if (allocablePaegHead == nullptr)
					return nullptr;

				if (VirtualAlloc((PVOID)((std::intptr_t)pData + CalPageSize(allocablePaegHead->pageIndex)), pageSize, MEM_COMMIT, PAGE_READWRITE) == NULL)
					return nullptr;

				return UsePage(allocablePaegHead->pageIndex);
			}
		};
	}
} 