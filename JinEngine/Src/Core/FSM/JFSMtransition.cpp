#include"JFSMtransition.h"
#include"JFSMstate.h"
#include"JFSMcondition.h"   
#include"JFSMfactory.h"
#include"../../Utility/JCommonUtility.h" 

namespace JinEngine
{
	namespace Core
	{
		JFSMtransition::JFSMtransitionInitData::JFSMtransitionInitData(const std::wstring& name, 
			const size_t guid,
			const size_t outputGuid,
			JUserPtr<JFSMstate> owneState)
			:JFSMIdentifierInitData(name, guid), outputGuid(outputGuid), owneState(owneState)
		{}
		bool JFSMtransition::JFSMtransitionInitData::IsValid() noexcept
		{
			return owneState.IsValid();
		}
		J_FSM_OBJECT_TYPE JFSMtransition::JFSMtransitionInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::TRANSITION;
		}

		J_FSM_OBJECT_TYPE JFSMtransition::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::TRANSITION;
		}
		uint JFSMtransition::GetConditioCount()const noexcept
		{
			return (uint)conditionVec.size();
		}
		float JFSMtransition::GetConditionOnValue(const uint index)const noexcept
		{
			if (index >= conditionVec.size())
				return errorOnValue;

			return conditionVec[index]->GetOnValue();
		}
		size_t JFSMtransition::GetOutputStateGuid()const noexcept
		{
			return outputStateGuid;
		}
		JFSMcondition* JFSMtransition::GetConditionByIndex(const uint index)const noexcept
		{
			if (index >= conditionVec.size())
				return nullptr;

			return conditionVec[index];
		}
		std::vector<JFSMcondition*> JFSMtransition::GetConditionVec()const noexcept
		{
			return conditionVec;
		}
		void JFSMtransition::SetConditionVec(std::vector<JFSMcondition*> vec)noexcept
		{
			conditionVec = vec;
		}
		JFSMparameterStorageUserAccess* JFSMtransition::GetParamStorageInterface()const noexcept
		{
			return ownerInterface->GetParamStorageInterface();
		}
		bool JFSMtransition::IsSameDiagram(const size_t diagramGuid)const noexcept
		{
			return ownerInterface->IsSameDiagram(diagramGuid);
		}
		bool JFSMtransition::HasSatisfiedCondition()const noexcept
		{
			const uint condVecSize = (uint)conditionVec.size();

			if (condVecSize == 0)
				return true;

			for (uint index = 0; index < condVecSize; ++index)
			{
				if (conditionVec[index]->IsSatisfied())
					return true;
			}
			return false;
		}
		bool JFSMtransition::AddType(JFSMcondition* newCondition)noexcept
		{
			JFSMcondition* res = nullptr;
			if (newCondition != nullptr)
			{
				newCondition->SetName(JCUtil::MakeUniqueName(conditionVec, newCondition->GetName()));
				conditionVec.push_back(newCondition);
				return true;
			}
			else
				return false;		 
		}
		bool JFSMtransition::RemoveType(JFSMcondition* condition)noexcept
		{
			const size_t guid = condition->GetGuid();
			const uint conditionVecSize = (uint)conditionVec.size();
			for (uint i = 0; i < conditionVecSize; ++i)
			{
				if (conditionVec[i]->GetGuid() == guid)
				{
					conditionVec.erase(conditionVec.begin() + i);
					break;
				}
			}
			return true;
		}
		JIdentifier* JFSMtransition::CreateCondition(JIdentifier* iden)noexcept
		{
			if (!iden->GetTypeInfo().IsChildOf< JFSMtransition>())
				return nullptr;

			using InitData = JFSMcondition::InitData;
			return JFFI<JFSMcondition>::Create(JPtrUtil::MakeOwnerPtr<InitData>(GetUserPtr<JFSMtransition>(iden)));
		}
		bool JFSMtransition::RemoveParameter(const size_t guid)noexcept
		{
			const uint conditionVecSize = (uint)conditionVec.size();
			for (uint index = 0; index < conditionVecSize; ++index)
			{
				if (conditionVec[index]->HasSameParameter(guid))
					conditionVec[index]->SetParameter(nullptr);
			}
			return true;
		}
		void JFSMtransition::Initialize()noexcept
		{
			const uint conditionVecSize = (uint)conditionVec.size();
			for (uint i = 0; i < conditionVecSize; ++i)
				conditionVec[i]->SetOnValue(0);
		}
		void JFSMtransition::Clear()
		{
			std::vector<JFSMcondition*> copy = conditionVec;
			const uint conditionVecSize = (uint)copy.size();
			//for (uint i = 0; i < conditionVecSize; ++i)
			//	copy[i]->Clear();
			for (uint i = 0; i < conditionVecSize; ++i)
				JFSMInterface::BeginDestroy(copy[i]);
			conditionVec.clear();
		}
		bool JFSMtransition::RegisterCashData()noexcept
		{
			if (ownerInterface == nullptr)
				MessageBox(0, L"NLL", 0, 0);
			return ownerInterface->AddType(this);
		}
		bool JFSMtransition::DeRegisterCashData()noexcept
		{
			return ownerInterface->RemoveType(this);
		}
		JFSMtransition::JFSMtransition(const JFSMtransitionInitData& initData)
			:JFSMInterface(initData),
			outputStateGuid(initData.outputGuid),
			ownerInterface(initData.owneState.Get())
		{}
		JFSMtransition::~JFSMtransition()
		{
			ownerInterface = nullptr;
		}
	}
}