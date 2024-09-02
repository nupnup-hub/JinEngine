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
#include"JAllocationInterface.h"

namespace JinEngine
{
    namespace Core
    { 
		//unuse
		class JChunkAlloc
		{
		private:
			/// Pointer to array of allocated blocks.
			BYTE* pData;
			/// Index of first empty block.
			BYTE firstAvailableBlock;
			/// Count of empty blocks.
			BYTE allocableBlockCount;
		private:
			size_t allocBlockSize;
		public:
			bool Initialize(BYTE* newData, BYTE blocks, const size_t blockSize);
			void Clear();
		public:
			void* Allocate(size_t blockSize);
			void Deallocate(void* p, size_t blockSize);
		private:
			uint GetBlockIndex(void* p)const noexcept;
		public: 
			bool CanAllocate(const uint blockCount)const noexcept;
			/// Returns true if block at address P is inside this Chunk.
			inline bool HasBlock(void* p, size_t chunkLength) const
			{
				BYTE* pc = static_cast<BYTE*>(p);
				return (pData <= pc) && (pc < pData + chunkLength);
			}
		};
    }
}
 