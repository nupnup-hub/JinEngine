#pragma once
#include"../JFSMtransition.h"  

namespace JinEngine
{
	namespace Core
	{ 
		__interface IJFSMconditionStorageUser;
		class JAnimationFSMstate;

		class JAnimationFSMtransition final : public JFSMtransition
		{
		private:
			friend class JAnimationFSMstate; 
			friend std::unique_ptr<JAnimationFSMtransition>::deleter_type;
		private:
			bool isWaitExitTime = true;
			bool isFrozen = false;
			float exitGameTimerate = 0.8f;
			float durationTime = 0.4f;
			float targetStateOffset = 0;
		public:		
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
		public:
			void Initialize()noexcept override;
		private:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			J_FILE_IO_RESULT LoadData(std::wifstream& stream, IJFSMconditionStorageUser& IConditionUser);
		private:
			JAnimationFSMtransition(const size_t outputStateGuid);  
			~JAnimationFSMtransition();
		};
	}
}