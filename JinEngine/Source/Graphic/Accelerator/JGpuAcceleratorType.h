#pragma once
#include"../../Core/Reflection/JReflection.h"
#include"../../Core/Math/JMatrix.h" 

namespace JinEngine
{
	class JGameObject; 
	namespace Graphic
	{
		enum J_GPU_ACCELERATOR_BUILD_OPTION
		{
			J_GPU_ACCELERATOR_BUILD_OPTION_NONE = 0,
			J_GPU_ACCELERATOR_BUILD_OPTION_STATIC = 1 << 0,	
			J_GPU_ACCELERATOR_BUILD_OPTION_OPAQUE = 1 << 1,
			J_GPU_ACCELERATOR_BUILD_OPTION_LIGHT_SHAPE = 1 << 2
		}; 
		using _J_GPU_ACCELERATOR_BUILD_OPTION = int;

		struct JGpuAcceleratorBuildDesc
		{
		public:
			std::vector<JUserPtr<JGameObject>> obj;
			std::vector<JUserPtr<JGameObject>> localLight;
			J_GPU_ACCELERATOR_BUILD_OPTION flag = J_GPU_ACCELERATOR_BUILD_OPTION_NONE;
		};		 
	}
}