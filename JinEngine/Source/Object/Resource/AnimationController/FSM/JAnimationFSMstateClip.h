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
#include"JAnimationFSMstate.h" 

namespace JinEngine
{ 
	class JAnimationClip;
	struct JAnimationTime;
	class JAnimationFSMstateClipPrivate;
	class JAnimationFSMdiagram;
	class JAnimationFSMstateClip final : public JAnimationFSMstate
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JAnimationFSMstateClip)
	public:
		class InitData : public JAnimationFSMstate::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JAnimationFSMdiagram>& ownerDiagram);
			InitData(const std::wstring& name, const size_t guid, const JUserPtr<JAnimationFSMdiagram>& ownerDiagram);
		};
	private:
		friend class JAnimationFSMstateClipPrivate;
		class JAnimationFSMstateClipImpl;
	private:
		std::unique_ptr<JAnimationFSMstateClipImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		J_ANIMATION_STATE_TYPE GetStateType()const noexcept final;
		JUserPtr<JAnimationClip> GetClip()const noexcept;
	public:
		void SetClip(const JUserPtr<JAnimationClip>& newClip)noexcept;
	public:
		bool CanLoop()const noexcept final;
	private:
		JAnimationFSMstateClip(const InitData& initData);
		~JAnimationFSMstateClip();
		JAnimationFSMstateClip(const JAnimationFSMstateClip& rhs) = delete;
		JAnimationFSMstateClip& operator=(const JAnimationFSMstateClip& rhs) = delete;
	};
}