#include"JVirtualAlloc.h" 
#include"JAllocationDesc.h"
#include"../Platform/JHardwareInfo.h"
#include"../Platform/JPlatformInfo.h"
#include<assert.h> 
#include<Windows.h> 
#include<string>
#include<fstream>


namespace JinEngine
{
	namespace Core
	{
		struct ReserveInfo
		{
		public:
			size_t size;
		};
		class JVirtualAllocApiInterface
		{
		public:
			virtual void* ReserveVirtualMemory(ReserveInfo* reserveInfo) = 0;
			virtual void ReleaseVirtualMemory(void* p) = 0;
		public:
			virtual void* Allocate(void* p, const size_t reqSize) = 0;
			virtual void DeAllocate(void* p, const size_t reqSize) = 0;
		};
		class JWindowApiInterface : public JVirtualAllocApiInterface
		{
		public:
			void* ReserveVirtualMemory(ReserveInfo* reserveInfo)final
			{
				return VirtualAlloc(NULL, reserveInfo->size, MEM_RESERVE, PAGE_READWRITE);
			}
			void ReleaseVirtualMemory(void* p)final
			{
				VirtualFree(p, 0, MEM_RELEASE);
			}
		public:
			void* Allocate(void* p, const size_t reqSize)final
			{
				return VirtualAlloc(p, reqSize, MEM_COMMIT, PAGE_READWRITE);
			}
			void DeAllocate(void* p, const size_t reqSize)final
			{
				VirtualFree(p, reqSize, MEM_DECOMMIT);
			}
		};

		JVirtualAlloc::BlockInfo::BlockInfo(const uint pageIndex, const uint blockIndex)
			:pageIndex(pageIndex), blockIndex(blockIndex)
		{}


