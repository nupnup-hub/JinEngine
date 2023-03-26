#include"JFSMparameter.h" 
#include"JFSMparameterStorage.h"
#include"JFSMownerInterface.h" 
#include"JFSMfactory.h"

namespace JinEngine
{
	namespace Core
	{
		JFSMparameter::JFSMparameterInitData::JFSMparameterInitData(const std::wstring& name, const size_t guid, const J_FSM_PARAMETER_VALUE_TYPE paramType, IJFSMparameterStorageInterface* paramStorage)
			:JFSMIdentifierInitData(name, guid), paramType(paramType), paramStorage(paramStorage)
		{}
		JFSMparameter::JFSMparameterInitData::JFSMparameterInitData(const size_t guid, IJFSMparameterStorageInterface* paramStorage)
			: JFSMIdentifierInitData(JIdentifier::GetDefaultName<JFSMparameter>(), guid), paramType(J_FSM_PARAMETER_VALUE_TYPE::BOOL), paramStorage(paramStorage)
		{} 
		bool JFSMparameter::JFSMparameterInitData::IsValid() noexcept
		{
			return paramStorage != nullptr;
		}
		J_FSM_OBJECT_TYPE JFSMparameter::JFSMparameterInitData::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::PARAMETER;
		}

		void JFSMparameter::Initialize()noexcept
		{
			value = 0;
		}
		J_FSM_OBJECT_TYPE JFSMparameter::GetFSMobjType()const noexcept
		{ 
			return J_FSM_OBJECT_TYPE::PARAMETER;
		}
		float JFSMparameter::GetValue()const noexcept
		{
			return TypeValue(paramType, value);
		}
		J_FSM_PARAMETER_VALUE_TYPE JFSMparameter::GetParamType()const noexcept
		{
			return paramType;
		} 
		void JFSMparameter::SetParamType(const J_FSM_PARAMETER_VALUE_TYPE paramType)noexcept
		{
			JFSMparameter::paramType = paramType;
			SetValue(value);
		}
		void JFSMparameter::Clear(){}
		bool JFSMparameter::RegisterCashData()noexcept
		{
			return paramStorage->AddParameter(this);
		}
		bool JFSMparameter::DeRegisterCashData()noexcept
		{ 
			return paramStorage->RemoveParameter(this);
		}
		void JFSMparameter::RegisterCallOnce()
		{			 
			auto createParamLam = [](JOwnerPtr<JFSMIdentifierInitData> initData)-> JFSMInterface*
			{ 
				if (initData.IsValid() && initData->GetFSMobjType() == J_FSM_OBJECT_TYPE::PARAMETER)
				{
					JFSMparameterInitData* paramInitData = static_cast<JFSMparameterInitData*>(initData.Get());
					JOwnerPtr<JFSMparameter> ownerPtr = JPtrUtil::MakeOwnerPtr<JFSMparameter>(*paramInitData);
					JFSMparameter* newCond = ownerPtr.Get();
					if (AddInstance(std::move(ownerPtr)))
						return newCond;
				}
				return nullptr;
			};
			JFFI<JFSMparameter>::Register(createParamLam);
		}
		JFSMparameter::JFSMparameter(const JFSMparameterInitData& initData)
			:JFSMInterface(initData), paramType(initData.paramType)
		{
			paramStorage = initData.paramStorage;
		}
		JFSMparameter::~JFSMparameter() 
		{
			paramStorage = nullptr;
		}
	}
}