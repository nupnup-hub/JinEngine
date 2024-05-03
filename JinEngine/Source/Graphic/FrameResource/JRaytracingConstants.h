#pragma once
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{ 
		template<uint sampleRayCount>
		struct JRaytracingDebugConstants
		{ 
			JVector4F rayOriginAndMinT[sampleRayCount];
			JVector4F rayDirAndMaxT[sampleRayCount];
			JVector4F intersectPosition[sampleRayCount];				//x, y, z = position , w = result 0, 1
			JVector2<uint> index;
		};
	}
}