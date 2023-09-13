#pragma once
#include"../../../../Core/FSM/JFSMtransition.h"  

namespace JinEngine
{
	class JAnimationFSMtransitionPrivate;
	class JAnimationFSMtransition final : public Core::JFSMtransition
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JAnimationFSMtransition)
	public:
		class InitData final : public Core::JFSMtransition::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<Core::JFSMstate>& inState, const JUserPtr<Core::JFSMstate>& outState);
			InitData(const std::wstring& name,
				const size_t guid,
				const JUserPtr<Core::JFSMstate>& inState,
				const JUserPtr<Core::JFSMstate>& outState);
		};
	private:
		friend class JAnimationFSMtransitionPrivate;
		class JAnimationFSMtransitionImpl;
	private:
		std::unique_ptr<JAnimationFSMtransitionImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		float GetExitTimeRate()const noexcept;
		float GetDurationTime()const noexcept;
		float GetTargetStartTimeRate()const noexcept;
	public:
		void SetIsWaitExitTime(const bool value)noexcept;
		void SetIsFrozen(const bool value)noexcept;
		void SetExitTimeRate(const float value)noexcept;
		void SetDurationTime(const float value)noexcept;
		void SetTargetStartTimeRate(const float value)noexcept;
	public:
		bool IsWaitExitTime()const noexcept;
		bool IsFrozen()const noexcept;
		bool IsSatisfiedOption(const float normalizedTime)const noexcept;
	private:
		JAnimationFSMtransition(const InitData& initData);
		~JAnimationFSMtransition();
	};
} 
