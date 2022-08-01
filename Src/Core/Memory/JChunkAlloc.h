#pragma once

namespace JinEngine
{
    namespace Core
    {
		//수정필요
		class JChunkAlloc
		{
		public:
			/// Pointer to array of allocated blocks.
			unsigned char* pData;
			/// Index of first empty block.
			unsigned char firstAvailableBlock;
			/// Count of empty blocks.
			unsigned char blocksAvailable;
		public:
			bool Init(size_t blockSize, unsigned char blocks);
			void* Allocate(size_t blockSize);
			void Deallocate(void* p, size_t blockSize);
			void Reset(size_t blockSize, unsigned char blocks);
			void Release();
			bool IsCorrupt(unsigned char numBlocks, size_t blockSize, bool checkIndexes) const;
			bool IsBlockAvailable(void* p, unsigned char numBlocks, size_t blockSize) const;

			/// Returns true if block at address P is inside this Chunk.
			inline bool HasBlock(void* p, size_t chunkLength) const
			{
				unsigned char* pc = static_cast<unsigned char*>(p);
				return (pData <= pc) && (pc < pData + chunkLength);
			}
			inline bool HasAvailable(unsigned char numBlocks) const
			{
				return (blocksAvailable == numBlocks);
			}
			inline bool IsFilled(void) const
			{
				return (0 == blocksAvailable);
			}
		};
    }
}
 