		JVirtualAlloc::JVirtualAlloc()
		{
			auto osName = GetOsName();
			if (osName == "Window")
				apiInterface = new JWindowApiInterface();
		}
		JVirtualAlloc::~JVirtualAlloc()
		{
			delete apiInterface;
		}
		bool JVirtualAlloc::Initialize(JAllocationDesc&& newDesc)
		{
			desc = std::move(newDesc);
			reservedBlockCount = desc.dataCount;
			 
			oriBlockSize = desc.dataSize;
			allocBlockSize = oriBlockSize;
			 
			CalculatePageFitAllocationData(allocBlockSize, 
				reservedBlockCount,
				pageSize, 
				reservedPageCount, 
				totalAllocSize,
				desc.useDataAlign,
				desc.fitAllocationGranularity);
			desc.alignDataSize = allocBlockSize;

			ReserveInfo info;
			info.size = totalAllocSize;
			pData = (DataPointer)apiInterface->ReserveVirtualMemory(&info);
			//pData = (DataPointer)VirtualAlloc(NULL, CalPageSize(reservedPageCount), MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
			if (pData != NULL)
			{
				committedPageCount = useBlockCount = lastAllocPageIndex = 0;
				isUsePage = new bool[reservedPageCount]();
				isUseBlock = new bool[reservedBlockCount]();
				//new type[count]() () is special syntax for value-initializing an array:

				allocablePaegHead = new PageInfo();
				allocablePaegHead->pageIndex = 0;
				return true;
			}
			else
				return false;
		}
		void* JVirtualAlloc::Allocate(const size_t reqSize)
		{  
			if (reqSize < oriBlockSize)
				return nullptr;

			const uint reqBlockCount = (uint)(reqSize / oriBlockSize);
			if (!CanAllocate(reqBlockCount))
			{	 
				if(CompactUnuseMemory(false))
					return Allocate(reqSize);
				else if (desc.canReAlloc && desc.notifyReAllocB != nullptr && Extend())
					return Allocate(reqSize);
				else
					return nullptr;
			}

			if (reqBlockCount == 1)
			{
				if (allocableBlockHead != nullptr)
					return CommitEmtpyBlock();
				else
					return CommitEmptyPage();
			}
			else
			{
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
		}
		void JVirtualAlloc::DeAllocate(void* p)
		{
			assert(p >= pData);
			UnUseBlock(p);
		}
		void JVirtualAlloc::DeAllocate(void* p, const size_t size)
		{
			assert(p >= pData);
			const uint blockCount = (uint)size / allocBlockSize;
			for (uint i = 0; i < blockCount; ++i)
				UnUseBlock(p);
		}
		void JVirtualAlloc::DeallocateAll()
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
		void JVirtualAlloc::ReleaseUnusePage()
		{
			if (desc.useMemoryCompaction)
				CompactUnuseMemory(true);
			else
				FreeUnuseMemory();
		}
		void JVirtualAlloc::Release()
		{
			apiInterface->ReleaseVirtualMemory(pData);
			//VirtualFree(pData, 0, MEM_RELEASE);
			delete[] isUsePage;
			delete[] isUseBlock;

			DeleteAllInfoPointer(&allocablePaegHead);
			DeleteAllInfoPointer(&allocableBlockHead);
			pData = nullptr;
			isUsePage = nullptr;
			isUseBlock = nullptr;

			allocBlockSize = pageSize = committedPageCount = useBlockCount = reservedBlockCount = reservedPageCount = 0;
			lastAllocPageIndex = 0;
		} 
		bool JVirtualAlloc::CanAllocate(const uint blockCount)const noexcept
		{
			return allocableBlockHead != nullptr || committedPageCount < reservedPageCount; 
		} 
		bool JVirtualAlloc::CanCompactMemory(const bool allowCompareBorder)const noexcept
		{
			if (allowCompareBorder)
				return GetDeAllocatedBlockCount() >= desc.memoryCompactBorder && desc.notifyReAllocB != nullptr;
			else
				return GetDeAllocatedBlockCount() > 0 && desc.notifyReAllocB != nullptr;
		}
		bool JVirtualAlloc::IsOverlapPage(const uint pageIndex)const noexcept
		{
			if (pageIndex + 1 >= reservedPageCount)
				return false;

			return CalPageSize(pageIndex) % allocBlockSize > 0;
		} 
		uint JVirtualAlloc::GetBlockIndex(void* p)const noexcept
		{
			return ((std::intptr_t)p - (std::intptr_t)pData) / allocBlockSize;
		}
		uint JVirtualAlloc::GetPageIndex(void* p)const noexcept
		{
			return GetPageIndex(GetBlockIndex(p));
		}
		uint JVirtualAlloc::GetPageIndex(const uint blockIndex)const noexcept
		{
			uint pageIndex = 0;
			if (blockIndex > 0)
			{
				pageIndex = CalBlockSize(blockIndex) / pageSize;
				//overlap
				if (pageSize - (CalBlockSize(blockIndex) % pageSize) < allocBlockSize)
					++pageIndex;
			}
			return pageIndex;
		}
		uint JVirtualAlloc::GetCommittedBlockCount()const noexcept
		{
			return (committedPageCount * pageSize) / allocBlockSize;
		}
		uint JVirtualAlloc::GetAllocableBlockCount()const noexcept
		{
			uint count = 0;
			BlockInfo* head = allocableBlockHead;
			while (head != nullptr)
			{
				++count;
				head = head->next;
			}
			return count;
		}
		uint JVirtualAlloc::GetAllocatedBlockCount()const noexcept
		{
			const uint committedBlockCount = GetCommittedBlockCount();
			if (committedBlockCount > reservedBlockCount)
				return reservedBlockCount - GetAllocableBlockCount();
			else
				return committedBlockCount - GetAllocableBlockCount();
		}
		uint JVirtualAlloc::GetDeAllocatedBlockCount()const noexcept
		{
			const uint committedBlockCount = GetCommittedBlockCount();
			if (committedBlockCount > reservedBlockCount)
				return reservedBlockCount - (useBlockCount + GetAllocableBlockCount());
			else
				return committedBlockCount - (useBlockCount + GetAllocableBlockCount());
		}
		JAllocationInfo JVirtualAlloc::GetInformation()const noexcept
		{
			JAllocationInfo info;
			info.totalReserveSize = totalAllocSize;
			info.totalCommittedSize = committedPageCount * pageSize;
			info.reservePageCount = reservedPageCount;
			info.committedPageCount = committedPageCount;
			info.reserveBlockCount = reservedBlockCount;
			info.committedBlockCount = (committedPageCount * pageSize) / allocBlockSize;
			info.useBlockCount = useBlockCount;

			info.oriBlockSize = oriBlockSize;
			info.allocBlockSize = allocBlockSize;
			info.allocType = J_ALLOCATION_TYPE::VIRTUAL;
			return info;
		}
		void JVirtualAlloc::CalPageInnerBlockIndex(const uint pageIndex, _Out_ uint& st, _Out_ uint& ed)const noexcept
		{
			if (pageIndex > 0)
			{
				st = CalPageSize(pageIndex) / allocBlockSize;
				ed = st + ((pageSize + (CalPageSize(pageIndex) % allocBlockSize)) / allocBlockSize - 1);
				//st + (pageSize +  preRest) / blcokSize - 1
			}
			else
			{
				st = 0;
				ed = (pageSize / allocBlockSize - 1);
			}
			if (ed >= reservedBlockCount)
				ed = reservedBlockCount - 1;
		} 
		JVirtualAlloc::DataPointer JVirtualAlloc::UsePage(const uint pageIndex)noexcept
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
			return CalPtrLocation(blockSt);
		}
		void JVirtualAlloc::UnUsePage(const uint pageIndex)noexcept
		{
			--committedPageCount;
			isUsePage[pageIndex] = false;

			PageInfo* newPageInfo = new PageInfo();
			newPageInfo->pageIndex = pageIndex;
			newPageInfo->next = allocablePaegHead;
			allocablePaegHead = newPageInfo;
		}
		JVirtualAlloc::DataPointer JVirtualAlloc::UseBlock(const uint pageIndex, const uint blockIndex)noexcept
		{
			++useBlockCount;
			isUseBlock[blockIndex] = true;
			DeleteInfoPointer(&allocableBlockHead);
			return CalPtrLocation(blockIndex);
		}
		void JVirtualAlloc::UnUseBlock(const uint pageIndex, const uint blockIndex)noexcept
		{
			--useBlockCount; 
			isUseBlock[blockIndex] = false;

			if (!desc.useMemoryCompaction)
			{
				BlockInfo* newBlockInfo = new BlockInfo(pageIndex, blockIndex);
				newBlockInfo->next = allocableBlockHead;
				allocableBlockHead = newBlockInfo;
			}
		} 
		void JVirtualAlloc::UnUseBlock(void* p)noexcept
		{
			const uint blockIndex = GetBlockIndex(p);
			UnUseBlock(GetPageIndex(blockIndex), blockIndex);
		}
		JVirtualAlloc::DataPointer JVirtualAlloc::CommitEmtpyBlock()
		{
			if (allocableBlockHead == nullptr)
				return nullptr;

			return UseBlock(allocableBlockHead->pageIndex, allocableBlockHead->blockIndex);
		}
		JVirtualAlloc::DataPointer JVirtualAlloc::CommitEmptyPage()
		{
			if (allocablePaegHead == nullptr)
			{
				int unuseCount = 0;
				for (uint i = 0; i < reservedBlockCount; ++i)
				{
					if (!isUseBlock[i])
						++unuseCount;
				}
				//MessageBox(0, L"useBlockCount", std::to_wstring(useBlockCount).c_str(), 0);
				//MessageBox(0, L"unuseCount", std::to_wstring(unuseCount).c_str(), 0);
				//MessageBox(0, L"reservedBlockCount", std::to_wstring(reservedBlockCount).c_str(), 0);
				return nullptr;
			}

			if (apiInterface->Allocate((PVOID)((std::intptr_t)pData + CalPageSize(allocablePaegHead->pageIndex)), pageSize) == NULL)
				return nullptr;
			return UsePage(allocablePaegHead->pageIndex);
		}
		bool JVirtualAlloc::CompactUnuseMemory(const bool allowCompareBorder)
		{
			if (!CanCompactMemory(allowCompareBorder))
				return false;

			const uint allocatedBlockCount = GetAllocatedBlockCount();
			for (uint i = 0; i < allocatedBlockCount; ++i)
			{ 
				if (!isUseBlock[i])
				{
					bool findUseBlock = false;
					uint movedBlockIndex = 0;
					for (uint j = i + 1; j < allocatedBlockCount; ++j)
					{
						if (isUseBlock[j])
						{
							findUseBlock = true;
							movedBlockIndex = j;
							break;
						}
					}

					if (!findUseBlock)
						break;

					DataPointer emptyPtr = CalPtrLocation(i);
					DataPointer movedPtr = CalPtrLocation(movedBlockIndex);
					memcpy(emptyPtr, movedPtr, allocBlockSize); 
 
					isUseBlock[i] = true;
					isUseBlock[movedBlockIndex] = false;
					 
					(*desc.notifyReAllocB)(emptyPtr, movedBlockIndex);
					//if (desc.notifyDebugB != nullptr)
					//	(*desc.notifyDebugB)(movedPtr, emptyPtr, i);
				}
			}
			FreeUnuseMemory();
			DeleteAllInfoPointer(&allocableBlockHead);

			const uint newallocatedBlockCount = GetAllocatedBlockCount();
			for (int64 i = newallocatedBlockCount - 1; i >= useBlockCount; --i)
			{
				ZeroMemory(CalPtrLocation(i), allocBlockSize);
				BlockInfo* newBlockInfo = new BlockInfo(GetPageIndex(i), i);
				newBlockInfo->next = allocableBlockHead;
				allocableBlockHead = newBlockInfo;
			} 
			return true;
		}
		void JVirtualAlloc::FreeUnuseMemory()
		{
			for (int64 i = lastAllocPageIndex; i >= 0; --i)
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
							info = &((*info)->next);
					}
					apiInterface->DeAllocate((PVOID)((std::intptr_t)pData + CalPageSize(i)), pageSize);
					//VirtualFree((PVOID)((std::intptr_t)pData + CalPageSize(i)), pageSize, MEM_DECOMMIT);
					UnUsePage(i);
				}
			}
		}
		bool JVirtualAlloc::Extend()
		{
			DataPointer exPData;
			size_t exAllocSize = totalAllocSize * desc.extendFactor;

			ReserveInfo info;
			info.size = exAllocSize;
			exPData = (DataPointer)apiInterface->ReserveVirtualMemory(&info);
			if (exPData == NULL)
				return false;

			if (apiInterface->Allocate(exPData, committedPageCount * pageSize) == NULL)
			{
				apiInterface->ReleaseVirtualMemory(exPData);
				return false;
			}
 
			for (uint i = 0; i < useBlockCount; ++i)
			{
				DataPointer oldBlockSt = &pData[i * allocBlockSize];
				DataPointer newBlockSt = &exPData[i * allocBlockSize];
				memcpy(newBlockSt, oldBlockSt, allocBlockSize);

				//if (desc.notifyDebugB != nullptr)
				//	(*desc.notifyDebugB)(oldBlockSt, newBlockSt, i);
				if (desc.notifyReAllocB != nullptr)
					(*desc.notifyReAllocB)(newBlockSt, i);
			}
			apiInterface->ReleaseVirtualMemory(pData);

			const size_t preReservePageCount = reservedPageCount;
			const size_t preReserveBlockCount = reservedBlockCount;

			pData = exPData;
			totalAllocSize = exAllocSize;
			reservedPageCount = totalAllocSize / pageSize;
			reservedBlockCount = (reservedPageCount * pageSize) / allocBlockSize;

			delete isUsePage;
			isUsePage = new bool[reservedPageCount]();
			memset(isUsePage, 1, preReservePageCount);

			delete isUseBlock;
			isUseBlock = new bool[reservedBlockCount]();
			memset(isUseBlock, 1, preReserveBlockCount);

			allocablePaegHead = new PageInfo();
			allocablePaegHead->pageIndex = preReservePageCount;
			return true;
		}
	};
}
