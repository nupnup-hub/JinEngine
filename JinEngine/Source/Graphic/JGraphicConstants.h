#include"../Core/JCoreEssential.h"
#include"../Core/Math/JVector.h"
#pragma once 
#define GRAPIC_DEBUG 
#if defined(GRAPIC_DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

namespace JinEngine
{
	namespace Graphic
	{
		using GraphicFence = uint64;
		//data type경우 값을 void*로 캐스팅가능(비권장) void* 주소값에 값이채워짐
		//그외에 사용자지정 데이터타입(클래스, 구조체)경우는 주소값만 전달해야함
		using ResourceHandle = void*;		 

		namespace Constants
		{
			static constexpr int gNumFrameResources = 3;
			static constexpr int gMaxFrameThread = 8;
			static constexpr int commonStencilRef = 1;
			static const JVector4F backBufferClearColor = JVector4F(0.45490196f, 0.54117647f, 0.63137255f, 1.0f);
		}
	}
}