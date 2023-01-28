#pragma once
#include"JFSMInterface.h"
#include"JFSMconditionValueType.h"
#include"JFSMconditionStorageAccess.h"  
#include"JFSMownerInterface.h"
#include"../JDataType.h" 
#include<memory>
#include<string>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;  
		class JFSMtransition;
		class JFSMstate;
		class JFSMdiagramOwnerInterface;

		class JFSMstateOwnerInterface
		{
		private:
			friend class JFSMstate; 
		private:
			virtual bool IsValidCondition(JFSMcondition* condition) noexcept = 0; 
			virtual bool IsDiagramState(const size_t guid)noexcept = 0;
		private:
			virtual bool AddState(JFSMstate* newState)noexcept = 0;
			virtual bool RemoveState(JFSMstate* state)noexcept = 0;
		};

		class JFSMdiagram : public JFSMInterface, 
			public JFSMstateOwnerInterface,
			public JFSMconditionStorageUserInterface
		{
			REGISTER_CLASS(JFSMdiagram)
		public:
			struct JFSMdiagramInitData : public JFSMIdentifierInitData
			{
			public:
				JFSMdiagramOwnerInterface* diagramOwner = nullptr;
			public:
				JFSMdiagramInitData(const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* diagramOwner);
				JFSMdiagramInitData(const size_t guid, JFSMdiagramOwnerInterface* diagramOwner);
				JFSMdiagramInitData(JFSMdiagramOwnerInterface* diagramOwner);
			public:
				bool IsValid() noexcept;
				J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			}; 
			using InitData = JFSMdiagramInitData;
		public:
			static constexpr uint maxNumberOffState = 100;
		private:
			//vector + unorered map => 64bit overhead 
			JFSMdiagramOwnerInterface* diagramOwner;
			std::vector<JFSMstate*> stateVec;
			std::unordered_map<size_t, JFSMstate*> stateMap;
			JFSMstate* initState = nullptr;
			size_t nowStateGuid; 
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final; 
			uint GetStateCount()const noexcept;
		protected:
			void Initialize()noexcept;
			JFSMstate* GetNowState()noexcept;
			JFSMstate* GetState(const size_t guid)noexcept; 
			JFSMstate* GetStateByIndex(const uint index)noexcept;  
			std::vector<JFSMstate*>& GetStateVec()noexcept;
			JFSMconditionStorageUserAccess* GetStroageUser()noexcept;
		private:
			bool IsValidCondition(JFSMcondition* condition) noexcept final;
			bool IsDiagramState(const size_t guid)noexcept final;
		private:
			bool AddState(JFSMstate* newState)noexcept final;
			bool RemoveState(JFSMstate* state)noexcept final;
		private:
			bool RegisterCashData()noexcept;
			bool DeRegisterCashData()noexcept;
		private:
			void Clear()noexcept override; 
		private:
			void NotifyRemoveCondition(JFSMcondition* condition)noexcept final;
		protected:
			JFSMdiagram(const JFSMdiagramInitData& initData);
			virtual ~JFSMdiagram();
			JFSMdiagram(const JFSMdiagram& rhs) = delete;
			JFSMdiagram& operator=(const JFSMdiagram& rhs) = delete;
		};
	}
}