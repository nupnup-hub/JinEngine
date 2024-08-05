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

namespace JinEngine::Core
{
	bool JChunkAlloc::Initialize(BYTE* newData, BYTE blocks, const size_t blockSize)
	{
		pData = newData;
		allocableBlockCount = blocks;
		allocBlockSize = blockSize;
		firstAvailableBlock = 0; 
		return true;
	}
	void JChunkAlloc::Clear()
	{
		delete[] pData;
		firstAvailableBlock = allocableBlockCount = firstAvailableBlock =0;
		pData = nullptr;
	}
	void* JChunkAlloc::Allocate(size_t blockSize)
	{
		if (firstAvailableBlock == allocableBlockCount)
			return nullptr;
		 
		BYTE* result = &pData[firstAvailableBlock];
		++firstAvailableBlock; 

		return result;
	}
	void JChunkAlloc::Deallocate(void* p, size_t blockSize)
	{
		uint blockIndex = GetBlockIndex(p);
		firstAvailableBlock = blockIndex; 

		memset(p, 0, blockSize);
	} 
	uint JChunkAlloc::GetBlockIndex(void* p)const noexcept
	{
		return ((std::intptr_t)p - (std::intptr_t)pData) / allocBlockSize;
	}
	bool JChunkAlloc::CanAllocate(const uint blockCount)const noexcept
	{
		return allocableBlockCount;
	}
}

