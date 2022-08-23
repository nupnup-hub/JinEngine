#pragma once
#include<memory>
#include<string>
#include<unordered_map>
#include"../JDataType.h"
#include"JFSMconditionValueType.h"
#include"JFSMconditionStorageAccess.h"  

namespace JinEngine
{
	namespace Core
	{
		class JFSMcondition;
		class JFSMtransition;
		class JFSMstate;

		class JFSMdiagram : public IJFSMconditionStorage
		{
		public:
			static constexpr uint maxNumberOffState = 100;
		private:
			//vector + unorered map => 64bit overhead
			std::string name;
			const size_t guid;
			IJFSMconditionStorageUser* conditionStorage;
			std::vector<std::unique_ptr<JFSMstate>> stateVec;
			std::unordered_map<size_t, JFSMstate*> stateMap;
			JFSMstate* initState = nullptr;
			size_t nowStateId; 
		public: 
			JFSMdiagram(const std::string& name, const size_t guid, IJFSMconditionStorageUser* conditionStorage);
			virtual ~JFSMdiagram();
			JFSMdiagram(const JFSMdiagram& rhs) = delete;
			JFSMdiagram& operator=(const JFSMdiagram& rhs) = delete;

			std::string GetName()const noexcept;
			std::string GetStateUniqueName(const std::string& initName)const noexcept;
			size_t GetGuid()const noexcept;
		protected:
			void Initialize()noexcept;
			uint GetStateCount()noexcept; 
			JFSMstate* GetNowState()noexcept;
			JFSMstate* GetState(const size_t guid)noexcept; 
			void SetTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid, const uint conditionIndex)noexcept;
			void SetTransitionCondtionOnValue(const size_t inputStateGuid, const size_t outputStateGuid, const uint conditionIndex, const float value)noexcept;
			void SetStateName(const size_t guid, const std::string& newName);

			bool AddTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept;
			bool DestroyState(const size_t stateId)noexcept; 
			bool RemoveTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept;
			bool RemoveTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid)noexcept;
			void Clear()noexcept;

			JFSMstate* GetStateByIndex(const uint index)noexcept;
			JFSMstate* AddState(std::unique_ptr<JFSMstate> state)noexcept;
			JFSMtransition* AddTransition(const size_t stateId, std::unique_ptr<JFSMtransition> transition)noexcept; 
		private:
			void NotifyRemoveCondition(JFSMcondition* condition)noexcept final;
		};
	}
}