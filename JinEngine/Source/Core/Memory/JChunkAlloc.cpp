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
