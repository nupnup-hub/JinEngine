#pragma once
#include"../ShadowMap/JCsmOption.h"
#include"../JGraphicConstants.h"
#include"../../Core/Math/JMatrix.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JShadowMapDrawConstants
		{
			JMatrix4x4 shadowMapTransform; 
		};
		struct JShadowMapArrayDrawConstants
		{
			JMatrix4x4 shadowMapTransform[JCsmOption::maxCountOfSplit]; 
		};
		struct JShadowMapCubeDrawConstants
		{ 
			JMatrix4x4 shadowMapTransform[Constants::cubeMapPlaneCount]; 
		};
	}
}