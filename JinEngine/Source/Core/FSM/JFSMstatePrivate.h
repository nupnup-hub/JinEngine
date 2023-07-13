#pragma once
#include"JFSMinterfacePrivate.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;
		class JFSMstate;
		class JFSMtransition;
		class JFSMstatePrivate : public JFSMinterfacePrivate
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
				friend class JFSMtransition;
			private:
				static bool AddTransition(const JUserPtr<JFSMtransition>& transition)noexcept;
				static bool RemoveTransition(const JUserPtr<JFSMtransition>& transition)noexcept;
			};
			class UpdateInterface 
			{
			private:
				friend class JFSMdiagram;
			private:
				virtual void Initialize(const JUserPtr<JFSMstate>& state)noexcept;
			};
			class ParameterInterface
			{
			private:
				friend class JFSMdiagram;
			private:
				static void RemoveParameter(const JUserPtr<JFSMstate>& state, const size_t guid);
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
			virtual JFSMstatePrivate::UpdateInterface& GetUpdateInterface() const noexcept;
		};
	}
}