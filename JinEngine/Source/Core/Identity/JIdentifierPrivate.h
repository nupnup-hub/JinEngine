#pragma once
#include"../Pointer/JOwnerPtr.h"
#include<memory>

namespace JinEngine
{ 
	namespace Core
	{
		class JDITypeDataBase;
		class JIdentifier;
		class JTypeImplBase;
		class JIdenCreatorInterface;
		class JTypeInfo;

		class JIdentifierPrivate
		{
		public:			
			class CreateInstanceInterface
			{
			private:
				friend class JIdentifier;
				friend class JIdenCreatorInterface;
				friend class ReleaseInterface;
			public:
				virtual ~CreateInstanceInterface() = default;
			protected: 
				static JUserPtr<JIdentifier> BeginCreate(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface)noexcept;
				static JUserPtr<JIdentifier> BeginCreateAndCopy(std::unique_ptr<JDITypeDataBase>&& initData, JUserPtr<JIdentifier> from)noexcept;		 
			private:
				virtual JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData) = 0;
				virtual bool CanCreateInstance(JDITypeDataBase* initData)const noexcept; 
			protected:
				virtual void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept;		//Initialize post creation	it is called by child to parent
			private:
				virtual void RegisterCash(JIdentifier* createdPtr)noexcept;				//Register Cash in onwer class or parent class
				virtual void SetValidInstance(JIdentifier* createdPtr)noexcept; 
			private:
				virtual bool Copy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to) = 0;
			protected:
				virtual bool CanCopy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to)noexcept = 0;
			};
			class DestroyInstanceInterface
			{
			private:
				friend class JIdentifier;
				friend class ReleaseInterface;
			public:
				virtual ~DestroyInstanceInterface() = default;
			protected:
				virtual bool CanDestroyInstancce(JIdentifier* ptr, const bool isForced)const noexcept; 
			protected:
				virtual void Clear(JIdentifier* ptr, const bool isForced);					//Clear Objectit is called by child to parent 
			private:
				virtual void DeRegisterCash(JIdentifier* ptr)noexcept;						//DeRegister Instance Cash in onwer class
				virtual void SetInvalidInstance(JIdentifier* ptr)noexcept; 
			}; 
		public:
			virtual ~JIdentifierPrivate() = default;
		public:
			virtual JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept = 0;
			virtual JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept = 0; 
		};
	}
}