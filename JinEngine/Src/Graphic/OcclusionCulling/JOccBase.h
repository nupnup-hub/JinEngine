#pragma once
namespace JinEngine
{
	namespace Graphic
	{
		class JOccBase
		{
		private:
			float updateFrequency = 0.1f;
			float lastUpdateTime = 0;
		private:
			bool canReadCullingResult = false;
			bool canOcclusionCullingStart = false;
		public:
			virtual ~JOccBase() = default;
		public:
			void UpdateTimer()noexcept;
		public:
			bool CanReadOcclusionResult()const noexcept;
			bool CanCullingStart()const noexcept;
		public:
			void SetUpdateFrequency(const float newUpdateFrequency)noexcept;
		protected:
			void SetReadResultTrigger(bool value)noexcept;
		};
	}
}