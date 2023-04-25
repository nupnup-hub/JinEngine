#pragma once

namespace JinEngine
{
	namespace Core
	{
		class JMemoryCapture
		{
		public:
			struct TotalMemory
			{
			public:
				size_t totalPhys;
				size_t availPhys;
				size_t totalVirtual;
				size_t availVirtual;
				size_t totalPage;
				size_t availPage;
			public:
				bool canUsePage;
			};
			struct ProcessMemory
			{
			public:
				size_t peakWorkingSetSize;
				size_t workingSetSize;
				size_t privateUsage;
			};
		public:
			static TotalMemory GetTotalMemory()noexcept;
			static ProcessMemory GetCurrentProcessMemory()noexcept;
		};
	}
}