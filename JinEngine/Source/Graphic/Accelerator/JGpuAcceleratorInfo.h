#pragma once  
#include"JGpuAcceleratorType.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Reflection/JReflection.h"
#include<memory>

namespace JinEngine
{
	namespace Graphic
	{
		class JGpuAcceleratorHolder;
		class JGpuAcceleratorManager;
		class JGpuAcceleratorInfo final
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGpuAcceleratorInfo) 
		private:
			friend class JGpuAcceleratorManager; 
		private:
			JGpuAcceleratorManager* am = nullptr;
		private:
			std::unique_ptr<JGpuAcceleratorHolder> holder = nullptr;
		private:
			J_GPU_ACCELERATOR_BUILD_OPTION buildOptionFlag; 
			int arrayIndex = invalidIndex;
		public:
			int GetArrayIndex()const noexcept;
			J_GPU_ACCELERATOR_BUILD_OPTION GetBuildOption()const noexcept;
		public:
			void SetArrayIndex(const int newValue)noexcept;  
		private:
			JGpuAcceleratorInfo(JGpuAcceleratorManager* manager,
				const J_GPU_ACCELERATOR_BUILD_OPTION buildOptionFlag,
				std::unique_ptr<JGpuAcceleratorHolder>&& holder);
			~JGpuAcceleratorInfo();
		};
	}
}