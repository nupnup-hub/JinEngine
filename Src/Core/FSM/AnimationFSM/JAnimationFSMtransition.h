#pragma once
#include"../JFSMtransition.h" 

namespace JinEngine
{
	namespace Core
	{
		class JAnimationFSMtransition : public JFSMtransition
		{
		private:
			bool isWaitExitTime = true;
			bool isFrozen = false;
			float exitGameTimerate = 0.8f;
			float durationTime = 0.4f;
			float targetStateOffset = 0;
		public:
			JAnimationFSMtransition(const size_t outputStateGuid);
			bool GetIsWaitExitTime()noexcept;
			bool GetIsFrozen()noexcept;
			float GetExitGameTimerate()noexcept;
			float GetDurationTime()noexcept;
			float GetTargetStateOffset()noexcept;

			void SetIsWaitExitTime(bool value)noexcept;
			void SetIsFrozen(bool value)noexcept;
			void SetExitGameTimerate(bool value)noexcept;
			void SetDurationTime(bool value)noexcept;
			void SetTargetStateOffset(bool value)noexcept;

			bool IsSatisfiedOption(const float normalizedTime)noexcept;
			void Initialize()noexcept override;
		};
	}
}