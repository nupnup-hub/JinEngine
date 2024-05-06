/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JFSMparameter.h" 
#include"JFSMparameterPrivate.h" 
#include"JFSMparameterStorage.h" 
#include"../Reflection/JTypeImplBase.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			using OwnTypeInterface = JFSMparameterStoragePrivate::OwnTypeInterface;
		}
		namespace
		{
			static JFSMparameterPrivate pPrivate;
		}
 
		class JFSMparameter::JFSMparameterImpl : public JTypeImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMparameterImpl)
			REGISTER_GUI_ENUM_CONDITION(ParameterType, J_FSM_PARAMETER_VALUE_TYPE, paramType, false)
		public:
			JWeakPtr<JFSMparameter> thisPointer;
		public:
			REGISTER_PROPERTY_EX(paramType, GetParamType, SetParamType)
			JUserPtr<JFSMparameterStoragePublicAccess> paramStorage = nullptr;
			J_FSM_PARAMETER_VALUE_TYPE paramType = J_FSM_PARAMETER_VALUE_TYPE::BOOL;
			float value = 0;
		public:
			JFSMparameterImpl(const InitData& initData, JFSMparameter* thisParamRaw)
				:paramStorage(initData.paramStorage)
			{}
			~JFSMparameterImpl() {}
		public:
			float GetValue()const noexcept
			{
				return FsmParamTypeValue(paramType, value);
			}
			J_FSM_PARAMETER_VALUE_TYPE GetParamType()const noexcept
			{
				return paramType;
			}
		public:
			template<typename T>
			void SetValue(T newValue)noexcept
			{
				value = FsmParamTypeValue(paramType, newValue);
			}
			void SetParamType(const J_FSM_PARAMETER_VALUE_TYPE newParamType)noexcept
			{
				paramType = newParamType;
				thisPointer->SetValue(value);
			}
		public:
			bool RegisterInstance()noexcept
			{
				return OwnTypeInterface::AddParameter(paramStorage, thisPointer);
			}
			bool DeRegisterInstance()noexcept
			{
				return OwnTypeInterface::RemoveParameter(paramStorage, thisPointer);
			}
		public:
			void Clear()noexcept
			{}
		public:
			void RegisterThisPointer(JFSMparameter* param)
			{
				thisPointer = Core::GetWeakPtr(param);
			}
			static void RegisterTypeData()
			{
				JIdentifier::RegisterPrivateInterface(JFSMparameter::StaticTypeInfo(), pPrivate);
				IMPL_REALLOC_BIND(JFSMparameter::JFSMparameterImpl, thisPointer)
			}
		};

		JFSMparameter::InitData::InitData(const JUserPtr<JFSMparameterStoragePublicAccess>& paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType)
			: JFSMinterface::InitData(JFSMparameter::StaticTypeInfo()), paramStorage(paramStorage), paramType(paramType)
		{}
		JFSMparameter::InitData::InitData(const std::wstring& name, const size_t guid, const JUserPtr<JFSMparameterStoragePublicAccess>& paramStorage, const J_FSM_PARAMETER_VALUE_TYPE paramType)
			: JFSMinterface::InitData(JFSMparameter::StaticTypeInfo(), name, guid), paramStorage(paramStorage), paramType(paramType)
		{}
		bool JFSMparameter::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && paramStorage != nullptr;
		}
 
		JIdentifierPrivate& JFSMparameter::PrivateInterface()const noexcept
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
		bool JFSMparameter::IsStorageParameter(const size_t storageGuid)const noexcept
		{
			return impl->paramStorage->GetGuid() == storageGuid;
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

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(JDITypeDataBase* initData)
		{
			return JPtrUtil::MakeOwnerPtr<JFSMparameter>(*static_cast<JFSMparameter::InitData*>(initData));
		}
		void CreateInstanceInterface::Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept
		{
			JIdentifierPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
			JFSMparameter* param = static_cast<JFSMparameter*>(createdPtr);
			param->impl->RegisterThisPointer(param);
		}
		void CreateInstanceInterface::RegisterCash(JIdentifier* createdPtr)noexcept
		{
			static_cast<JFSMparameter*>(createdPtr)->impl->RegisterInstance();
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMparameter::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}

		void DestroyInstanceInterface::Clear(JIdentifier* ptr, const bool isForced)
		{
			static_cast<JFSMparameter*>(ptr)->impl->Clear();
			JFSMinterfacePrivate::DestroyInstanceInterface::Clear(ptr, isForced);
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			static_cast<JFSMparameter*>(ptr)->impl->DeRegisterInstance();
		}
 
		JIdentifierPrivate::CreateInstanceInterface& JFSMparameterPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		JIdentifierPrivate::DestroyInstanceInterface& JFSMparameterPrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
	}
}