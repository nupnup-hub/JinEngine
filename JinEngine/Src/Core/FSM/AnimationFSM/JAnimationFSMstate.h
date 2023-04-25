#pragma once
#include"JAnimationStateType.h" 
#include"../JFSMstate.h"  

namespace JinEngine
{ 
	namespace Core
	{ 
		class JAnimationUpdateData; 
		class JAnimationFSMtransition; 
		class JAnimationFSMstatePrivate;
		class JAnimationFSMstate : public JFSMstate
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JAnimationFSMstate)
		private:
			friend class JAnimationFSMstatePrivate;
			class JAnimationFSMstateImpl;
		private:
			std::unique_ptr<JAnimationFSMstateImpl> impl;
		public:
			virtual J_ANIMATION_STATE_TYPE GetStateType()const noexcept = 0;
			JAnimationFSMtransition* GetTransitionByIndex(uint index)noexcept;
		public:
			virtual bool CanLoop()const noexcept = 0;
		public:   
			JAnimationFSMtransition* FindNextStateTransition(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept;
		protected:
			JAnimationFSMstate(const InitData& initData);
			~JAnimationFSMstate();
			JAnimationFSMstate(const JAnimationFSMstate& rhs) = delete;
			JAnimationFSMstate& operator=(const JAnimationFSMstate& rhs) = delete;
		};
	}
}
