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
#include"../Core/JCoreEssential.h"
#include"../Core/Math/JVector.h"
#include"../Core/Unit/JManagedVariable.h"

#define _DEBUG
#ifdef _DEBUG
#define GRAPIC_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

namespace JinEngine
{
	namespace Graphic
	{
		using GraphicFence = uint64;
		//data type경우 값을 void*로 캐스팅가능(비권장) void* 주소값에 값이채워짐
		//사용자지정 데이터타입(클래스, 구조체)경우는 주소값만 전달해야함
		using ResourceHandle = void*;		 

		namespace Constants
		{
			static constexpr uint gNumFrameResources = 3;
			static constexpr uint gMaxFrameThread = 8;
			static constexpr uint stencilclearValue = 0;
			static constexpr uint commonStencilRef = 1;
			static constexpr uint outlineStencilRef= 2;
			static constexpr uint outlineStencilRange = 7;
			static constexpr uint cubeMapPlaneCount = 6; 
			 
			static constexpr Core::JRangeVarSetting<uint, 3, 1, 4> restirBounceRange;
			static constexpr Core::JRangeVarSetting<uint, 32, 1, 1024> restirTemporalSampleRange;
			static constexpr Core::JRangeVarSetting<uint, 128, 1, 4096> restirSpatialSampleRange;
			static constexpr Core::JRangeVarSetting<uint, 128, 1, 1024> restirSampleAgeRange;
			static constexpr Core::JRangeVarSetting<uint, 10, 1, 1024> neighborWeightSumThresholdRange;
			static constexpr Core::JRangeVarSetting<uint, 10, 1, 32> spatialWeightAverageThresholdRange;
 
			static JVector4F BackBufferClearColor()noexcept
			{
				return  JVector4F(0.45490196f, 0.54117647f, 0.63137255f, 1.0f);
			} 
			static JVector4F BlackColor()noexcept
			{
				return JVector4F(0, 0, 0, 0);
			} 
			static JVector4F WhileColor()noexcept
			{
				return JVector4F(1.0f, 1.0f, 1.0f, 1.0f);
			} 
		}
	}
}