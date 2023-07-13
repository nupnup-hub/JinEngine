#pragma once
#include"../JDataType.h" 
#include"../../Utility/JVector.h"
#include<memory>
#include<vector>

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
				// --- MP Information for device %d ---\n
				size_t sharedMemPerBlock;
				int multiProcessorCount;
				int registersPerBlock;
				int threadWarpSize;
				int maxThreadsPerBlock;
				int maxBlocksPerMultiProcessor;
				JVector3<int> maxThreadsDim;
				JVector3<int> maxGridDim;		 
			};  
		public: 
			static CpuInfo GetCpuInfo()noexcept;
			static std::vector<GpuInfo> GetGpuInfo()noexcept;
		}; 
	}
}