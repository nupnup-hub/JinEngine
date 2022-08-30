#pragma once
#include"JFSMIdentifier.h"
#include"../JDataType.h"
#include<string>
#include<vector>
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;
		class JFSMconditionWrap;
		class JFSMtransition;

		class JFSMstate : public JFSMIdentifier
		{
			friend class JFSMdiagram;
		public:
			static constexpr uint maxNumberOffTransistions = 100;
		private:
			std::vector<std::unique_ptr<JFSMtransition>> transition;
			bool decidedNextState = false;
		public:
			JFSMstate(const std::wstring& name, const size_t id);
			~JFSMstate();
			JFSMstate(const JFSMstate& rhs) = delete;
			JFSMstate& operator=(const JFSMstate& rhs) = delete;
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
		protected: 
			void SetTransitionCondtion(const size_t outputStateGuid, const uint conditionIndex, JFSMcondition* newCondition)noexcept;
			void SetTransitionCondtionOnValue(const size_t outputStateGuid, const uint conditionIndex, const float onValue)noexcept;
		protected:
			virtual void Initialize()noexcept;
			void EnterState()noexcept;
		protected:
			JFSMtransition* AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept;
			bool RemoveTransition(const size_t outputStateGuid)noexcept;
			JFSMconditionWrap* AddTransitionCondition(const size_t outputStateGuid, JFSMcondition* condition)noexcept;
			bool RemoveCondition(const size_t guid)noexcept;
			bool RemoveTransitionCondition(const size_t outputStateGuid, const size_t conditionGuid)noexcept;
			virtual void Clear()noexcept;
		};
	}
}