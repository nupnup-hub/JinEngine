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