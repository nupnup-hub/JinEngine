#pragma once
#include"../JDataType.h"
#include"../Singleton/JSingletonHolder.h"
#include<memory>

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
		class JHardwareInfoImpl
		{
			template<typename T> friend class JCreateUsingNew;
		private:
			J_ARCHITECTURE architecture;
			uint processCount;
			size_t allocationGranularity;
			size_t pageSize;			 
		public: 
			J_ARCHITECTURE GetArchitectureType()noexcept;
			uint GetProcessCount()noexcept;
			size_t GetAllocationGranularity()noexcept;
			size_t GetPageSize()noexcept;
		private: 
			JHardwareInfoImpl();
			void Initialize();
		};
		using JHardwareInfo = Core::JSingletonHolder<JHardwareInfoImpl>;
	}
}