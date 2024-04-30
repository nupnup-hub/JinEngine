#pragma once
#include"../../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{
			static constexpr uint ssaoMaxSampleCount = 32;
			static constexpr uint ssaoSliceCount = 16;
			static constexpr uint ssaoSplitCount = 4;		//used by hbao occlusion calculation
		}
	}
}