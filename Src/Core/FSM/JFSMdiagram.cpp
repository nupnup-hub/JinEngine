#include"JFSMdiagram.h" 
#include"JFSMstate.h" 
#include"JFSMcondition.h"
#include"JFSMtransition.h" 
#include"../../Utility/JCommonUtility.h"
 
namespace JinEngine
{
	namespace Core
	{
		JFSMdiagram::JFSMdiagram(const std::string& name, const size_t guid, IJFSMconditionStorageUser* conditionStorage)
			: name(name), guid(guid), conditionStorage(conditionStorage)
		{}
		JFSMdiagram::~JFSMdiagram() {}
		std::string JFSMdiagram::GetName()const noexcept
		{
			return name;
		}
		std::string JFSMdiagram::GetStateUniqueName(const std::string& initName)const noexcept
		{
			return JCommonUtility::MakeUniqueName(stateVec, initName);
		}
		size_t JFSMdiagram::GetGuid()const noexcept
		{
			return guid;
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
		JFSMstate* JFSMdiagram::GetNowState()noexcept
		{
			auto data = stateMap.find(nowStateId);
			if (data == stateMap.end())
				return nullptr;
			else
				return data->second;
		}
		JFSMstate* JFSMdiagram::GetState(const size_t guid)noexcept
		{
			auto data = stateMap.find(guid);
			if (data == stateMap.end())
				return nullptr;
			else
				return data->second;
		}
		void JFSMdiagram::SetTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid, const uint conditionIndex)noexcept
		{
			JFSMcondition* newCondition = conditionStorage->GetCondition(conditionGuid);
			if (newCondition == nullptr)
				return;

			auto inputState = stateMap.find(inputStateGuid);
			auto outputState = stateMap.find(outputStateGuid);
			if (inputState != stateMap.end() && outputState != stateMap.end())
				inputState->second->SetTransitionCondtion(outputStateGuid, conditionIndex, newCondition);
		}
		void JFSMdiagram::SetTransitionCondtionOnValue(const size_t inputStateGuid, const size_t outputStateGuid, const uint conditionIndex, const float value)noexcept
		{
			auto inputState = stateMap.find(inputStateGuid);
			auto outputState = stateMap.find(outputStateGuid);
			if (inputState != stateMap.end() && outputState != stateMap.end())
				inputState->second->SetTransitionCondtionOnValue(outputStateGuid, conditionIndex, value);
		}
		void JFSMdiagram::SetStateName(const size_t guid, const std::string& newName)
		{
			JFSMstate* tarState = GetState(guid);
			if (tarState != nullptr)
				tarState->SetName(newName);
		}
		bool JFSMdiagram::AddTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuId)noexcept
		{
			auto data = stateMap.find(inputStateGuid);
			if (data == stateMap.end())
				return false;

			return data->second->AddTransitionCondition(outputStateGuId, nullptr);
		}
		bool JFSMdiagram::DestroyState(const size_t stateId)noexcept
		{
			if (stateMap.find(stateId) != stateMap.end())
			{
				stateMap.erase(stateId);
				const uint stateVecSize = (uint)stateVec.size();
				for (uint i = 0; i < stateVecSize; ++i)
				{
					if (stateVec[i]->GetGuid() == stateId)
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
		bool JFSMdiagram::RemoveTransition(const size_t inputStateGuid, const size_t outputStateGuId)noexcept
		{
			JFSMstate* state = GetState(inputStateGuid);
			return state != nullptr ? state->RemoveTransition(outputStateGuId) : false;
		}
		bool JFSMdiagram::RemoveTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuId, const size_t conditionGuid)noexcept
		{
			auto inputState = stateMap.find(inputStateGuid);
			auto outputState = stateMap.find(outputStateGuId);

			if (inputState != stateMap.end() && outputState != stateMap.end())
				inputState->second->RemoveTransitionCondition(outputStateGuId, conditionGuid);
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
			  
			std::string newName = JCommonUtility::MakeUniqueName(stateVec, state->GetName());
			if (stateVec.size() == 0)
			{
				initState = state.get();
				nowStateId = state->GetGuid();
			}
			state->SetName(newName);
			res = state.get();
			stateMap.emplace(state->GetGuid(), state.get());
			stateVec.push_back(std::move(state));

			return res;
		}
		JFSMtransition* JFSMdiagram::AddTransition(const size_t stateId, std::unique_ptr<JFSMtransition> transition)noexcept
		{
			if (transition == nullptr)
				return nullptr;

			auto state = stateMap.find(stateId);
			return state != stateMap.end() ? state->second->AddTransition(std::move(transition)) : nullptr;
		}
		void JFSMdiagram::NotifyRemoveCondition(JFSMcondition* condition)noexcept
		{
			if (condition == nullptr)
				return;

			const uint stateVecCount = (uint)stateVec.size();
			for (uint i = 0; i < stateVecCount; ++i)
				stateVec[i]->RemoveCondition(condition->GetGuid());
		}
	}
}
