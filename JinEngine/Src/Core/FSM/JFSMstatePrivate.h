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
				friend class JFSMtransition;
			private:
				static bool AddTransition(JFSMtransition* transition)noexcept;
				static bool RemoveTransition(JFSMtransition* transition)noexcept;
			};
			class UpdateInterface 
			{
			private:
				friend class JFSMdiagram;
			private:
				virtual void Initialize(JFSMstate* state)noexcept;
			};
			class ParameterInterface
			{
			private:
				friend class JFSMdiagram;
			private:
				static void RemoveParameter(JFSMstate* state, const size_t guid);
			};
		public:
			Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
			virtual JFSMstatePrivate::UpdateInterface& GetUpdateInterface() const noexcept;
		};
	}
}