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