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