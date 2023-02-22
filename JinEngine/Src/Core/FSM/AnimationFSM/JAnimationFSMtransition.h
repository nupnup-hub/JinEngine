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
			REGISTER_PROPERTY_EX(exitGameTimerate, GetExitGameTimerate, SetExitGameTimerate, GUI_SLIDER(0.0f, 1.0f))
			float exitGameTimerate = 0.8f;
			REGISTER_PROPERTY_EX(durationTime, GetDurationTime, SetDurationTime, GUI_SLIDER(0.0f, 1.0f))
			float durationTime = 0.4f;
			REGISTER_PROPERTY_EX(targetStateTimeOffset, GetTargetStateTimeOffset, SetTargetStateTimeOffset, GUI_SLIDER(0.0f, 1.0f))
			float targetStateTimeOffset = 0;
		public:
			REGISTER_PARENT_METHOD_GUI_WIDGET(JFSMtransition, Condition, GetConditionVec, SetConditionVec, GUI_LIST(J_GUI_LIST_TYPE::DYNAMIC, true, CreateCondition))
		public:		
			bool IsWaitExitTime()noexcept;
			bool IsFrozen()noexcept;
			float GetExitGameTimerate()noexcept;
			float GetDurationTime()noexcept;
			float GetTargetStateTimeOffset()noexcept;

			void SetIsWaitExitTime(const bool value)noexcept;
			void SetIsFrozen(const bool value)noexcept;
			void SetExitGameTimerate(const float value)noexcept;
			void SetDurationTime(const float value)noexcept;
			void SetTargetStateTimeOffset(const float value)noexcept;

			bool IsSatisfiedOption(const float normalizedTime)noexcept;
		public:
			void Initialize()noexcept override;
		private:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream);
		private:
			static void RegisterJFunc();
		private:
			JAnimationFSMtransition(const JFSMtransitionInitData& initData);
			~JAnimationFSMtransition();
		};
	}
}