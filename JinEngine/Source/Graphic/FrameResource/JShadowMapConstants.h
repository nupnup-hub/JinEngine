#pragma once
#include"../ShadowMap/JCsmOption.h" 
#include"../../Core/Math/JMatrix.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JShadowMapArrayDrawConstants
		{
			JMatrix4x4 shadowMapTransform[JCsmOption::maxCountOfSplit];
		};
		struct JShadowMapCubeDrawConstants
		{
			JMatrix4x4 shadowMapTransform[6];
		};
		struct JShadowMapDrawConstants
		{
			JMatrix4x4 shadowMapTransform;
		};
	}
}