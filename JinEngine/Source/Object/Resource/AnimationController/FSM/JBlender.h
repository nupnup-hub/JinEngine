#pragma once

namespace JinEngine
{
	struct JBlender
	{
	private:
		float startTime;
		float endTime;
		static constexpr float maxValue = 0.999f;
	private:
		bool isActivated = false;
	public:
		void Initialize(const float startTime, const float endTime)noexcept;
		void Clear()noexcept;
	public:
		float GetBlnederValue(const float nowTime)const noexcept;
		bool IsBlenderEnd(const float nowTime)const noexcept;
		bool IsActivated()const noexcept;
	};
}