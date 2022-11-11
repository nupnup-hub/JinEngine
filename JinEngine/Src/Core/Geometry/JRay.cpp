#include"JRay.h"

namespace JinEngine
{
	namespace Core
	{ 
		JRay::JRay(const JVector3<float>& p, const JVector3<float>& dir, float time, float maxTime)
			:p(p), dir(dir), time(time), maxTime(maxTime)
		{}
	}
}