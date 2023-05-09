#include"JFSMcondition.h"
#include"JFSMconditionPrivate.h"
#include"JFSMdiagramPrivate.h"
#include"JFSMtransitionPrivate.h"
#include"JFSMdiagram.h"
#include"JFSMstate.h"
#include"JFSMtransition.h"
#include"JFSMparameter.h" 
#include"../Guid/GuidCreator.h"
#include"../Reflection/JTypeImplBase.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{ 
			static JFSMconditionPrivate cPrivate;
		}
 
		class JFSMcondition::JFSMconditionImpl : public JTypeImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMconditionImpl)
		public:
			JWeakPtr<JFSMcondition> thisPointer = nullptr;
		public:
			JUserPtr<JFSMtransition> owner;
			REGISTER_PROPERTY_EX(parameter, GetParameter, SetParameter, GUI_SELECTOR(J_GUI_SELECTOR_IMAGE::NONE, false, GetSroageParameter))
			JUserPtr<JFSMparameter> parameter = nullptr;
			REGISTER_PROPERTY_EX(onValue, GetOnValue, SetOnValue,
				GUI_CHECKBOX(GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::BOOL)),
				GUI_FIXED_INPUT(false, J_PARAMETER_TYPE::Int, GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::INT)),
				GUI_FIXED_INPUT(false, J_PARAMETER_TYPE::Float, GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::FLOAT)))
				float onValue = 0;
		public:
			JFSMconditionImpl(const InitData& initData, JFSMcondition* thisCondRaw)
				:owner(initData.ownerTransition)
			{ }
			~JFSMconditionImpl() {}
		public:
			JUserPtr<JFSMparameter> GetParameter()const noexcept
			{
				return parameter;
			}
			float GetOnValue()const noexcept
			{
				if (HasParameter())
					return TypeValue(parameter->GetParamType(), onValue);
				else
					return 0;
			}
			static std::vector<JUserPtr<JIdentifier>> GetSroageParameter(JUserPtr<JIdentifier> iden)noexcept
			{
				if (!iden->GetTypeInfo().IsChildOf< JFSMcondition>())
					return std::vector<JUserPtr<JIdentifier>>{};

				JUserPtr<JFSMcondition> cond = ConvertChildUserPtr<JFSMcondition>(std::move(iden));
				JUserPtr<JFSMdiagram> diagram = cond->GetOwner()->GetInState()->GetOwner();
				std::vector<JUserPtr<JFSMparameter>> paramVec = JFSMdiagramPrivate::ParamInterface::GetStorageParameter(diagram);
				return std::vector<JUserPtr<JIdentifier>>(paramVec.begin(), paramVec.end());
			}
		public:
			void SetParameter(const JUserPtr<JFSMparameter>& newParam)noexcept
			{
				if (newParam == nullptr || IsValidParameter(newParam.Get()))
					parameter = newParam;

				if (parameter == nullptr)
					onValue = 0;
			}
			void SetOnValue(float newValue)noexcept
			{
				if (HasParameter())
					onValue = TypeValue(parameter->GetParamType(), newValue);
			}
		public:
			bool HasParameter()const noexcept
			{
				return parameter != nullptr;
			}
			bool HasSameParameter(const size_t guid)const noexcept
			{
				return HasParameter() ? parameter->GetGuid() == guid : false;
			}
			bool IsSatisfied()const noexcept
			{
				return (PassDefectInspection()) && (parameter->GetValue() == GetOnValue());
			}
			bool IsValidParameter(JFSMparameter* newParam)const noexcept
			{ 
				return newParam != nullptr ? owner->GetInState()->GetOwner()->CanUseParameter(newParam->GetGuid()) : false;
			}
			bool PassDefectInspection()const noexcept
			{
				return parameter != nullptr;
			}
		public:
			bool RegisterInstance()noexcept
			{
				return JFSMtransitionPrivate::OwnTypeInterface::AddCondition(thisPointer);
			}
			bool DeRegisterInstance()noexcept
			{
				return JFSMtransitionPrivate::OwnTypeInterface::RemoveCondition(thisPointer);
			}
		public:
			void Clear()noexcept
			{
				parameter = nullptr;
			}
		public: 
			void RegisterThisPointer(JFSMcondition* cond)
			{
				thisPointer = GetWeakPtr(cond);
			}
			static void RegisterTypeData()
			{
				JIdentifier::RegisterPrivateInterface(JFSMcondition::StaticTypeInfo(),cPrivate);
				IMPL_REALLOC_BIND(JFSMcondition::JFSMconditionImpl, thisPointer)
			}
		};

		JFSMcondition::InitData::InitData(const JUserPtr<JFSMtransition>& ownerTransition)
			:JFSMinterface::InitData(JFSMcondition::StaticTypeInfo()), ownerTransition(ownerTransition)
		{}
		JFSMcondition::InitData::InitData(const std::wstring& name, const size_t guid, const JUserPtr<JFSMtransition>& ownerTransition)
			: JFSMinterface::InitData(JFSMcondition::StaticTypeInfo(), name, guid), ownerTransition(ownerTransition)
		{}
		JFSMcondition::InitData::InitData(const JTypeInfo& initTypeInfo, const JUserPtr<JFSMtransition>& ownerTransition)
			: JFSMinterface::InitData(initTypeInfo), ownerTransition(ownerTransition)
		{}
		JFSMcondition::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, const JUserPtr<JFSMtransition>& ownerTransition)
			: JFSMinterface::InitData(initTypeInfo, name, guid), ownerTransition(ownerTransition)
		{}
		bool JFSMcondition::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && ownerTransition != nullptr;
		}

		JIdentifierPrivate& JFSMcondition::GetPrivateInterface()const noexcept
		{
			return cPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMcondition::GetFSMobjType()const noexcept
		{ 
			return J_FSM_OBJECT_TYPE::CONDITION;
		}
		JUserPtr<JFSMtransition> JFSMcondition::GetOwner()const noexcept
		{
			return impl->owner;
		}
		JUserPtr<JFSMparameter> JFSMcondition::GetParameter()const noexcept
		{
			return impl->GetParameter();
		}
		float JFSMcondition::GetOnValue()const noexcept
		{ 
			return impl->GetOnValue();
		}
		void JFSMcondition::SetParameter(const JUserPtr<JFSMparameter>& newParam)noexcept
		{
			impl->SetParameter(newParam);
		}
		void JFSMcondition::SetOnValue(float newValue)noexcept
		{
			impl->SetOnValue(newValue);
		}
		bool JFSMcondition::HasParameter()const noexcept
		{
			return impl->HasParameter();
		}
		bool JFSMcondition::HasSameParameter(const size_t guid)const noexcept
		{
			return impl->HasSameParameter(guid);
		}
		bool JFSMcondition::IsSatisfied()const noexcept
		{
			return impl->IsSatisfied();
		}
		bool JFSMcondition::PassDefectInspection()const noexcept
		{
			return impl->PassDefectInspection();
		}
		JFSMcondition::JFSMcondition(const InitData& initData)
			:JFSMinterface(initData), impl(std::make_unique<JFSMconditionImpl>(initData, this))
		{}
		JFSMcondition::~JFSMcondition()
		{
			impl.reset();
		}

		using CreateInstanceInterface = JFSMconditionPrivate::CreateInstanceInterface;
		using DestroyInstanceInterface = JFSMconditionPrivate::DestroyInstanceInterface;

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(JDITypeDataBase* initData)
		{
			return JPtrUtil::MakeOwnerPtr<JFSMcondition>(*static_cast<JFSMcondition::InitData*>(initData));
		}
		void CreateInstanceInterface::Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept
		{
			JFSMinterfacePrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
			JFSMcondition* cond = static_cast<JFSMcondition*>(createdPtr);
			cond->impl->RegisterThisPointer(cond); 
		}
		void CreateInstanceInterface::RegisterCash(JIdentifier* createdPtr)noexcept
		{
			static_cast<JFSMcondition*>(createdPtr)->impl->RegisterInstance();
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMcondition::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}

		void DestroyInstanceInterface::Clear(JIdentifier* ptr, const bool isForced)
		{
			JFSMinterfacePrivate::DestroyInstanceInterface::Clear(ptr, isForced);
			static_cast<JFSMcondition*>(ptr)->impl->Clear();
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			static_cast<JFSMcondition*>(ptr)->impl->DeRegisterInstance();
		}

		JIdentifierPrivate::CreateInstanceInterface& JFSMconditionPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		JIdentifierPrivate::DestroyInstanceInterface& JFSMconditionPrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
	}
}