#include"JFSMdiagram.h" 
#include"JFSMstate.h"  
#include"JFSMtransition.h" 
#include"JFSMfactory.h" 
#include"JFSMownerInterface.h"
#include"../Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMdiagram::JFSMdiagramInitData::JFSMdiagramInitData(const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* ownerInterface)
			:JFSMIdentifierInitData(name, guid), ownerInterface(ownerInterface)
		{}
		JFSMdiagram::JFSMdiagramInitData::JFSMdiagramInitData(const size_t guid, JFSMdiagramOwnerInterface* ownerInterface)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMdiagram>(), guid), ownerInterface(ownerInterface)
		{}
		JFSMdiagram::JFSMdiagramInitData::JFSMdiagramInitData(JFSMdiagramOwnerInterface* ownerInterface)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMdiagram>(), MakeGuid()), ownerInterface(ownerInterface)
		{}
		bool JFSMdiagram::JFSMdiagramInitData::IsValid() noexcept
		{
			return ownerInterface != nullptr;
		}
		J_FSM_OBJECT_TYPE JFSMdiagram::JFSMdiagramInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::DIAGRAM;
		}

		J_FSM_OBJECT_TYPE JFSMdiagram::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::DIAGRAM;
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
		JFSMstate* JFSMdiagram::GetNowState()const noexcept
		{
			auto data = stateMap.find(nowStateGuid);
			if (data != stateMap.end())
				return data->second;
			else
				return nullptr;
		}
		JFSMstate* JFSMdiagram::GetState(const size_t guid)const noexcept
		{
			auto data = stateMap.find(guid);
			if (data != stateMap.end())
				return data->second;
			else
				return nullptr;
		}
		JFSMstate* JFSMdiagram::GetStateByIndex(const uint index)const noexcept
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
		JFSMparameterStorageUserAccess* JFSMdiagram::GetParamStorageInterface()const noexcept
		{
			return ownerInterface->GetParameterStorageUser();
		}
		bool JFSMdiagram::IsDiagramState(const size_t guid)const noexcept
		{
			return GetState(guid) != nullptr;
		}
		bool JFSMdiagram::AddType(JFSMstate* newState)noexcept
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
			newState->SetName(JCUtil::MakeUniqueName(stateVec, newState->GetName()));
			stateMap.emplace(newState->GetGuid(), newState);
			stateVec.push_back(newState);

			return true;
		}
		bool JFSMdiagram::RemoveType(JFSMstate* state)noexcept
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
						Destroy(stateVec[i]->GetTransitionByOutGuid(state->GetGuid()));

					if (guid == nowStateGuid)
					{
						nowStateGuid = 0;
						if (stateVec.size() > 0)
							nowStateGuid = stateVec[0]->GetGuid();
					}
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
			return ownerInterface->AddType(this);
		}
		bool JFSMdiagram::DeRegisterCashData()noexcept
		{
			return ownerInterface->RemoveType(this);
		}
		void JFSMdiagram::Clear()noexcept
		{
			std::vector<JFSMstate*> copy = stateVec;
			const uint stateVecCount = (uint)copy.size();
			//for (uint i = 0; i < stateVecCount; ++i)
			//	copy[i]->Clear(); 
			for (uint i = 0; i < stateVecCount; ++i)
				JFSMInterface::Destroy(copy[i]);
			initState = nullptr;
			stateMap.clear();
			stateVec.clear();
		}
		void JFSMdiagram::NotifyRemoveParameter(const size_t guid)noexcept
		{ 
			const uint stateVecCount = (uint)stateVec.size();
			for (uint i = 0; i < stateVecCount; ++i)
				stateVec[i]->RemoveParameter(guid);
		}
		JFSMdiagram::JFSMdiagram(const JFSMdiagramInitData& initData)
			:JFSMInterface(initData),
			ownerInterface(initData.ownerInterface)
		{
			ownerInterface->GetParameterStorageUser()->AddUser(this, GetGuid());
		}
		JFSMdiagram::~JFSMdiagram()
		{
			ownerInterface->GetParameterStorageUser()->RemoveUser(this, GetGuid());
		}
	}
}
