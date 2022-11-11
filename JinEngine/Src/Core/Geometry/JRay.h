#pragma once 
#include"../../Utility/JVector.h"

namespace JinEngine
{
	namespace Core
	{
		class JRay
		{ 
		public:
			JVector3<float> p;
			JVector3<float> dir;
			float time;
			float maxTime;
		public: 
			JRay(const JVector3<float>& p, const JVector3<float>& dir, float time = 0, float maxTime = FLT_MAX);
		};
	}
}