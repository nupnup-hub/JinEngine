#pragma once 
#include"../../Core/Math/JVector.h"  

namespace JinEngine
{
	namespace Graphic
	{
		class JSampler
		{  
		public:
			static std::vector<JVector3F> CreateHemisphereSample(const uint maxSampleCount, float cosDensityPower); 
		};
	}
}