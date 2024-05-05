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
		//data type��� ���� void*�� ĳ���ð���(�����) void* �ּҰ��� ����ä����
		//��������� ������Ÿ��(Ŭ����, ����ü)���� �ּҰ��� �����ؾ���
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