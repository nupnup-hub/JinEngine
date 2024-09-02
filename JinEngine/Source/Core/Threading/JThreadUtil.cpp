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

#include"JThreadUtil.h"

namespace JinEngine::Core
{
	void JThreadUtil::DispatchWorkIndex(const uint taskCount, const uint threadCount, const uint threadIndex, _Out_ uint& stIndex, _Out_ uint& edIndex) noexcept
	{
		if (taskCount == 0)
		{
			stIndex = 0;
			edIndex = 0;
			return;
		}
		if (taskCount < threadCount)
		{
			if (threadIndex < taskCount)
			{
				stIndex = threadIndex;
				edIndex = threadIndex + 1;
			}
			else
			{
				stIndex = 0;
				edIndex = 0;
			}
		}
		else
		{
			const uint threadPer = taskCount / threadCount;
			stIndex = threadPer * threadIndex;

			if (threadIndex == threadCount - 1)
				edIndex = taskCount;
			else
				edIndex = threadPer * (threadIndex + 1);
		}
	}
}