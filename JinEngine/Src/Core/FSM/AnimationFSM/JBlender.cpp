#include"JBlender.h"
#include<algorithm>

namespace JinEngine
{
	namespace Core
	{
		void JBlender::Initialize(const float startTime, const float endTime)noexcept
		{
			JBlender::startTime = startTime;
			JBlender::endTime = endTime;
		}
		float JBlender::GetBlnederValue(const float nowTime)noexcept
		{
			float u = (nowTime - startTime) / (endTime - startTime);
			float v = 1 - u;

			float b = (powf(v, 3) + (3 * powf(v, 2) * u)) * 0.001f + (3 * v * (powf(u, 2)) + powf(u, 3)) * 1.0f;
			//return  b < maxValue ? b : 1;
			return b;
		}
		bool JBlender::IsBlenderEnd(const float nowTime)noexcept
		{
			float u = (nowTime - startTime) / (endTime - startTime);
			float v = 1 - u;

			return v <= 0;
		}
	}
}