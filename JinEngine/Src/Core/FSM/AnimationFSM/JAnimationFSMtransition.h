#pragma once
#include"../JFSMtransition.h"  

namespace JinEngine
{
	namespace Core
	{  
		class JAnimationFSMstate;
		class JAnimationFSMtransition final : public JFSMtransition
		{
			REGISTER_CLASS(JAnimationFSMtransition) 
		private:
			friend class JAnimationFSMstate;  
		private:
			REGISTER_PROPERTY_EX(isWaitExitTime, IsWaitExitTime, SetIsWaitExitTime, GUI_CHECKBOX())
			bool isWaitExitTime = true;
			REGISTER_PROPERTY_EX(isFrozen, IsFrozen, SetIsFrozen, GUI_CHECKBOX())
			bool isFrozen = false;
			REGISTER_PROPERTY_EX(exitTimeRate, GetExitTimeRate, SetExitTimeRate, GUI_SLIDER(0.0f, 1.0f))
			float exitTimeRate = 0.75f;
			REGISTER_PROPERTY_EX(durationTime, GetDurationTime, SetDurationTime, GUI_SLIDER(0.0f, 100.0f))
			float durationTime = 0.25f;
			REGISTER_PROPERTY_EX(targetStartTimeRate, GetTargetStartTimeRate, SetTargetStartTimeRate, GUI_SLIDER(0.0f, 1.0f))
			float targetStartTimeRate = 0;
		public:
			REGISTER_PARENT_METHOD_GUI_WIDGET(JFSMtransition, Condition, GetConditionVec, SetConditionVec, GUI_LIST(J_GUI_LIST_TYPE::DYNAMIC, true, CreateCondition))
		public:		
			bool IsWaitExitTime()noexcept;
			bool IsFrozen()noexcept;
			float GetExitTimeRate()noexcept;
			float GetDurationTime()noexcept;
			float GetTargetStartTimeRate()noexcept;

			void SetIsWaitExitTime(const bool value)noexcept;
			void SetIsFrozen(const bool value)noexcept;
			void SetExitTimeRate(const float value)noexcept;
			void SetDurationTime(const float value)noexcept;
			void SetTargetStartTimeRate(const float value)noexcept;

			bool IsSatisfiedOption(const float normalizedTime)noexcept; 
		public:
			void Initialize()noexcept override;
		private:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		private:
			static void RegisterCallOnce();
		private:
			JAnimationFSMtransition(const JFSMtransitionInitData& initData);
			~JAnimationFSMtransition();
		};
	}
}