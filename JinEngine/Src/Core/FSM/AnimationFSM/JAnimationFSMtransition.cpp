#include"JAnimationFSMtransition.h" 
#include"../JFSMconditionStorageAccess.h"
#include"../../File/JFileConstant.h" 
#include"../../File/JFileIOHelper.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
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
		J_FILE_IO_RESULT JAnimationFSMtransition::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	    
			JFileIOHelper::StoreAtomicData(stream, L"ConditionCount:", GetConditioCount());
			const uint conditionCount = GetConditioCount(); 
			for (uint i = 0; i < conditionCount; ++i)
			{
				JFileIOHelper::StoreAtomicData(stream, JFileConstant::StreamHasObjGuidSymbol(), GetConditionByIndex(i)->GetGuid());
				JFileIOHelper::StoreAtomicData(stream, L"ConditionValue:", GetConditionOnValue(i));
			}
			JFileIOHelper::StoreAtomicData(stream, L"IsWaitExitTime:", isWaitExitTime);
			JFileIOHelper::StoreAtomicData(stream, L"IsFrozen:", isFrozen);
			JFileIOHelper::StoreAtomicData(stream, L"ExitGameTimerate:", exitGameTimerate);
			JFileIOHelper::StoreAtomicData(stream, L"DurationTime:", durationTime);
			JFileIOHelper::StoreAtomicData(stream, L"TargetStateOffset:", targetStateOffset);

			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JAnimationFSMtransition::LoadData(std::wifstream& stream, JFSMconditionStorageUserAccess& IConditionUser)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
  
			uint conditionCount = 0; 
			JFileIOHelper::LoadAtomicData(stream, conditionCount);

			for (uint i = 0; i < conditionCount; ++i)
			{
				size_t conditionGuid;
				float onValue;

				JFileIOHelper::LoadAtomicData(stream, conditionGuid);
				JFileIOHelper::LoadAtomicData(stream, onValue);

				JFSMcondition* newCondition = IConditionUser.GetCondition(conditionGuid);
				if (newCondition != nullptr)
				{
					JFSMconditionWrap* newConditionWrap = AddCondition(newCondition);
					newConditionWrap->SetOnValue(onValue);
				}
			}

			bool isWaitExitTime = false;
			bool isFrozen = false;
			float exitGameTimerate = 0;
			float durationTime =0;
			float targetStateOffset = 0;

			JFileIOHelper::LoadAtomicData(stream, isWaitExitTime);
			JFileIOHelper::LoadAtomicData(stream, isFrozen);
			JFileIOHelper::LoadAtomicData(stream, exitGameTimerate);
			JFileIOHelper::LoadAtomicData(stream, durationTime);
			JFileIOHelper::LoadAtomicData(stream, targetStateOffset);

			SetIsWaitExitTime(isWaitExitTime);
			SetIsFrozen(isFrozen);
			SetExitGameTimerate(exitGameTimerate);
			SetDurationTime(durationTime);
			SetTargetStateOffset(targetStateOffset);

			return J_FILE_IO_RESULT::SUCCESS;
		}
		JAnimationFSMtransition::JAnimationFSMtransition(const size_t outputStateGuid)
			:JFSMtransition(outputStateGuid)
		{}
		JAnimationFSMtransition::~JAnimationFSMtransition(){}
	}
}