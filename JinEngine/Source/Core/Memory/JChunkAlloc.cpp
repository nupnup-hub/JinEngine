#include"JChunkAlloc.h"

namespace JinEngine
{
	namespace Core
	{
		bool JChunkAlloc::Init(size_t blockSize, unsigned char blocks)
		{
			blocksAvailable = blocks;
			return true;
		}
		void* JChunkAlloc::Allocate(size_t blockSize)
		{
			return pData;
		}
		void JChunkAlloc::Deallocate(void* p, size_t blockSize)
		{

		}
		void JChunkAlloc::Reset(size_t blockSize, unsigned char blocks)
		{

		}
		void JChunkAlloc::Release()
		{

		}
		bool JChunkAlloc::IsCorrupt(unsigned char numBlocks, size_t blockSize, bool checkIndexes) const
		{
			return true;
		}
		bool JChunkAlloc::IsBlockAvailable(void* p, unsigned char numBlocks, size_t blockSize) const
		{
			return true;
		}
	}
}
