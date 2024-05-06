/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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
 