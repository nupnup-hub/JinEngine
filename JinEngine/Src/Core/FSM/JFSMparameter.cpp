#include"JFSMparameter.h" 
#include"JFSMparameterPrivate.h" 
#include"JFSMparameterStorage.h" 
#include"../Identity/JIdentifierImplBase.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			static JFSMparameterPrivate pPrivate;
		}
 
		class JFSMparameter::JFSMparameterImpl : public JIdentifierImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMparameterImpl)
			REGISTER_GUI_ENUM_CONDITION(ParameterType, J_FSM_PARAMETER_VALUE_TYPE, paramType)
		public:
			JFSMparameter* thisParam;
		public:
			REGISTER_PROPERTY_EX(paramType, GetParamType, SetParamType)
			JFSMparameterStorageInterface* paramStorage = nullptr;
			J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL;
			float value = 0;
		public:
			JFSMparameterImpl(const InitData& initData, JFSMparameter* thisParam)
				:paramStorage(initData.paramStorage), thisParam(thisParam)
			{ }
			~JFSMparameterImpl() {}
		public:
			float GetValue()const noexcept
			{
				return TypeValue(paramType, value);
			}
			J_FSM_PARAMETER_VALUE_TYPE GetParamType()const noexcept
			{
				return paramType;
			}
		public:
			template<typename T>
			void SetValue(T newValue)noexcept
			{
				value = TypeValue(paramType, newValue);
			}
			void SetParamType(const J_FSM_PARAMETER_VALUE_TYPE newParamType)noexcept
			{
				paramType = newParamType;
				thisParam->SetValue(value);
			}
		public:
			bool RegisterInstance()noexcept
			{
				return paramStorage->AddParameter(thisParam);
			}
			bool DeRegisterInstance()noexcept
			{
				return paramStorage->RemoveParameter(thisParam);
			}
		public:
			void Clear()noexcept
			{}
		public:
			static void RegisterCallOnce()
			{
				Core::JIdentifier::RegisterPrivateInterface(JFSMparameter::StaticTypeInfo(), pPrivate);
			}
		};

		JFSMparameter::InitData::InitData(JFSMparameterStorageInterface* paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType)
			: JFSMinterface::InitData(initTypeInfo), paramStorage(paramStorage), paramType(paramType)
		{}
		JFSMparameter::InitData::InitData(const std::wstring& name, const size_t guid, JFSMparameterStorageInterface* paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType)
			: JFSMinterface::InitData(initTypeInfo, name, guid), paramStorage(paramStorage), paramType(paramType)
		{}
		bool JFSMparameter::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && paramStorage != nullptr;

		}
 
		Core::JIdentifierPrivate& JFSMparameter::GetPrivateInterface()const noexcept
		{
			return pPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMparameter::GetFSMobjType()const noexcept
		{ 
			return J_FSM_OBJECT_TYPE::PARAMETER;
		}
		float JFSMparameter::GetValue()const noexcept
		{
			return impl->GetValue();
		}
		J_FSM_PARAMETER_VALUE_TYPE JFSMparameter::GetParamType()const noexcept
		{
			return impl->GetParamType();
		} 
		void JFSMparameter::SetValue(const float value)noexcept
		{
			impl->SetValue(value);
		}
		void JFSMparameter::SetValue(const int value)noexcept
		{
			impl->SetValue(value);
		}
		void JFSMparameter::SetValue(const bool value)noexcept
		{
			impl->SetValue(value);
		}
		void JFSMparameter::SetParamType(const J_FSM_PARAMETER_VALUE_TYPE paramType)noexcept
		{
			impl->SetParamType(paramType);
		} 
		JFSMparameter::JFSMparameter(const InitData& initData)
			:JFSMinterface(initData), impl(std::make_unique<JFSMparameterImpl>(initData, this))
		{}
		JFSMparameter::~JFSMparameter()
		{
			impl.reset();
		}

		using CreateInstanceInterface = JFSMparameterPrivate::CreateInstanceInterface;
		using DestroyInstanceInterface = JFSMparameterPrivate::DestroyInstanceInterface;

		Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
		{
			return Core::JPtrUtil::MakeOwnerPtr<JFSMparameter>(*static_cast<JFSMparameter::InitData*>(initData.get()));
		}
		bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMparameter::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}
		void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
		{
			static_cast<JFSMparameter*>(createdPtr)->impl->RegisterInstance();
		}

		void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
		{
			static_cast<JFSMparameter*>(ptr)->impl->Clear();
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			static_cast<JFSMparameter*>(ptr)->impl->DeRegisterInstance();
		}

		Core::JIdentifierPrivate::CreateInstanceInterface& JFSMparameterPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		Core::JIdentifierPrivate::DestroyInstanceInterface& JFSMparameterPrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
	}
}