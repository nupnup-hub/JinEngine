#include"JHardwareInfo.h"
#include"JPlatformInfo.h"
#include "cuda_runtime.h"
#include"../Exception/JExceptionMacro.h"
#include <windows.h> 

namespace JinEngine
{
	namespace Core
	{
		JHardwareInfoImpl::JHardwareInfoImpl()
		{
			LoadCpuInfo();
			LoadGpuInfo();
		}
		JHardwareInfoImpl::CpuInfo JHardwareInfoImpl::GetCpuInfo()const noexcept
		{
			return cpuInfo;
		}
		std::vector<JHardwareInfoImpl::GpuInfo> JHardwareInfoImpl::GetGpuInfo()const noexcept
		{
			return gpuInfo;
		}
		void JHardwareInfoImpl::LoadCpuInfo()
		{
#if defined OS_WINDOW
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			switch (sysInfo.wProcessorArchitecture)
			{
			case PROCESSOR_ARCHITECTURE_AMD64:
				cpuInfo.architecture = J_ARCHITECTURE::AMD64;
				break;
			case PROCESSOR_ARCHITECTURE_ARM:
				cpuInfo.architecture = J_ARCHITECTURE::ARM;
				break;
			case PROCESSOR_ARCHITECTURE_ARM64:
				cpuInfo.architecture = J_ARCHITECTURE::ARM64;
				break;
			case PROCESSOR_ARCHITECTURE_IA64:
				cpuInfo.architecture = J_ARCHITECTURE::IA64;
				break;
			case PROCESSOR_ARCHITECTURE_INTEL:
				cpuInfo.architecture = J_ARCHITECTURE::INTEL;
				break;
			case PROCESSOR_ARCHITECTURE_UNKNOWN:
				cpuInfo.architecture = J_ARCHITECTURE::UNKNOWN;
				break;
			default:
				break;
		} 
			cpuInfo.processCount = sysInfo.dwNumberOfProcessors;
			cpuInfo.allocationGranularity = sysInfo.dwAllocationGranularity;
			cpuInfo.pageSize = sysInfo.dwPageSize;
#else
#endif
		}
		void JHardwareInfoImpl::LoadGpuInfo()
		{
			//수정필요 
			//nvidia만 구현된상태 Amd추가필요
			gpuInfo.clear();
			cudaDeviceProp prop;
			int count;
			ThrowIfFailedC(cudaGetDeviceCount(&count));

			gpuInfo.resize(count);
			for (int i = 0; i < count; i++)
			{ 
				ThrowIfFailedC(cudaGetDeviceProperties(&prop, i));
				gpuInfo[i].vendor = J_GRAPHIC_VENDOR::NVIDIA;
				gpuInfo[i].name = prop.name;
				gpuInfo[i].majorComputeCapability = prop.major;
				gpuInfo[i].minorComputeCapability = prop.minor;
				gpuInfo[i].clockRate = prop.clockRate;
				gpuInfo[i].canDeviceOverlap = prop.deviceOverlap;
				gpuInfo[i].canKernelExecTimeout = prop.kernelExecTimeoutEnabled;

				gpuInfo[i].totalGlobalMem = prop.totalGlobalMem;
				gpuInfo[i].totalConstMem = prop.totalConstMem;
				gpuInfo[i].memPitch = prop.memPitch;
				gpuInfo[i].textureAlignment = prop.textureAlignment;
				 
				gpuInfo[i].multiProcessorCount = prop.multiProcessorCount;
				gpuInfo[i].sharedMemPerBlock = prop.sharedMemPerBlock;
				gpuInfo[i].registersPerBlock = prop.regsPerBlock;
				gpuInfo[i].threadWarpSize = prop.warpSize;
				gpuInfo[i].maxThreadsPerBlock = prop.maxThreadsPerBlock;
				gpuInfo[i].maxBlocksPerMultiProcessor = prop.maxBlocksPerMultiProcessor;
				gpuInfo[i].maxThreadsDim = JVector3<int>(prop.maxThreadsDim[0], prop.maxThreadsDim[1],prop.maxThreadsDim[2]);
				gpuInfo[i].maxGridDim = JVector3<int>(prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2]);
			}
		}
	}
}