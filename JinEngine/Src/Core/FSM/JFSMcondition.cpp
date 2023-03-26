#include"JFSMcondition.h"
#include"JFSMtransition.h"
#include"JFSMfactory.h"
#include"JFSMparameterStorageAccess.h"
#include"../Guid/GuidCreator.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMcondition::JFSMconditionInitData::JFSMconditionInitData(const std::wstring& name, 
			const size_t guid, 
			JUserPtr<JFSMtransition> ownerTransition)
			:JFSMIdentifierInitData(name, guid), ownerTransition(ownerTransition)
		{}	
		JFSMcondition::JFSMconditionInitData::JFSMconditionInitData(JUserPtr<JFSMtransition> ownerTransition)
			:JFSMIdentifierInitData(GetDefaultName<JFSMcondition>(), Core::MakeGuid()), ownerTransition(ownerTransition)
		{}
		bool JFSMcondition::JFSMconditionInitData::IsValid() noexcept
		{
			return true;
		}
		J_FSM_OBJECT_TYPE JFSMcondition::JFSMconditionInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::CONDITION;
		}

		J_FSM_OBJECT_TYPE JFSMcondition::GetFSMobjType()const noexcept
		{ 
			return J_FSM_OBJECT_TYPE::CONDITION;
		}
		JFSMparameter* JFSMcondition::GetParameter()const noexcept
		{
			return parameter;
		}
		float JFSMcondition::GetOnValue()const noexcept
		{ 
			if (HasParameter())
				return TypeValue(parameter->GetParamType(), onValue);
			else
				return 0;
		}
		void JFSMcondition::SetParameter(JFSMparameter* newParam)noexcept
		{
			if (newParam == nullptr || IsValidParameter(newParam))
				parameter = newParam; 

			if (parameter == nullptr)
				onValue = 0;
		}
		void JFSMcondition::SetOnValue(float newValue)noexcept
		{
			if (HasParameter())
				onValue = TypeValue(parameter->GetParamType(), newValue);
		}
		bool JFSMcondition::HasParameter()const noexcept
		{
			return parameter != nullptr;
		}
		bool JFSMcondition::HasSameParameter(const size_t guid)const noexcept
		{
			return HasParameter() ? parameter->GetGuid() == guid : false;
		}
		bool JFSMcondition::IsSatisfied()const noexcept
		{
			return (PassDefectInspection()) && (parameter->GetValue() == GetOnValue());
		}
		bool JFSMcondition::PassDefectInspection()const noexcept
		{
			return parameter != nullptr;
		}
		std::vector<JIdentifier*> JFSMcondition::GetSroageParameter(JIdentifier* iden)noexcept
		{
			if (!iden->GetTypeInfo().IsChildOf< JFSMcondition>())
				return std::vector<JIdentifier*>{};
			std::vector<JFSMparameter*> paramVec = static_cast<JFSMcondition*>(iden)->ownerInterface->GetParamStorageInterface()->GetParameterVec();		 
			return std::vector<JIdentifier*>(paramVec.begin(), paramVec.end());
		}
		bool JFSMcondition::IsValidParameter(JFSMparameter* newParam)const noexcept
		{
			sizeof(JFSMcondition);
			return newParam ? ownerInterface->GetParamStorageInterface()->GetParameter(newParam->GetGuid()) : false;
		} 
		void JFSMcondition::Clear()
		{
			parameter = nullptr;
		}
		bool JFSMcondition::RegisterCashData()noexcept
		{
			return ownerInterface->AddType(this);
		}
		bool JFSMcondition::DeRegisterCashData()noexcept
		{
			return ownerInterface->RemoveType(this);
		}
		void JFSMcondition::RegisterCallOnce()
		{
			auto createCondLam = [](JOwnerPtr<JFSMIdentifierInitData> initData)-> JFSMInterface*
			{
				if (initData.IsValid() && initData->GetFSMobjType() == J_FSM_OBJECT_TYPE::CONDITION)
				{
					JFSMconditionInitData* condInitData = static_cast<JFSMconditionInitData*>(initData.Get());
					JOwnerPtr<JFSMcondition> ownerPtr = JPtrUtil::MakeOwnerPtr<JFSMcondition>(*condInitData);
					JFSMcondition* newCond = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newCond;
				}
				return nullptr;
			};
			JFFI<JFSMcondition>::Register(createCondLam);
		}
		JFSMcondition::JFSMcondition(const JFSMconditionInitData& initData)
			:JFSMInterface(initData), ownerInterface(initData.ownerTransition.Get())
		{

		}
		JFSMcondition::~JFSMcondition(){}
	}
}