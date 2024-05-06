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