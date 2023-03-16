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
		class JFSMtransition; 
		class JFSMdiagram; 
		class JFSMstateOwnerInterface;
		__interface JFSMparameterStorageUserAccess;

		class JFSMtransitionOwnerInterface : public JTypeCashInterface<JFSMtransition>
		{
		private:
			friend class JFSMtransition;
		private:
			virtual JFSMparameterStorageUserAccess* GetParamStorageInterface()const noexcept = 0;
			virtual bool IsSameDiagram(const size_t diagramGuid)const noexcept = 0;
		};

		class JFSMstate : public JFSMInterface, public JFSMtransitionOwnerInterface
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
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			};
			using InitData = JFSMstateInitData;
		public:
			static constexpr uint maxNumberOffTransistions = 100;
		private:
			JFSMstateOwnerInterface* ownerInterface;
			std::vector<JFSMtransition*> transitionVec;
			bool decidedNextState = false;
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			uint GetTransitionCount()const noexcept;
			JFSMtransition* GetTransition(const size_t guid)noexcept;
			JFSMtransition* GetTransitionByOutGuid(const size_t outputGuid)noexcept;
			JFSMtransition* GetTransitionByIndex(uint index)noexcept; 
		protected:
			virtual void Initialize()noexcept;
			void EnterState()noexcept;
		protected:  
			bool RemoveParameter(const size_t guid)noexcept; 
			void Clear()noexcept override; 
		private:
			JFSMparameterStorageUserAccess* GetParamStorageInterface()const noexcept final;
			bool IsSameDiagram(const size_t diagramGuid)const noexcept final;
		private:
			bool AddType(JFSMtransition* newTransition)noexcept final;
			bool RemoveType(JFSMtransition* transition)noexcept final;
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