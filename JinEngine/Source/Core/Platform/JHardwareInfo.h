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
#include"../Math/JVector.h"

namespace JinEngine
{
	namespace Core
	{
		enum class J_ARCHITECTURE
		{
			AMD64,
			ARM,
			ARM64,
			IA64,
			INTEL,
			UNKNOWN
		};
		enum class J_GRAPHIC_VENDOR
		{
			NVIDIA,
			AMD
		};
		class JHardwareInfo
		{
		public:
			struct CpuInfo
			{
			public:
				J_ARCHITECTURE architecture;
				uint processCount;
				size_t allocationGranularity;
				size_t pageSize;
			public:
				CpuInfo() = default;
			};
			struct GpuInfo
			{
			public:
				J_GRAPHIC_VENDOR vendor;
				//--- General Information for device
				std::string name;
				int majorComputeCapability;
				int minorComputeCapability;
				int clockRate;
				bool canDeviceOverlap;
				bool canKernelExecTimeout;
				//--- Memory Information for device
				size_t totalGlobalMem;
				size_t totalConstMem;
				size_t memPitch;
				size_t textureAlignment;
				int memoryClockRate;
				int memoryBusWidth;			//byte
				// --- MP Information for device %d ---\n
				size_t sharedMemPerBlock;
				int multiProcessorCount;
				int registersPerBlock;
				int registersPerMultiProcessor;
				int threadWarpSize;
				int maxThreadsPerBlock;
				int maxBlocksPerMultiProcessor;
				JVector3<int> maxThreadsDim;
				JVector3<int> maxGridDim;	 
			};  
		public: 
			static CpuInfo GetCpuInfo();
			static std::vector<GpuInfo> GetGpuInfo();
		}; 
	}
}