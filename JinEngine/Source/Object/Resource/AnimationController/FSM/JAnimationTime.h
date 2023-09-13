#pragma once

namespace JinEngine
{
	struct JAnimationTime
	{
	public:
		float timePos = 0;
		float startTime = 0;
		float endTime = 0;
		float normalizedTime = 0;
		float nextUpdateTime = 0;
	public:
		JAnimationTime() = default;
		~JAnimationTime() = default;
	public:
		void Clear();
		bool IsOverEndTime()const noexcept;
	};
}