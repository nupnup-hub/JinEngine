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
#include"JAnimationStateType.h" 
#include"../../../../Core/Fsm/JFSMstate.h"  

namespace JinEngine
{ 
	class JAnimationUpdateData;
	class JAnimationFSMtransition;
	class JAnimationFSMstatePrivate;
	class JAnimationFSMstate : public Core::JFSMstate
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JAnimationFSMstate)
	private:
		friend class JAnimationFSMstatePrivate;
		class JAnimationFSMstateImpl;
	private:
		std::unique_ptr<JAnimationFSMstateImpl> impl;
	public:
		virtual J_ANIMATION_STATE_TYPE GetStateType()const noexcept = 0;
		JUserPtr<JAnimationFSMtransition> GetTransitionByIndex(uint index)noexcept;
	public:
		virtual bool CanLoop()const noexcept = 0;
	public:
		JUserPtr<JAnimationFSMtransition> FindNextStateTransition(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept;
	protected:
		JAnimationFSMstate(const InitData& initData);
		~JAnimationFSMstate();
		JAnimationFSMstate(const JAnimationFSMstate& rhs) = delete;
		JAnimationFSMstate& operator=(const JAnimationFSMstate& rhs) = delete;
	};
}
