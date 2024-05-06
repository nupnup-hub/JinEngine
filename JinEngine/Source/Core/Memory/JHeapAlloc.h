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
#include<Windows.h>  

namespace JinEngine
{
	namespace Core
	{ 
		//Unuse
		class JHeapAlloc : public JAllocationInterface
		{
		private:
			using DataPointer = BYTE*; 
		private:
			HANDLE heapHandle; 
			JAllocationDesc desc; 
		private:
			size_t committedBlockCount = 0; 
			size_t reservedBlockCount = 0;
		public:
			bool Initialize(JAllocationDesc&& newDesc)final;
			void* Allocate(const size_t size)final;
			void DeAllocate(void* p)final;
			void DeAllocate(void* p, const size_t size) final;
			void ReleaseUnusePage()final;
			void Release()final;
		public:
			bool CanAllocate(const uint blockCount)const noexcept final; 
		public:
			size_t GetAlignedAllocBlockSize()const noexcept final;
			JAllocationInfo GetInformation()const noexcept final;
		};
	}
}
 