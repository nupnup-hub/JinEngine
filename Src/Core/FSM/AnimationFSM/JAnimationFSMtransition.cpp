#include"JAnimationFSMtransition.h"
#include"../JFSMLoadGuidMap.h"
#include"../JFSMconditionStorageAccess.h"
#include<fstream>

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
		J_FILE_IO_RESULT JAnimationFSMtransition::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	 
			stream << GetOutputStateGuid() << '\n';
			const uint conditionCount = GetConditioCount();
			stream << conditionCount << '\n';

			for (uint i = 0; i < conditionCount; ++i)
			{
				stream << GetConditionByIndex(i)->GetGuid() << '\n';
				stream << GetConditionOnValue(i) << '\n';
			}

			stream << isWaitExitTime << '\n';
			stream << isFrozen << '\n';
			stream << exitGameTimerate << '\n';
			stream << durationTime << '\n';
			stream << targetStateOffset << '\n';

			return J_FILE_IO_RESULT::SUCCESS;
		}
		std::unique_ptr<JAnimationFSMtransition> JAnimationFSMtransition::LoadData(std::wifstream& stream, JFSMLoadGuidMap& guidMap, IJFSMconditionStorageUser& IConditionUser)
		{
			if (!stream.is_open())
				return nullptr;
 
			size_t outputGuid = 0;
			stream >> outputGuid;

			std::unique_ptr<JAnimationFSMtransition> newTransition = nullptr;
			if (guidMap.isNewGuid)
			{
				auto newStateGuid = guidMap.state.find(outputGuid);
				if (newStateGuid != guidMap.state.end())
					newTransition = std::make_unique<JAnimationFSMtransition>(guidMap.state.find(outputGuid)->second);
			}
			else
				newTransition = std::make_unique<JAnimationFSMtransition>(outputGuid);

			uint conditionCount = 0;
			stream >> conditionCount;

			for (uint i = 0; i < conditionCount; ++i)
			{
				size_t guid;
				float onValue;

				stream >> guid;
				stream >> onValue;

				JFSMcondition* newCondition = nullptr;		
				if (guidMap.isNewGuid)
				{
					auto newGuid = guidMap.condition.find(guid);
					if (newGuid == guidMap.condition.end())
						continue;

					newCondition = IConditionUser.GetCondition(newGuid->second);
				}
				else
					newCondition = IConditionUser.GetCondition(guid);

				JFSMconditionWrap* newConditionWrap = newTransition->AddCondition(newCondition);
				newConditionWrap->SetOnValue(onValue);
			}

			bool isWaitExitTime = false;
			bool isFrozen = false;
			float exitGameTimerate = 0;
			float durationTime =0;
			float targetStateOffset = 0;

			stream >> isWaitExitTime;
			stream >> isFrozen;
			stream >> exitGameTimerate;
			stream >> durationTime;
			stream >> targetStateOffset;

			newTransition->SetIsWaitExitTime(isWaitExitTime);
			newTransition->SetIsFrozen(isFrozen);
			newTransition->SetExitGameTimerate(exitGameTimerate);
			newTransition->SetDurationTime(durationTime);
			newTransition->SetTargetStateOffset(targetStateOffset);

			return newTransition;
		}
		void JAnimationFSMtransition::Initialize()noexcept
		{
			JFSMtransition::Initialize();
		}
	}
}