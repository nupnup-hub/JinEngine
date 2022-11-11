#include"JFSMdiagram.h" 
#include"JFSMstate.h" 
#include"JFSMcondition.h"
#include"JFSMtransition.h" 
#include"JFSMfactory.h" 
#include"../Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMdiagram::JFSMdiagramInitData::JFSMdiagramInitData(const std::wstring& name, const size_t guid, JUserPtr<IJFSMdiagramOwner> diagramOwner)
			:JFSMIdentifierInitData(name, guid), diagramOwner(diagramOwner)
		{}
		JFSMdiagram::JFSMdiagramInitData::JFSMdiagramInitData(const size_t guid, JUserPtr<IJFSMdiagramOwner> diagramOwner)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMdiagram>(), guid), diagramOwner(diagramOwner)
		{}
		JFSMdiagram::JFSMdiagramInitData::JFSMdiagramInitData(JUserPtr<IJFSMdiagramOwner> diagramOwner)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMdiagram>(), MakeGuid()), diagramOwner(diagramOwner)
		{}
		bool JFSMdiagram::JFSMdiagramInitData::IsValid() noexcept
		{
			return diagramOwner.IsValid();
		}
		J_FSM_OBJECT_TYPE JFSMdiagram::JFSMdiagramInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::DIAGRAM;
		}

		J_FSM_OBJECT_TYPE JFSMdiagram::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::DIAGRAM;
		}
		std::wstring JFSMdiagram::GetUniqueStateName(const std::wstring& initName)const noexcept
		{
			return JCUtil::MakeUniqueName(stateVec, initName);
		}
		uint JFSMdiagram::GetStateCount()const noexcept
		{
			return (uint)stateVec.size();
		}
		void JFSMdiagram::Initialize()noexcept
		{
			const uint stateVecSize = (uint)stateVec.size();
			for (uint i = 0; i < stateVecSize; ++i)
				stateVec[i]->Initialize();
		}
		JFSMstate* JFSMdiagram::GetNowState()noexcept
		{
			auto data = stateMap.find(nowStateGuid);
			if (data != stateMap.end())
				return data->second;
			else
				return nullptr;
		}
		JFSMstate* JFSMdiagram::GetState(const size_t guid)noexcept
		{
			auto data = stateMap.find(guid);
			if (data != stateMap.end())
				return data->second;
			else
				return nullptr;
		}
		JFSMstate* JFSMdiagram::GetStateByIndex(const uint index)noexcept
		{
			const uint stateCount = (uint)stateVec.size();
			if (index < stateCount)
				return stateVec[index];
			else
				return nullptr;
		}
		std::vector<JFSMstate*>& JFSMdiagram::GetStateVec()noexcept
		{
			return stateVec;
		}
		IJFSMconditionStorageUser* JFSMdiagram::GetStroageUser()noexcept
		{
			return diagramOwner->GetConditionStorageUser();
		}
		bool JFSMdiagram::IsValidCondition(JFSMcondition* condition) noexcept
		{
			return condition != nullptr && diagramOwner->GetConditionStorageUser()->GetCondition(condition->GetGuid());
		}
		bool JFSMdiagram::IsDiagramState(const size_t guid)noexcept
		{
			return GetState(guid) != nullptr;
		}
		bool JFSMdiagram::AddState(JFSMstate* newState)noexcept
		{
			if (newState == nullptr)
				return false;

			const uint stateVecSize = (uint)stateVec.size();
			if (stateVecSize >= maxNumberOffState)
				return false;

			if (stateVec.size() == 0)
			{
				initState = newState;
				nowStateGuid = newState->GetGuid();
			}
			stateMap.emplace(newState->GetGuid(), newState);
			stateVec.push_back(newState);

			return true;
		}
		bool JFSMdiagram::RemoveState(JFSMstate* state)noexcept
		{
			if (state == nullptr)
				return false;

			const size_t guid = state->GetGuid();
			if (stateMap.find(guid) != stateMap.end())
			{
				stateMap.erase(guid);
				int index = JCUtil::GetJIdenIndex(stateVec, guid);
				if (index != -1)
				{
					stateVec[index]->Clear();
					stateVec.erase(stateVec.begin() + index);
					const uint stateCount = (uint)stateVec.size();
					for (uint i = 0; i < stateCount; ++i)
						stateVec[i]->RemoveTransition(state->GetGuid());

					return true;
				}
				else
					return false;
			}
			else
				return false;
		}
		bool JFSMdiagram::RegisterCashData()noexcept
		{
			return diagramOwner->AddDiagram(this);
		}
		bool JFSMdiagram::DeRegisterCashData()noexcept
		{
			return diagramOwner->RemoveDiagram(this);
		}
		void JFSMdiagram::Clear()noexcept
		{
			stateMap.clear();
			const uint stateVecCount = (uint)stateVec.size();
			for (uint i = 0; i < stateVecCount; ++i)
				stateVec[i]->Destroy();
			stateVec.clear();
		}
		void JFSMdiagram::NotifyRemoveCondition(JFSMcondition* condition)noexcept
		{
			if (condition == nullptr)
				return;

			const uint stateVecCount = (uint)stateVec.size();
			for (uint i = 0; i < stateVecCount; ++i)
				stateVec[i]->RemoveCondition(condition->GetGuid());
		}
		JFSMdiagram::JFSMdiagram(const JFSMdiagramInitData& initData)
			:JFSMInterface(initData.diagramOwner->GetUniqueDiagramName(initData.name), initData.guid),
			diagramOwner(initData.diagramOwner.Get())
		{
			diagramOwner->GetConditionStorageUser()->AddUser(this, GetGuid());
		}
		JFSMdiagram::~JFSMdiagram()
		{
			diagramOwner->GetConditionStorageUser()->RemoveUser(this, GetGuid());
		}
	}
}
