#pragma once

namespace JinEngine
{
	namespace Constants
	{
		static constexpr int maxScenePerDircitonalLight = 4;

		static constexpr float lightMinDistance = 0.1f;
		static constexpr float lightMaxDistance = 10000;
		static constexpr float lightMaxDistanceHalf = lightMaxDistance * 0.5f;

		//for calculate perspective proj
		//static constexpr float lightNear = 1.0f;
		static constexpr float lightNear = 1.0f;
		static constexpr float lightMaxFar = lightMaxDistance;
		static constexpr float lightMaxFarHalf = lightMaxFar * 0.5f;

		static constexpr size_t minShadowMapSize = 256;
		static constexpr size_t maxShdowMapSize = 4096;
	}
}