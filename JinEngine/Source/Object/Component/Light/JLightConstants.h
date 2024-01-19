#pragma once

namespace JinEngine
{
	namespace Constants
	{
		static constexpr int maxScenePerDircitonalLight = 4;

		static constexpr float globarLightMinDistance = 0.1f;
		static constexpr float globarLightMaxDistance = 2048;
		static constexpr float globarLightMaxDistanceHalf = globarLightMaxDistance * 0.5f;

		static constexpr float localLightMinDistance = 0.1f;
		static constexpr float localLightMaxDistance = 512;
		static constexpr float localLightMaxDistanceHalf = localLightMaxDistance * 0.5f;

		//for calculate perspective proj
		//static constexpr float lightNear = 1.0f;
		//static constexpr float lightNear = 1.0f;
		//static constexpr float lightMaxFar = lightMaxDistance;
		//static constexpr float lightMaxFarHalf = lightMaxFar * 0.5f;
		 
		static constexpr size_t minShadowMapSize = 256;
		static constexpr size_t maxShdowMapSize = 4096;

		static constexpr uint localLightCount = 3;	//point, spot, rect
	}
}