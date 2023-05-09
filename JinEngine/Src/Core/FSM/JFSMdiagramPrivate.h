#pragma once
#include"JFSMinterfacePrivate.h"
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		class JFSMstate; 
		class JFSMparameter;
		class JFSMcondition;
		class JFSMdiagram;
		class JFSMdiagramOwnerInterface;
		class JFSMdiagramPrivate : public JFSMinterfacePrivate
		{
		public:
			class CreateInstanceInterface : public JFSMinterfacePrivate::CreateInstanceInterface
			{ 
			private:
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)override;
			protected:
				void Initialize (JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept override;
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
				friend class JFSMstate;
			private:
				static bool AddState(const JUserPtr<JFSMstate>& state)noexcept;
				static bool RemoveState(const JUserPtr<JFSMstate>& state)noexcept;
			};
			class OwnerTypeInterface
			{
			private:
				friend class JFSMdiagramOwnerInterface;
			private:
				static void SetOwnerPointer(const JUserPtr<JFSMdiagram>& digram, JFSMdiagramOwnerInterface* ownInterface)noexcept;
			};
			class ParamInterface
			{
			private:
				friend class JFSMcondition;
			private:
				static std::vector<JUserPtr<JFSMparameter>> GetStorageParameter(const JUserPtr<JFSMdiagram>& diagram)noexcept;
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept override;
			JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept override;
		};
	}
}