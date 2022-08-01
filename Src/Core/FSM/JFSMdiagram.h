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
			IJFSMconditionStorageUser* conditionStorage;
			std::vector<std::unique_ptr<JFSMstate>> stateVec;
			std::unordered_map<size_t, JFSMstate*> stateMap;
			JFSMstate* initState = nullptr;
			size_t nowStateId;
			std::hash<std::string> strHash;
		public:
			JFSMdiagram(const std::string& name, IJFSMconditionStorageUser* conditionStorage);
			virtual ~JFSMdiagram();
			JFSMdiagram(const JFSMdiagram& rhs) = delete;
			JFSMdiagram& operator=(const JFSMdiagram& rhs) = delete;

			std::string GetName()const noexcept;
		protected:
			void Initialize()noexcept;
			uint GetStateCount()noexcept;
			bool GetStateId(const std::string& name, size_t& id)noexcept;
			JFSMstate* GetNowState()noexcept;
			JFSMstate* GetState(const size_t id)noexcept;
			JFSMstate* GetState(const std::string& name)noexcept;
			void SetTransitionCondition(const std::string& stateName, const std::string& outputStateName, const std::string& newConditionName, const uint oldConditionIndex)noexcept;
			void SetTransitionCondtionOnValue(const std::string& stateName, const std::string& outputStateName, const uint conditionIndex, const float value)noexcept;
			void SetStateName(const std::string& oldName, const std::string& newName);

			bool AddTransitionCondition(const size_t stateId, const size_t outputId)noexcept;
			bool AddTransitionCondition(const std::string& stateName, const std::string& outputStateName)noexcept;
			bool EraseState(const size_t stateId)noexcept;
			bool EraseState(const std::string& stateName)noexcept;
			bool EraseTransition(const size_t stateId, const size_t outputId)noexcept;
			bool EraseTransition(const std::string& stateName, const std::string& outputStateName)noexcept;
			bool EraseTransitionCondition(const std::string& stateName, const std::string& outputStateName, const std::string& conditionName)noexcept;
			void Clear()noexcept;

			JFSMstate* GetStateByIndex(const uint index)noexcept;
			JFSMstate* AddState(std::unique_ptr<JFSMstate> state)noexcept;
			JFSMtransition* AddTransition(const size_t stateId, std::unique_ptr<JFSMtransition> transition)noexcept;
			JFSMtransition* AddTransition(const std::string& stateName, std::unique_ptr<JFSMtransition> transition)noexcept;
		private:
			void NotifyEraseCondition(JFSMcondition* condition)noexcept final;
		};
	}
}