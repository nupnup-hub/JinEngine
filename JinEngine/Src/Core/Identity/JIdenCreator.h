#pragma once
#include"../DI/JDIDataBase.h" 
#include<string>

namespace JinEngine
{
	class JGameObject;
	class JComponent;
	class JDirectory;
	namespace Core
	{
		class JIdentifier;
		class JIdentifierPrivate;
		class JIdenCreatorInterface
		{
		public:
			static Core::JUserPtr<JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData);
			static Core::JUserPtr<JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface);
			static Core::JUserPtr<JIdentifier> CreateAndCopy(std::unique_ptr<Core::JDITypeDataBase>&& initData, JIdentifier* from);
		public:
			static Core::JUserPtr<JIdentifier> CreateAndCopy(JGameObject* from, JGameObject* toParent);
			static Core::JUserPtr<JIdentifier> CreateAndCopy(JComponent* from, JGameObject* toOwner);
			static Core::JUserPtr<JIdentifier> CreateAndCopy(JDirectory* from, JDirectory* toParent);
		public:
			template<typename T, typename ...Param>
			static T* Create(Param&&... var)
			{
				return static_cast<T*>(JIdenCreatorInterface::Create(std::make_unique<T::InitData>(std::forward<Param>(var)...)).Get());
			}
			template<typename T, typename ...Param>
			static Core::JUserPtr<T> CreateRetUser(Param&&... var)
			{
				return Core::JUserPtr<T>::ConvertChildUser(JIdenCreatorInterface::Create(std::make_unique<T::InitData>(std::forward<Param>(var)...)));
			}
		};
	}
	using JICI = Core::JIdenCreatorInterface;
}