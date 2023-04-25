#include"JFSMcondition.h"
#include"JFSMconditionPrivate.h"
#include"JFSMdiagramPrivate.h"
#include"JFSMtransitionPrivate.h"
#include"JFSMdiagram.h"
#include"JFSMstate.h"
#include"JFSMtransition.h"
#include"JFSMparameter.h" 
#include"../Guid/GuidCreator.h"
#include"../Identity/JIdentifierImplBase.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{ 
			static JFSMconditionPrivate cPrivate;
		}
 
		class JFSMcondition::JFSMconditionImpl : public JIdentifierImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMconditionImpl)
		public:
			JFSMtransition* owner;
			REGISTER_PROPERTY_EX(parameter, GetParameter, SetParameter, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false, GetSroageParameter))
			JFSMparameter* parameter = nullptr;
			REGISTER_PROPERTY_EX(onValue, GetOnValue, SetOnValue,
				GUI_CHECKBOX(GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::BOOL)),
				GUI_FIXED_INPUT(false, Core::J_PARAMETER_TYPE::Int, GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::INT)),
				GUI_FIXED_INPUT(false, Core::J_PARAMETER_TYPE::Float, GUI_ENUM_CONDITION_REF_USER(ParameterType, parameter, J_FSM_PARAMETER_VALUE_TYPE::FLOAT)))
				float onValue = 0;
		public:
			JFSMconditionImpl(const InitData& initData, JFSMcondition* thisCond)
				:owner(initData.ownerTransition.Get())
			{ }
			~JFSMconditionImpl() {}
		public:
			JFSMparameter* GetParameter()const noexcept
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
			static std::vector<JIdentifier*> GetSroageParameter(JIdentifier* iden)noexcept
			{
				if (!iden->GetTypeInfo().IsChildOf< JFSMcondition>())
					return std::vector<JIdentifier*>{};

				JFSMdiagram* diagram = static_cast<JFSMcondition*>(iden)->GetOwner()->GetOwner()->GetOwner();
				std::vector<JFSMparameter*> paramVec = JFSMdiagramPrivate::ParamInterface::GetStorageParameter(diagram);
				return std::vector<JIdentifier*>(paramVec.begin(), paramVec.end());
			}
		public:
			void SetParameter(JFSMparameter* newParam)noexcept
			{
				if (newParam == nullptr || IsValidParameter(newParam))
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
				JFSMdiagram* diagram = owner->GetOwner()->GetOwner();
				return newParam != nullptr ? diagram->CanUseParameter(newParam->GetGuid()) : false;
			}
			bool PassDefectInspection()const noexcept
			{
				return parameter != nullptr;
			}
		public:
			static bool RegisterInstance(JFSMcondition* cond)noexcept
			{
				return JFSMtransitionPrivate::OwnTypeInterface::AddCondition(cond);
			}
			static bool DeRegisterInstance(JFSMcondition* cond)noexcept
			{
				return JFSMtransitionPrivate::OwnTypeInterface::RemoveCondition(cond);
			}
		public:
			void Clear()noexcept
			{
				parameter = nullptr;
			}
		public:
			static void RegisterCallOnce()
			{
				Core::JIdentifier::RegisterPrivateInterface(JFSMcondition::StaticTypeInfo(),cPrivate);
			}
		};

		JFSMcondition::InitData::InitData(JUserPtr<JFSMtransition> ownerTransition)
			:JFSMinterface::InitData(JFSMcondition::StaticTypeInfo()), ownerTransition(ownerTransition)
		{}
		JFSMcondition::InitData::InitData(const std::wstring& name, const size_t guid, JUserPtr<JFSMtransition> ownerTransition)
			: JFSMinterface::InitData(JFSMcondition::StaticTypeInfo(), name, guid), ownerTransition(ownerTransition)
		{}
		JFSMcondition::InitData::InitData(const JTypeInfo& initTypeInfo, JUserPtr<JFSMtransition> ownerTransition)
			: JFSMinterface::InitData(initTypeInfo), ownerTransition(ownerTransition)
		{}
		JFSMcondition::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, JUserPtr<JFSMtransition> ownerTransition)
			: JFSMinterface::InitData(initTypeInfo, name, guid), ownerTransition(ownerTransition)
		{}
		bool JFSMcondition::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && ownerTransition != nullptr;
		}

		Core::JIdentifierPrivate& JFSMcondition::GetPrivateInterface()const noexcept
		{
			return cPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMcondition::GetFSMobjType()const noexcept
		{ 
			return J_FSM_OBJECT_TYPE::CONDITION;
		}
		JFSMtransition* JFSMcondition::GetOwner()const noexcept
		{
			return impl->owner;
		}
		JFSMparameter* JFSMcondition::GetParameter()const noexcept
		{
			return impl->GetParameter();
		}
		float JFSMcondition::GetOnValue()const noexcept
		{ 
			return impl->GetOnValue();
		}
		void JFSMcondition::SetParameter(JFSMparameter* newParam)noexcept
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

		Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
		{
			return Core::JPtrUtil::MakeOwnerPtr<JFSMcondition>(*static_cast<JFSMcondition::InitData*>(initData.get()));
		}
		bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMcondition::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}
		void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
		{
			JFSMcondition::JFSMconditionImpl::RegisterInstance(static_cast<JFSMcondition*>(createdPtr));
		}

		void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
		{
			static_cast<JFSMcondition*>(ptr)->impl->Clear();
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			JFSMcondition::JFSMconditionImpl::DeRegisterInstance(static_cast<JFSMcondition*>(ptr));
		}

		Core::JIdentifierPrivate::CreateInstanceInterface& JFSMconditionPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		Core::JIdentifierPrivate::DestroyInstanceInterface& JFSMconditionPrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
	}
}