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
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)override;
			protected:
				void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept override;
			private:
				void RegisterCash(JIdentifier* createdPtr)noexcept override;
				bool CanCreateInstance(JDITypeDataBase* initData)const noexcept override;
			};
			class DestroyInstanceInterface : public JIdentifierPrivate::DestroyInstanceInterface
			{
			protected:
				void Clear(JIdentifier* ptr, const bool isForced)override;
			private:			
				void DeRegisterCash(JIdentifier* ptr)noexcept override;
			};
			class OwnTypeInterface
			{
			private:
				friend class JFSMcondition;
			private:
				static bool AddCondition(const JUserPtr<JFSMcondition>& cond)noexcept;
				static bool RemoveCondition(const JUserPtr<JFSMcondition>& cond)noexcept;
			};
			class UpdateInterface
			{
			private:
				friend class JFSMstate;
			private:
				virtual void Initialize(const JUserPtr<JFSMtransition>& transition)noexcept;
			};
			class ParameterInterface
			{
			private:
				friend class JFSMstate;
			private:
				static void RemoveParameter(const JUserPtr<JFSMtransition>& transition, const size_t guid);
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
			virtual JFSMtransitionPrivate::UpdateInterface& GetUpdateInterface() const noexcept;
		};
	}
}