#pragma once 
#include"../JObjectFlag.h" 
#include"../../Core/Factory/JFactory.h"
#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{
	class JComponent;
	class JGameObject;
	class JComponentFactoryImplBase;
	template<typename T> class JComponentFactoryImpl;

	namespace
	{
		class JComponentFactory
		{
		private:
			friend class JComponentFactoryImplBase;
			template<typename T> friend class JComponentFactoryImpl;
		private:
			Core::JFactory<std::string, false, JComponent*, JGameObject*> defaultFactory;
			Core::JFactory<std::string, false, JComponent*, const size_t, const JOBJECT_FLAG, JGameObject*> initFactory;
			Core::JFactory<std::string, false, JComponent*, std::wifstream&, JGameObject*> loadFactory;
			Core::JFactory<std::string, false, JComponent*, JComponent*, JGameObject*> copyFactory;
		private:
			template<typename Type>
			bool Register(Core::JCallableInterface<JComponent*, JGameObject*>* callable)
			{
				if constexpr (std::is_base_of_v <JComponent, Type >)
					return defaultFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}
			template<typename Type>
			bool Register(Core::JCallableInterface<JComponent*, const size_t, const JOBJECT_FLAG, JGameObject*>* callable)
			{
				if constexpr (std::is_base_of_v <JComponent, Type >)
					return initFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}
			template<typename Type>
			bool Register(Core::JCallableInterface<JComponent*, std::wifstream&, JGameObject*>* callable)
			{
				if constexpr (std::is_base_of_v <JComponent, Type >)
					return loadFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}
			template<typename Type>
			bool Register(Core::JCallableInterface<JComponent*, JComponent*, JGameObject*>* callable)
			{
				if constexpr (std::is_base_of_v <JComponent, Type >)
					return copyFactory.Regist(Type::TypeName(), callable);
				else
					return false;
			}

			JComponent* Create(const std::string& typeName, JGameObject& owner)
			{
				return defaultFactory.Invoke(typeName, &owner);
			}
			JComponent* Load(const std::string& typeName, std::wifstream& stream, JGameObject& owner)
			{
				return loadFactory.Invoke(typeName, stream, &owner);
			}
			JComponent* Copy(const std::string& typeName, JComponent& ori, JGameObject& owner)
			{
				return copyFactory.Invoke(typeName, &ori, &owner);
			}

			template<typename Type>
			Type* Create(JGameObject& owner)
			{
				return static_cast<Type*>(defaultFactory.Invoke(Type::TypeName(), &owner));
			}
			template<typename Type>
			Type* Create(size_t guid, JOBJECT_FLAG flag, JGameObject& owner)
			{
				return static_cast<Type*>(initFactory.Invoke(Type::TypeName(), std::move(guid), std::move(flag), &owner));
			}
			template<typename Type>
			Type* Load(std::wifstream& stream, JGameObject& owner)
			{
				return static_cast<Type*>(loadFactory.Invoke(Type::TypeName(), stream, &owner));
			}
			template<typename Type>
			Type* Copy(Type& ori, JGameObject& owner)
			{
				return static_cast<Type*>(copyFactory.Invoke(Type::TypeName(), &ori, &owner));
			}
		};
	}
	 
	using JCF = Core::JSingletonHolder<JComponentFactory>;

	class JComponentFactoryImplBase
	{
	private:
		friend class JGameObject;
	protected:
		using DefaultPtr = JComponent * (*)(JGameObject*);
		using InitPtr = JComponent * (*)(const size_t, const JOBJECT_FLAG, JGameObject*);
		using LoadPtr = JComponent * (*)(std::wifstream&, JGameObject*);
		using CopytPtr = JComponent * (*)(JComponent*, JGameObject*);

		using DefaultCallable = Core::JStaticCallable<JComponent*, JGameObject*>;
		using InitCallable = Core::JStaticCallable<JComponent*, const size_t, const JOBJECT_FLAG, JGameObject*>;
		using LoadCallable = Core::JStaticCallable<JComponent*, std::wifstream&, JGameObject*>;
		using CopyCallable = Core::JStaticCallable<JComponent*, JComponent*, JGameObject*>;
	private:
		using AddStoragePtr = JComponent * (JGameObject::*)(JComponent*);
		using AddStorageCallable = Core::JMemeberCallable<JGameObject, JComponent*, JComponent*>;
	protected:
		static AddStorageCallable* addStorage;
	public:
		static JComponent* CreateByName(const std::string& typeName, JGameObject& owner)
		{
			JComponent* res = JCF::Instance().Create(typeName, owner);
			(*addStorage)(&owner, &*res);
			return res;
		}
	public:
		static JComponent* CopyByName(const std::string& typeName, JComponent& ori, JGameObject& owner)
		{
			JComponent* res = JCF::Instance().Copy(typeName, ori, owner);
			(*addStorage)(&owner, &*res);
			return res;
		}
	protected:
		static JComponent* LoadByName(const std::string& typeName, std::wifstream& stream, JGameObject& owner)
		{
			JComponent* res = JCF::Instance().Load(typeName, stream, owner);
			(*addStorage)(&owner, &*res);
			return res;
		}
	private:
		static void RegistAddStroage(AddStoragePtr addPtr)
		{
			static AddStorageCallable addStorage{ addPtr };
			JComponentFactoryImplBase::addStorage = &addStorage;
		}
	};

	template<typename T>
	class JComponentFactoryImpl : public JComponentFactoryImplBase
	{
	private:
		friend T;
		friend class JGameObject;
	public:
		static T* Create(JGameObject& owner)
		{
			T* res = JCF::Instance().Create<T>(owner);
			(*addStorage)(&owner, &*res);
			return res;
		}
		static T* Create(const size_t guid, const JOBJECT_FLAG flag, JGameObject& owner)
		{
			T* res = JCF::Instance().Create<T>(guid, flag, owner);
			(*addStorage)(&owner, &*res);
			return res;
		}
		static T* Copy(T& ori, JGameObject& owner)
		{
			T* res = JCF::Instance().Copy<T>(ori, owner);
			(*addStorage)(&owner, &*res);
			return res;
		}
	private:
		static T* Load(std::wifstream& stream, JGameObject& owner)
		{
			T* res = JCF::Instance().Load<T>(stream, owner);
			(*addStorage)(&owner, &*res);
			return res;
		}
	private:
		static void Regist(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr)
		{
			static DefaultCallable defaultCallable{ defaultPtr };
			static InitCallable initCallable{ initPtr };
			static LoadCallable loadCallable{ loadPtr };
			static CopyCallable copyCallable{ copytPtr };

			JCF::Instance().Register<T>(&defaultCallable);
			JCF::Instance().Register<T>(&initCallable);
			JCF::Instance().Register<T>(&loadCallable);
			JCF::Instance().Register<T>(&copyCallable);
		}
	};

	using JCFIB = JComponentFactoryImplBase;
	template<typename T>
	using JCFI = JComponentFactoryImpl<T>;
}