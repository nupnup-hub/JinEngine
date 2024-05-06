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


#include"JMemoryCapture.h"
#include"../Platform/JPlatformInfo.h"
#include <windows.h>
#include <psapi.h>

namespace JinEngine
{
	namespace Core
	{
		//수정필요!
		//다른 플랫폼 코드 추가 필요
		JMemoryCapture::TotalMemory JMemoryCapture::GetTotalMemory()noexcept
		{
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&memInfo);

			TotalMemory result;
			result.totalPhys = memInfo.ullTotalPhys;
			result.availPhys = memInfo.ullAvailPhys;
			result.totalVirtual = memInfo.ullTotalVirtual;
			result.availVirtual = memInfo.ullAvailVirtual;
			result.totalPage = memInfo.ullTotalPageFile;
			result.availPage = memInfo.ullAvailPageFile;
			result.canUsePage = true;
			return result;
		}
		JMemoryCapture::ProcessMemory JMemoryCapture::GetCurrentProcessMemory()noexcept
		{
			PROCESS_MEMORY_COUNTERS_EX pmc;
			GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

			ProcessMemory result;
			result.peakWorkingSetSize = pmc.PeakWorkingSetSize;
			result.workingSetSize = pmc.WorkingSetSize;
			result.privateUsage = pmc.PrivateUsage;
			return result;
		}
	}
}