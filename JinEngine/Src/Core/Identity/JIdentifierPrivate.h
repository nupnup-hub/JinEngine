#pragma once
#include"../Pointer/JOwnerPtr.h"
#include<memory>

namespace JinEngine
{
	namespace Editor
	{
		class JEditorObjectReleaseInterface;
	}
	namespace Core
	{
		class JDITypeDataBase;
		class JIdentifier;
		class JIdentifierImplBase;
		class JIdenCreatorInterface;
		class JTypeInfo;

		class JIdentifierPrivate
		{
		public:			
			class ReleaseInterface
			{
			private:
				friend class Editor::JEditorObjectReleaseInterface;
			private:
				static Core::JOwnerPtr<JIdentifier> ReleaseInstance(JIdentifier* ptr);	//release ownerPtr in typeInfo
				static bool RestoreInstance(Core::JOwnerPtr<JIdentifier>&& instance);
			};		
			class CreateInstanceInterface
			{
			private:
				friend class JIdentifier;
				friend class JIdenCreatorInterface;
				friend class ReleaseInterface;
			public:
				virtual ~CreateInstanceInterface() = default;
			protected: 
				static JIdentifier* BeginCreate(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface)noexcept;
				static JIdentifier* BeginCreateAndCopy(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifier* from)noexcept;
			private:
				virtual Core::JOwnerPtr<JIdentifier> Create(std::unique_ptr<JDITypeDataBase>&& initData) = 0;
				virtual bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept;
				virtual void PrepareCreateInstance()noexcept;
				virtual void RegisterCash(JIdentifier* createdPtr)noexcept;				//Register Cash in onwer class or parent class
				virtual void SetValidInstance(JIdentifier* createdPtr)noexcept;
				static bool AddInstance(Core::JOwnerPtr<JIdentifier>&& createdOwner)noexcept;	//Add Instance in typeinfo 
			private:
				virtual bool Copy(JIdentifier* from, JIdentifier* to) = 0;
			protected:
				virtual bool CanCopy(JIdentifier* from, JIdentifier* to)noexcept = 0;	 
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
			private:
				virtual void Clear(JIdentifier* ptr, const bool isForced) = 0;				//Clear Object	
				virtual void PrepareDestroyInstance(JIdentifier* ptr)noexcept;			 		
				virtual void DeRegisterCash(JIdentifier* ptr)noexcept;						//DeRegister Cash in onwer class
				virtual void SetInvalidInstance(JIdentifier* ptr)noexcept;
			private:
				static bool RemoveInstance(JIdentifier* ptr)noexcept;	//Remove Instance in typeinfo 
			}; 
		public:
			virtual ~JIdentifierPrivate() = default;
		public:
			virtual JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept = 0;
			virtual JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept = 0; 
		};
	}
}