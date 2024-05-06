/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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
