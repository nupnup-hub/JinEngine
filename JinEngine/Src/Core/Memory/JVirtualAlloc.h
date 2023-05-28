#pragma once 
#include"JAllocationInterface.h" 

namespace JinEngine
{
	namespace Core
	{ 		
		//수정필요
		//현재 Virtual에 경우 할당량을 초과하면 새로운 Heap주소 할당하고 가상메모리 예약량을 늘리는
		//방향으로 개선해야함
		//...가비지 콜렉터 추가후 구현예정

		class JVirtualAllocApiInterface;
		class JVirtualAlloc : public JAllocationInterface
		{ 
		private:
			using DataPointer = BYTE*;
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
				BlockInfo(const uint pageIndex, const uint blockIndex);
			};
		private:
			JVirtualAllocApiInterface* apiInterface;
		private:
			/// Pointer to array of allocated blocks.
			DataPointer pData = nullptr;
			bool* isUsePage = nullptr;
			bool* isUseBlock = nullptr;
		private:
			PageInfo* allocablePaegHead = nullptr;
			BlockInfo* allocableBlockHead = nullptr;
		private:
			size_t oriBlockSize = 0;
			size_t allocBlockSize = 0;
			size_t pageSize = 0;
			size_t totalAllocSize = 0;
			uint committedPageCount = 0; 
			uint reservedBlockCount = 0;
			uint reservedPageCount = 0;
		private:
			uint useBlockCount = 0;  
		private:
			uint lastAllocPageIndex = 0;	//store last page index  caution! it isn't store realloc page index
		private:
			JAllocationDesc desc;
		public:
			JVirtualAlloc();
			~JVirtualAlloc();
		public:
			bool Initialize(JAllocationDesc&& newDesc)final;
			void* Allocate(const size_t reqSize)final;
			void DeAllocate(void* p)final;
			void DeAllocate(void* p, const size_t size)final;
			void DeallocateAll();
			void ReleaseUnusePage()final;
			void Release()final;
		public:
			bool CanAllocate(const uint blockCount)const noexcept final; 
		private:
			bool CanCompactMemory()const noexcept;
			bool IsOverlapPage(const uint pageIndex)const noexcept;
		private:
			uint GetBlockIndex(void* p)const noexcept;
			uint GetPageIndex(void* p)const noexcept;
			uint GetPageIndex(const uint blockIndex)const noexcept;
			uint GetCommittedBlockCount()const noexcept;
			uint GetAllocableBlockCount()const noexcept; 
			uint GetAllocatedBlockCount()const noexcept;
			uint GetDeAllocatedBlockCount()const noexcept;
			JAllocationInfo GetInformation()const noexcept final;
		private:
			inline DataPointer CalPtrLocation(const uint index)const noexcept
			{
				return &pData[index * allocBlockSize];
			}
			inline size_t CalPageSize(const uint pageIndex)const noexcept
			{
				return pageSize * (size_t)pageIndex;
			}
			inline size_t CalBlockSize(const uint blockIndex)const noexcept
			{
				return allocBlockSize * (size_t)blockIndex;
			} 
			void CalPageInnerBlockIndex(const uint pageIndex, _Out_ uint& st, _Out_ uint& ed)const noexcept;
		private:
			DataPointer UsePage(const uint pageIndex)noexcept;
			void UnUsePage(const uint pageIndex)noexcept;
			DataPointer UseBlock(const uint pageIndex, const uint blockIndex)noexcept;
			void UnUseBlock(const uint pageIndex, const uint blockIndex)noexcept;
			void UnUseBlock(void* p)noexcept;
		private:
			DataPointer CommitEmtpyBlock();
			DataPointer CommitEmptyPage();
		private:
			bool CompactUnuseMemory();
			void FreeUnuseMemory();
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
			bool Extend();
		};
	}
}