#include"JFSMstate.h"
#include"JFSMtransition.h" 
#include"JFSMdiagram.h"
#include"../Guid/GuIdCreator.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMstate::JFSMstateInitData::JFSMstateInitData(const std::wstring& name, const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram)
			:JFSMIdentifierInitData(name, guid), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::JFSMstateInitData::JFSMstateInitData(const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMstate>(), guid), ownerDiagram(ownerDiagram)
		{}
		bool JFSMstate::JFSMstateInitData::IsValid() noexcept
		{
			return ownerDiagram.IsValid();
		}
		J_FSM_OBJECT_TYPE JFSMstate::JFSMstateInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::STATE;
		}

		J_FSM_OBJECT_TYPE JFSMstate::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::STATE;
		}
		uint JFSMstate::GetTransitionCount()const noexcept
		{
			return (uint)transitionVec.size();
		}
		JFSMtransition* JFSMstate::GetTransition(const size_t guid)noexcept
		{
			int index = JCUtil::GetJIdenIndex(transitionVec, guid);
			if (index != -1)
				return transitionVec[index];
			else
				return nullptr;
		}
		JFSMtransition* JFSMstate::GetTransitionByOutGuid(const size_t outputGuid)noexcept
		{
			const uint transitionSize = (uint)transitionVec.size();
			for (uint i = 0; i < transitionSize; ++i)
			{
				if (transitionVec[i]->GetOutputStateGuid() == outputGuid)
					return transitionVec[i];
			}
			return nullptr;
		}
		JFSMtransition* JFSMstate::GetTransitionByIndex(uint index)noexcept
		{
			if (transitionVec.size() <= index)
				return nullptr;
			else
				return transitionVec[index];
		}
		void JFSMstate::Initialize()noexcept
		{
			decidedNextState = false;
			const uint transitionSize = (uint)transitionVec.size();

			for (uint index = 0; index < transitionSize; ++index)
				transitionVec[index]->Initialize();
		}
		void JFSMstate::EnterState()noexcept
		{
			decidedNextState = false;
		}
		bool JFSMstate::RemoveParameter(const size_t guid)noexcept
		{
			const uint transitionSize = (uint)transitionVec.size();
			for (uint i = 0; i < transitionSize; ++i)
				transitionVec[i]->RemoveParameter(guid);
			return true;
		}
		void JFSMstate::Clear()noexcept
		{
			const uint transitionSize = (uint)transitionVec.size();
			for (uint i = 0; i < transitionSize; ++i)
				JFSMInterface::Destroy(transitionVec[i]);
			transitionVec.clear();
		}
		JFSMparameterStorageUserAccess* JFSMstate::GetParamStorageInterface()const noexcept
		{
			return ownerInterface->GetParamStorageInterface();
		}
		bool JFSMstate::AddType(JFSMtransition* newTransition)noexcept
		{
			JFSMtransition* res = nullptr;
			if (newTransition != nullptr && ownerInterface->IsDiagramState(newTransition->GetOutputStateGuid()))
			{
				newTransition->SetName(JCUtil::MakeUniqueName(transitionVec, newTransition->GetName()));
				transitionVec.push_back(newTransition);
				return true;
			}
			else
				return false;
		}
		bool JFSMstate::RemoveType(JFSMtransition* transition)noexcept
		{
			const uint transitionSize = (uint)transitionVec.size();
			for (uint index = 0; index < transitionSize; ++index)
			{
				if (transitionVec[index]->GetGuid() == transition->GetGuid())
				{ 
					transitionVec.erase(transitionVec.begin() + index);
					return true;
				}
			}
			return false;
		}
		bool JFSMstate::RegisterCashData()noexcept
		{
			return ownerInterface->AddType(this);
		}
		bool JFSMstate::DeRegisterCashData()noexcept
		{
			return ownerInterface->RemoveType(this);
		}
		JFSMstate::JFSMstate(const JFSMstateInitData& initData)
			:JFSMInterface(initData), ownerInterface(initData.ownerDiagram.Get())
		{}
		JFSMstate::~JFSMstate()
		{
			ownerInterface = nullptr;
		}
	}
}