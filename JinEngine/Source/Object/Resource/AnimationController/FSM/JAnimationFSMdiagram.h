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
#include"../../../../Core/FSM/JFSMdiagram.h"  

namespace JinEngine
{
	class JAnimationClip; 
	class JAnimationController;
	class JSkeletonAsset;

	namespace Graphic
	{
		struct JAnimationConstants;
	}

	class JAnimationFSMstate;
	class JAnimationFSMtransition;
	class JAnimationFSMdiagramPrivate;
	class JAnimationFSMdiagram final : public Core::JFSMdiagram
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JAnimationFSMdiagram)
	public:
		class InitData : public JFSMdiagram::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(Core::JFSMdiagramOwnerInterface* ownerInterface);
			InitData(const std::wstring& name, const size_t guid, Core::JFSMdiagramOwnerInterface* ownerInterface);
		};
	private:
		friend class JAnimationFSMdiagramPrivate;
		class JAnimationFSMdiagramImpl;
	private:
		std::unique_ptr<JAnimationFSMdiagramImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		JUserPtr<JAnimationFSMstate> GetState(const size_t stateGuid)noexcept;
		JUserPtr<JAnimationFSMstate> GetStateByIndex(const uint index)noexcept;
		JUserPtr<JAnimationFSMtransition> GetTransition(const size_t transitionGuid)noexcept;
	public:
		void SetClip(const size_t stateGuid, const JUserPtr<JAnimationClip>& clip)noexcept;
	private:
		JAnimationFSMdiagram(const InitData& initData);
		~JAnimationFSMdiagram();
	};
}