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
		//data type��� ���� void*�� ĳ���ð���(�����) void* �ּҰ��� ����ä����
		//�׿ܿ� ��������� ������Ÿ��(Ŭ����, ����ü)���� �ּҰ��� �����ؾ���
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