#pragma once

namespace JinEngine
{
	namespace Core
	{
		struct JBlender
		{
		private:
			float startTime;
			float endTime;
			static constexpr float maxValue = 0.999f;
		public:
			void Initialize(const float startTime, const float endTime)noexcept;
			float GetBlnederValue(const float nowTime)noexcept;
			bool IsBlenderEnd(const float nowTime)noexcept;
		};
	}
}