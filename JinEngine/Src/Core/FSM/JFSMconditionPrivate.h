#pragma once
#include"JFSMinterfacePrivate.h"

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMtransition;
		class JFSMconditionPrivate : public JFSMinterfacePrivate
		{
		public:
			class CreateInstanceInterface : public JFSMinterfacePrivate::CreateInstanceInterface
			{
			private:
				Core::JOwnerPtr<JIdentifier> Create(std::unique_ptr<JDITypeDataBase>&& initData)override;
				bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept override;
				void RegisterCash(Core::JIdentifier* createdPtr)noexcept override;
			};
			class DestroyInstanceInterface : public Core::JIdentifierPrivate::DestroyInstanceInterface
			{
			private:
				void Clear(JIdentifier* ptr, const bool isForced)override;
				void DeRegisterCash(JIdentifier* ptr)noexcept override;
			};
		public:
			Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		};
	}
}