#include"JFSMdiagram.h" 
#include"JFSMstate.h" 
#include"JFSMcondition.h"
#include"JFSMtransition.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMdiagram::JFSMdiagram(const std::string& name, IJFSMconditionStorageUser* conditionStorage)
			:name(name), conditionStorage(conditionStorage)
		{}
		JFSMdiagram::~JFSMdiagram() {}
		std::string JFSMdiagram::GetName()const noexcept
		{
			return name;
		}
		void JFSMdiagram::Initialize()noexcept
		{
			const uint stateVecSize = (uint)stateVec.size();
			for (uint i = 0; i < stateVecSize; ++i)
				stateVec[i]->Initialize();
		}
		uint JFSMdiagram::GetStateCount()noexcept
		{
			return (uint)stateVec.size();
		}
		bool JFSMdiagram::GetStateId(const std::string& name, size_t& id)noexcept
		{
			const uint stateVecSize = (uint)stateVec.size();
			size_t guid = strHash(name);
			JFSMstate* state = GetState(guid);
			if (state != nullptr)
			{
				id = state->GetId();
				return true;
			}
			else
			{
				return false;
			}
		}
		JFSMstate* JFSMdiagram::GetNowState()noexcept
		{
			auto data = stateMap.find(nowStateId);
			if (data == stateMap.end())
				return nullptr;
			else
				return data->second;
		}
		JFSMstate* JFSMdiagram::GetState(const size_t id)noexcept
		{
			auto data = stateMap.find(id);
			if (data == stateMap.end())
				return nullptr;
			else
				return data->second;
		}
		JFSMstate* JFSMdiagram::GetState(const std::string& name)noexcept
		{
			return GetState(strHash(name));
		}
		void JFSMdiagram::SetTransitionCondition(const std::string& stateName, const std::string& outputStateName, const std::string& newConditionName, const uint oldConditionIndex)noexcept
		{
			JFSMcondition* newCondition = conditionStorage->GetCondition(newConditionName);
			if (newCondition == nullptr)
				return;

			size_t stateid, outputStateId;
			if (GetStateId(stateName, stateid) && GetStateId(outputStateName, outputStateId))
				stateMap[stateid]->SetTransitionCondtion(outputStateId, newCondition, oldConditionIndex);
		}
		void JFSMdiagram::SetTransitionCondtionOnValue(const std::string& stateName, const std::string& outputStateName, const uint conditionIndex, const float value)noexcept
		{
			size_t stateid, outputStateId;
			if (GetStateId(stateName, stateid) && GetStateId(outputStateName, outputStateId))
				stateMap[stateid]->SetTransitionCondtionOnValue(outputStateId, conditionIndex, value);
		}
		void JFSMdiagram::SetStateName(const std::string& oldName, const std::string& newName)
		{
			JFSMstate* tarState = GetState(oldName);
			if (tarState != nullptr)
			{
				const size_t oldGuid = strHash(oldName);
				const size_t newGuid = strHash(newName);
				stateMap.erase(oldGuid);
				tarState->SetName(newName, newGuid);
				stateMap.emplace(newGuid, tarState);
			}
		}
		bool JFSMdiagram::AddTransitionCondition(const size_t stateId, const size_t outputId)noexcept
		{
			auto data = stateMap.find(stateId);
			if (data == stateMap.end())
				return false;

			return data->second->AddTransitionCondition(outputId, nullptr);
		}
		bool JFSMdiagram::AddTransitionCondition(const std::string& stateName, const std::string& outputStateName)noexcept
		{
			size_t stateId;
			size_t outputId;
			if (GetStateId(stateName, stateId), GetStateId(outputStateName, outputId))
				return AddTransitionCondition(stateId, outputId);
			else
				return false;
		}
		bool JFSMdiagram::EraseState(const size_t stateId)noexcept
		{
			if (stateMap.find(stateId) != stateMap.end())
			{
				stateMap.erase(stateId);
				const uint stateVecSize = (uint)stateVec.size();
				for (uint i = 0; i < stateVecSize; ++i)
				{
					if (stateVec[i]->GetId() == stateId)
					{
						stateVec[i]->Clear();
						stateVec[i].reset();
						stateVec.erase(stateVec.begin() + i);
					}
				}
				return true;
			}
			else
				return false;
		}
		bool JFSMdiagram::EraseState(const std::string& stateName)noexcept
		{
			size_t id = 0;
			if (GetStateId(stateName, id))
				EraseState(id);
			else
				return false;
		}
		bool JFSMdiagram::EraseTransition(const size_t stateId, const size_t outputId)noexcept
		{
			if (stateMap.find(stateId) != stateMap.end())
				return stateMap[stateId]->EraseTransition(outputId);
			else
				return false;
		}
		bool JFSMdiagram::EraseTransition(const std::string& stateName, const std::string& outputStateName)noexcept
		{
			size_t stateId;
			size_t outputId;
			if (GetStateId(stateName, stateId), GetStateId(outputStateName, outputId))
				return EraseTransition(stateId, outputId);
			else
				return false;
		}
		bool JFSMdiagram::EraseTransitionCondition(const std::string& stateName, const std::string& outputStateName, const std::string& conditionName)noexcept
		{
			size_t stateid, outputStateId;
			if (GetStateId(stateName, stateid) && GetStateId(outputStateName, outputStateId))
				stateMap[stateid]->EraseTransitionCondition(outputStateId, conditionName);
			else
				return false;
		}
		void JFSMdiagram::Clear()noexcept
		{
			stateMap.clear();
			const uint stateVecCount = (uint)stateVec.size();
			for (uint i = 0; i < stateVecCount; ++i)
			{
				stateVec[i]->Clear();
				stateVec[i].reset();
			}
			stateVec.clear();
		}
		JFSMstate* JFSMdiagram::GetStateByIndex(const uint index)noexcept
		{
			const uint stateVecSize = (uint)stateVec.size();
			if (index >= stateVecSize)
				return nullptr;
			else
				return stateVec[index].get();
		}
		JFSMstate* JFSMdiagram::AddState(std::unique_ptr<JFSMstate> state)noexcept
		{
			JFSMstate* res = nullptr;
			if (state == nullptr)
				return res;

			const uint stateVecSize = (uint)stateVec.size();
			if (stateVecSize >= maxNumberOffState)
				return res;

			static std::string defaultStateName = "newState";
			std::string newName = defaultStateName;
			bool isOk = false;
			int sameCount = 0;

			while (!isOk)
			{
				bool hasSameName = false;
				for (uint i = 0; i < stateVecSize; ++i)
				{
					if (stateVec[i]->GetName() == newName)
					{
						hasSameName = true;
						break;
					}
				}

				if (hasSameName)
				{
					JCommonUtility::ModifyOverlappedName(newName, newName.length(), sameCount);
					++sameCount;
				}
				else
					isOk = true;
			}
			size_t guid = strHash(newName);
			if (stateVec.size() == 0)
			{
				initState = state.get();
				nowStateId = guid;
			}
			state->SetName(newName, guid);
			res = state.get();
			stateMap.emplace(state->GetId(), state.get());
			stateVec.push_back(std::move(state));

			return res;
		}
		JFSMtransition* JFSMdiagram::AddTransition(const size_t stateId, std::unique_ptr<JFSMtransition> transition)noexcept
		{
			if (transition == nullptr)
				return nullptr;

			return stateMap[stateId]->AddTransition(std::move(transition));
		}
		JFSMtransition* JFSMdiagram::AddTransition(const std::string& stateName, std::unique_ptr<JFSMtransition> transition)noexcept
		{
			if (transition == nullptr)
				return nullptr;

			size_t stateId;
			if (GetStateId(stateName, stateId))
				return AddTransition(stateId, std::move(transition));
			else
				return nullptr;
		}
		void JFSMdiagram::NotifyEraseCondition(JFSMcondition* condition)noexcept
		{
			if (condition == nullptr)
				return;

			const uint stateVecCount = (uint)stateVec.size();
			for (uint i = 0; i < stateVecCount; ++i)
				stateVec[i]->EraseCondition(condition->GetName());
		}
	}
}
