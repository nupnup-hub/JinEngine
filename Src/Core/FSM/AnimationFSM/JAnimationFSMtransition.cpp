#include"JAnimationFSMtransition.h"

namespace JinEngine
{
	namespace Core
	{
		JAnimationFSMtransition::JAnimationFSMtransition(const size_t outputStateGuid)
			:JFSMtransition(outputStateGuid)
		{}
		bool JAnimationFSMtransition::GetIsWaitExitTime()noexcept
		{
			return isWaitExitTime;
		}
		bool JAnimationFSMtransition::GetIsFrozen()noexcept
		{
			return isFrozen;
		}
		float JAnimationFSMtransition::GetExitGameTimerate()noexcept
		{
			return exitGameTimerate;
		}
		float JAnimationFSMtransition::GetDurationTime()noexcept
		{
			return durationTime;
		}
		float JAnimationFSMtransition::GetTargetStateOffset()noexcept
		{
			return targetStateOffset;
		}

		void JAnimationFSMtransition::SetIsWaitExitTime(bool value)noexcept
		{
			isWaitExitTime = value;
		}
		void JAnimationFSMtransition::SetIsFrozen(bool value)noexcept
		{
			isFrozen = value;
		}
		void JAnimationFSMtransition::SetExitGameTimerate(bool value)noexcept
		{
			exitGameTimerate = value;
		}
		void JAnimationFSMtransition::SetDurationTime(bool value)noexcept
		{
			durationTime = value;
		}
		void JAnimationFSMtransition::SetTargetStateOffset(bool value)noexcept
		{
			targetStateOffset = value;
		}
		bool JAnimationFSMtransition::IsSatisfiedOption(const float normalizedTime)noexcept
		{
			if (isWaitExitTime)
			{
				if (normalizedTime >= exitGameTimerate)
					return true;
				else
					return false;
			}
			else
				return true;

		}
		void JAnimationFSMtransition::Initialize()noexcept
		{
			JFSMtransition::Initialize();
		}
	}
}