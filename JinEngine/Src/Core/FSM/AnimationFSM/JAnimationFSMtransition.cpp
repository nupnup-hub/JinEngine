#include"JAnimationFSMtransition.h" 
#include"../JFSMcondition.h"
#include"../JFSMparameter.h"
#include"../JFSMparameterStorageAccess.h"
#include"../JFSMfactory.h"
#include"../../File/JFileConstant.h" 
#include"../../File/JFileIOHelper.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		bool JAnimationFSMtransition::IsWaitExitTime()noexcept
		{
			return isWaitExitTime;
		}
		bool JAnimationFSMtransition::IsFrozen()noexcept
		{
			return isFrozen;
		}
		float JAnimationFSMtransition::GetExitTimeRate()noexcept
		{
			return exitTimeRate;
		}
		float JAnimationFSMtransition::GetDurationTime()noexcept
		{
			return durationTime;
		}
		float JAnimationFSMtransition::GetTargetStartTimeRate()noexcept
		{
			return targetStartTimeRate;
		}

		void JAnimationFSMtransition::SetIsWaitExitTime(const bool value)noexcept
		{
			isWaitExitTime = value;
		}
		void JAnimationFSMtransition::SetIsFrozen(const bool value)noexcept
		{
			isFrozen = value;
		}
		void JAnimationFSMtransition::SetExitTimeRate(const float value)noexcept
		{
			exitTimeRate = value;
		}
		void JAnimationFSMtransition::SetDurationTime(const float value)noexcept
		{
			durationTime = value;
		}
		void JAnimationFSMtransition::SetTargetStartTimeRate(const float value)noexcept
		{
			targetStartTimeRate = value;
		}
		bool JAnimationFSMtransition::IsSatisfiedOption(const float normalizedTime)noexcept
		{
			if (isWaitExitTime)
			{
				if (normalizedTime >= exitTimeRate)
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
				JFSMcondition* cond = GetConditionByIndex(i);
				JFileIOHelper::StoreFsmObjectIden(stream, cond);
				JFileIOHelper::StoreHasObjectIden(stream, cond->GetParameter(), Core::JFileConstant::StreamUncopiableGuidSymbol());
				JFileIOHelper::StoreAtomicData(stream, L"ConditionValue:", GetConditionOnValue(i));
			}
			JFileIOHelper::StoreAtomicData(stream, L"IsWaitExitTime:", isWaitExitTime);
			JFileIOHelper::StoreAtomicData(stream, L"IsFrozen:", isFrozen);
			JFileIOHelper::StoreAtomicData(stream, L"ExitGameTimerate:", exitTimeRate);
			JFileIOHelper::StoreAtomicData(stream, L"DurationTime:", durationTime);
			JFileIOHelper::StoreAtomicData(stream, L"TargetStateOffset:", targetStartTimeRate);

			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JAnimationFSMtransition::LoadData(std::wifstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
  
			uint conditionCount = 0; 
			JFileIOHelper::LoadAtomicData(stream, conditionCount);

			for (uint i = 0; i < conditionCount; ++i)
			{
				std::wstring condName;
				size_t condGuid;
				J_FSM_OBJECT_TYPE fsmType; 
				float onValue;

				JFileIOHelper::LoadFsmObjectIden(stream, condName, condGuid, fsmType);
				JFSMparameter* param = static_cast<JFSMparameter*>(JFileIOHelper::LoadHasObjectIden(stream));
				JFileIOHelper::LoadAtomicData(stream, onValue);
				 
				JFSMcondition* newCondition = JFFI<JFSMcondition>::Create(Core::JPtrUtil::MakeOwnerPtr<JFSMcondition::InitData>
					(condName, condGuid, Core::GetUserPtr(this)));
				if (param != nullptr)
					newCondition->SetParameter(param);
				newCondition->SetOnValue(onValue);
			}

			bool isWaitExitTime = false;
			bool isFrozen = false;
			float exitTimeRate = 0;
			float durationTime =0;
			float targetStartTimeRate = 0;

			JFileIOHelper::LoadAtomicData(stream, isWaitExitTime);
			JFileIOHelper::LoadAtomicData(stream, isFrozen);
			JFileIOHelper::LoadAtomicData(stream, exitTimeRate);
			JFileIOHelper::LoadAtomicData(stream, durationTime);
			JFileIOHelper::LoadAtomicData(stream, targetStartTimeRate);

			SetIsWaitExitTime(isWaitExitTime);
			SetIsFrozen(isFrozen);
			SetExitTimeRate(exitTimeRate);
			SetDurationTime(durationTime);
			SetTargetStartTimeRate(targetStartTimeRate);

			return J_FILE_IO_RESULT::SUCCESS;
		}
		void JAnimationFSMtransition::RegisterJFunc()
		{
			auto createTransitionLam = [](JOwnerPtr<JFSMIdentifierInitData> initData)-> JFSMInterface*
			{
				if (initData.IsValid() && initData->GetFSMobjType() == J_FSM_OBJECT_TYPE::TRANSITION)
				{
					JFSMtransitionInitData* transitionInitData = static_cast<JFSMtransitionInitData*>(initData.Get());
					JOwnerPtr<JAnimationFSMtransition> ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimationFSMtransition>(*transitionInitData);
					JAnimationFSMtransition* newTransition = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newTransition;
				}
				return nullptr;
			};
			JFFI<JAnimationFSMtransition>::Register(createTransitionLam);

			SET_GUI_FLAG(J_GUI_OPTION_DISPLAY_PARENT);
		}
		JAnimationFSMtransition::JAnimationFSMtransition(const JFSMtransitionInitData& initData)
			:JFSMtransition(initData)
		{}
		JAnimationFSMtransition::~JAnimationFSMtransition(){}
	}
}