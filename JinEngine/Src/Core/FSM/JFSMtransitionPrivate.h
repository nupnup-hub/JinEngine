#pragma once
#include"JFSMinterfacePrivate.h"

namespace JinEngine
{
	namespace Core
	{ 
		class JFSMstate;
		class JFSMtransition;
		class JFSMcondition;
		class JFSMtransitionPrivate : public JFSMinterfacePrivate
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
			class OwnTypeInterface
			{
			private:
				friend class JFSMcondition;
			private:
				static bool AddCondition(JFSMcondition* cond)noexcept;
				static bool RemoveCondition(JFSMcondition* cond)noexcept;
			};
			class UpdateInterface
			{
			private:
				friend class JFSMstate;
			private:
				virtual void Initialize(JFSMtransition* transition)noexcept;
			};
			class ParameterInterface
			{
			private:
				friend class JFSMstate;
			private:
				static void RemoveParameter(JFSMtransition* state, const size_t guid);
			};
		public:
			Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
			virtual JFSMtransitionPrivate::UpdateInterface& GetUpdateInterface() const noexcept;
		};
	}
}