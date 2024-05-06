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


#include"JHardwareInfo.h"
#include"JPlatformInfo.h"
#include "cuda_runtime.h"
#include"../Exception/JExceptionMacro.h" 
#include <windows.h> 

namespace JinEngine
{
	namespace Core
	{ 
		JHardwareInfo::CpuInfo JHardwareInfo::GetCpuInfo()
		{ 
			JHardwareInfo::CpuInfo cpuInfo;
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
			return cpuInfo;
		}
		std::vector<JHardwareInfo::GpuInfo> JHardwareInfo::GetGpuInfo()
		{ 
			std::vector<JHardwareInfo::GpuInfo> gpuInfo;
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
				gpuInfo[i].memoryClockRate = prop.memoryClockRate;
				gpuInfo[i].memoryBusWidth = prop.memoryBusWidth / 8;
				   
				gpuInfo[i].multiProcessorCount = prop.multiProcessorCount;
				gpuInfo[i].sharedMemPerBlock = prop.sharedMemPerBlock;
				gpuInfo[i].registersPerBlock = prop.regsPerBlock;
				gpuInfo[i].registersPerMultiProcessor = prop.regsPerMultiprocessor;
				gpuInfo[i].threadWarpSize = prop.warpSize;
				gpuInfo[i].maxThreadsPerBlock = prop.maxThreadsPerBlock;
				gpuInfo[i].maxBlocksPerMultiProcessor = prop.maxBlocksPerMultiProcessor;
				gpuInfo[i].maxThreadsDim = JVector3<int>(prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2]);
				gpuInfo[i].maxGridDim = JVector3<int>(prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2]);
			} 
			return gpuInfo;
		}
	}
}