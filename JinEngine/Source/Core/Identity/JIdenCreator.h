#pragma once
#include"../DI/JDIDataBase.h"  

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
			static JUserPtr<JIdentifier> Create(std::unique_ptr<JDITypeDataBase>&& initData);
			static JUserPtr<JIdentifier> Create(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface);
			static JUserPtr<JIdentifier> CreateAndCopy(std::unique_ptr<JDITypeDataBase>&& initData, JUserPtr<JIdentifier> from);
		public:
			template<typename T, typename ...Param>
			static JUserPtr<T> Create(Param&&... var)
			{
				return JUserPtr<T>::ConvertChild(JIdenCreatorInterface::Create(std::make_unique<T::InitData>(std::forward<Param>(var)...)));
			}
			template<typename T, typename ...Param>
			static T* CreateRetRaw(Param&&... var)
			{
				return static_cast<T*>(JIdenCreatorInterface::Create(std::make_unique<T::InitData>(std::forward<Param>(var)...)).Get());
			}
		};
	}
	using JICI = Core::JIdenCreatorInterface;
}