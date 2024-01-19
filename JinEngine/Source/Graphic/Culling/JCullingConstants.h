#pragma once 
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{
			static constexpr float cullingUpdateFrequencyMin = 0.0f;
			static constexpr float cullingUpdateFrequencyMax = 1.0f;
			 
			static constexpr uint frustumNonCullingValue = 0;
			static constexpr uint hzbNonCullingValue = 0;
			static constexpr uint hdNonCullingValue = 1;
			 
			static constexpr uint lightPerClusterVariation = 3;
			static constexpr uint litClusterXVariation = 2;
			static constexpr uint litClusterYVariation = 2;
			static constexpr uint litClusterZVariation = 3;
			static constexpr uint litClusterVariation = litClusterXVariation * litClusterYVariation * litClusterZVariation;

			//0 is fixed value
			static constexpr uint litClusterXRange[litClusterXVariation] = { 24, 48 };
			static constexpr uint litClusterYRange[litClusterYVariation] = { 12, 24 };
			static constexpr uint litClusterZRange[litClusterZVariation] = { 32, 64, 128 };
			static constexpr uint maxLightPerClusterRange[lightPerClusterVariation] = { 15, 30, 45 };
			static constexpr float litClusterNear = 50.0f;
			static constexpr float litClusterNearMin = 1.0f;
			static constexpr float litClusterNearMax = 256.0f;

			static constexpr float litClusterRangeOffset = 1.1f;
			static constexpr float litClusterRangeOffsetMin = 0.75f;
			static constexpr float litClusterRangeOffsetMax = 2.0;
		}
	}
}