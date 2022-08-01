#include"JHardwareInfo.h"
#include"JPlatformInfo.h"
#include <windows.h> 

namespace JinEngine
{
	namespace Core
	{
		JHardwareInfoImpl::JHardwareInfoImpl()
		{
			Initialize();
		}
		J_ARCHITECTURE JHardwareInfoImpl::GetArchitectureType()noexcept
		{
			return architecture;
		}
		uint JHardwareInfoImpl::GetProcessCount()noexcept
		{
			return processCount;
		}
		size_t JHardwareInfoImpl::GetAllocationGranularity()noexcept
		{
			return allocationGranularity;
		}
		size_t JHardwareInfoImpl::GetPageSize()noexcept
		{
			return pageSize;
		}
		void JHardwareInfoImpl::Initialize()
		{
#if defined OS_WINDOW
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			switch (sysInfo.wProcessorArchitecture)
			{
			case PROCESSOR_ARCHITECTURE_AMD64:
				architecture = J_ARCHITECTURE::AMD64;
				break;
			case PROCESSOR_ARCHITECTURE_ARM:
				architecture = J_ARCHITECTURE::ARM;
				break;
			case PROCESSOR_ARCHITECTURE_ARM64:
				architecture = J_ARCHITECTURE::ARM64;
				break;
			case PROCESSOR_ARCHITECTURE_IA64:
				architecture = J_ARCHITECTURE::IA64;
				break;
			case PROCESSOR_ARCHITECTURE_INTEL:
				architecture = J_ARCHITECTURE::INTEL;
				break;
			case PROCESSOR_ARCHITECTURE_UNKNOWN:
				architecture = J_ARCHITECTURE::UNKNOWN;
				break;
			default:
				break;
		}
			processCount = sysInfo.dwNumberOfProcessors;
			allocationGranularity = sysInfo.dwAllocationGranularity;
			pageSize = sysInfo.dwPageSize;
#else
 
#endif
		}
	}
}