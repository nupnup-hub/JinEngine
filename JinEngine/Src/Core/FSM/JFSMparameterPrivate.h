#pragma once
#include"JFSMinterfacePrivate.h"

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMparameterStorage;
		class JFSMparameterPrivate final: public JFSMinterfacePrivate
		{
		public:
			class CreateInstanceInterface final : public JFSMinterfacePrivate::CreateInstanceInterface
			{
			private:
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)final;
				void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept final;
				void RegisterCash(JIdentifier* createdPtr)noexcept final;
				bool CanCreateInstance(JDITypeDataBase* initData)const noexcept final;
			};
			class DestroyInstanceInterface final : public JIdentifierPrivate::DestroyInstanceInterface
			{
			private:
				void Clear(JIdentifier* ptr, const bool isForced)final;
				void DeRegisterCash(JIdentifier* ptr)noexcept final;
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
			JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		};
	}
}