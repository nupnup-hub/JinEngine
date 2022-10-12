#pragma once
#include"JFSMInterface.h"
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
		class JFSMdiagram; 
		class JFSMstateOwner;

		class JFSMstate : public JFSMInterface
		{
			REGISTER_CLASS(JFSMstate)
		private:
			friend class JFSMdiagram;
		public:
			struct JFSMstateInitData : public JFSMIdentifierInitData
			{
			public:
				JUserPtr<JFSMdiagram> ownerDiagram;
			public:
				JFSMstateInitData(const std::wstring& name, const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram);
				JFSMstateInitData(const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept;
			};
			using InitData = JFSMstateInitData;
		public:
			static constexpr uint maxNumberOffTransistions = 100;
		private:
			std::vector<std::unique_ptr<JFSMtransition>> transition;
			bool decidedNextState = false;
			JFSMstateOwner* ownerDiagram;
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			uint GetTransitionCount()const noexcept;
		protected:
			JFSMtransition* GetTransition(uint index)noexcept;
		protected: 
			void SetTransitionCondtion(const size_t outputStateGuid, const uint conditionIndex, JFSMcondition* newCondition)noexcept;
			void SetTransitionCondtionOnValue(const size_t outputStateGuid, const uint conditionIndex, const float onValue)noexcept;
		protected:
			virtual void Initialize()noexcept;
			void EnterState()noexcept;
		protected:
			JFSMtransition* AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept;
			JFSMconditionWrap* AddTransitionCondition(const size_t outputStateGuid, JFSMcondition* condition)noexcept;
			bool RemoveTransition(const size_t outputStateGuid)noexcept;
			bool RemoveCondition(const size_t guid)noexcept;
			bool RemoveTransitionCondition(const size_t outputStateGuid, const size_t conditionGuid)noexcept;
			void Clear()noexcept override; 
		private:
			bool RegisterCashData()noexcept final;
			bool DeRegisterCashData()noexcept final;
		protected:
			JFSMstate(const JFSMstateInitData& initData);
			~JFSMstate();
			JFSMstate(const JFSMstate& rhs) = delete;
			JFSMstate& operator=(const JFSMstate& rhs) = delete;
		};
	}
}