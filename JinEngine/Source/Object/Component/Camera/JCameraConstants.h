#pragma once
#include"../../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Constants
	{
		static constexpr float minCamFrustumNear = 1.0f;
		static constexpr float maxCamFrustumFar = 10000.0f;
		static constexpr float defaultCamFrustumNear = minCamFrustumNear;
		static constexpr float defaultCamFrustumFar = maxCamFrustumFar * 0.05f;
		static constexpr float defaultCamFovDeg = 60;	//degree
	}
}