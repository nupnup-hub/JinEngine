#pragma once
#include"JFSMIdentifier.h"
#include"JFSMconditionValueType.h"
#include"JFSMconditionStorageAccess.h"  
#include"../JDataType.h"
#include<memory>
#include<string>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;
		class JFSMconditionWrap;
		class JFSMtransition;
		class JFSMstate;

		class JFSMdiagram : public JFSMIdentifier, public IJFSMconditionStorage
		{
		public:
			static constexpr uint maxNumberOffState = 100;
		private:
			//vector + unorered map => 64bit overhead
			IJFSMconditionStorageUser* conditionStorage;
			std::vector<std::unique_ptr<JFSMstate>> stateVec;
			std::unordered_map<size_t, JFSMstate*> stateMap;
			JFSMstate* initState = nullptr;
			size_t nowStateGuid; 
		public: 
			JFSMdiagram(const std::wstring& name, const size_t guid, IJFSMconditionStorageUser* conditionStorage);
			virtual ~JFSMdiagram();
			JFSMdiagram(const JFSMdiagram& rhs) = delete;
			JFSMdiagram& operator=(const JFSMdiagram& rhs) = delete;
		public:
			J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept final;
			std::wstring GetStateUniqueName(const std::wstring& initName)const noexcept;
		protected:
			void Initialize()noexcept;
			uint GetStateCount()noexcept; 
			JFSMstate* GetNowState()noexcept;
			JFSMstate* GetState(const size_t guid)noexcept; 
			JFSMstate* GetStateByIndex(const uint index)noexcept;
			IJFSMconditionStorageUser* GetIConditionStorage()noexcept;

			void SetTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid, const uint conditionIndex)noexcept;
			void SetTransitionCondtionOnValue(const size_t inputStateGuid, const size_t outputStateGuid, const uint conditionIndex, const float value)noexcept;
 
			JFSMstate* AddState(std::unique_ptr<JFSMstate> state)noexcept;
			JFSMtransition* AddTransition(const size_t stateGuid, std::unique_ptr<JFSMtransition> transition)noexcept; 
			JFSMconditionWrap* AddTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid)noexcept;

			bool RemoveState(const size_t stateGuid)noexcept;
			bool RemoveTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept;
			bool RemoveTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid)noexcept;
			void Clear()noexcept;
		private:
			void NotifyRemoveCondition(JFSMcondition* condition)noexcept final;
		};
	}
}