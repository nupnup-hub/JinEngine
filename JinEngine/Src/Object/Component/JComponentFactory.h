#pragma once 
#include"JComponent.h"
#include"../JObjectFlag.h" 
#include"../../Core/Factory/JFactory.h"
#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{
	class JComponent;
	class JGameObject;
	class JComponentFactoryImplBase;
	template<typename T> class JComponentFactoryImpl;


	class JComponentFactory
	{
	private:
		friend class JComponentFactoryImplBase;
		template<typename T> friend class JComponentFactoryImpl;
	private:
		Core::JFactory<std::string, false, JComponent*, JGameObject*> defaultFactory;
		Core::JFactory<std::string, false, JComponent*, const size_t, const J_OBJECT_FLAG, JGameObject*> initFactory;
		Core::JFactory<std::string, false, JComponent*, std::wifstream&, JGameObject*> loadFactory;
		Core::JFactory<std::string, false, JComponent*, JComponent*, JGameObject*> copyFactory;
	private:
		bool Register(const std::string& iden, Core::JCallableInterface<JComponent*, JGameObject*>* callable);
		bool Register(const std::string& iden, Core::JCallableInterface<JComponent*, const size_t, const J_OBJECT_FLAG, JGameObject*>* callable);
		bool Register(const std::string& iden, Core::JCallableInterface<JComponent*, std::wifstream&, JGameObject*>* callable);
		bool Register(const std::string& iden, Core::JCallableInterface<JComponent*, JComponent*, JGameObject*>* callable);

		JComponent* Create(const std::string& typeName, JGameObject& owner);
		JComponent* Create(const std::string& typeName, const size_t guid, const J_OBJECT_FLAG flag, JGameObject& owner);
		JComponent* Load(const std::string& typeName, std::wifstream& stream, JGameObject& owner);
		JComponent* Copy(const std::string& typeName, JComponent& ori, JGameObject& owner);
	};

	using JCF = Core::JSingletonHolder<JComponentFactory>;

	class JComponentFactoryImplBase
	{
	private:
		friend class JComponent;
	protected:
		using Default = Core::JStaticCallableType<JComponent*, JGameObject*>;
		using Init = Core::JStaticCallableType<JComponent*, const size_t, const J_OBJECT_FLAG, JGameObject*>;
		using Load = Core::JStaticCallableType<JComponent*, std::wifstream&, JGameObject*>;
		using Copy = Core::JStaticCallableType<JComponent*, JComponent*, JGameObject*>;
	public:
		static JComponent* CreateByName(const std::string& typeName, JGameObject& owner)
		{
			return JCF::Instance().Create(typeName, owner);
		}
	public:
		static JComponent* CopyByName(JComponent& ori, JGameObject& owner)
		{
			return JCF::Instance().Copy(ori.GetTypeInfo().Name(), ori, owner);
		}
	protected:
		static JComponent* LoadByName(const std::string& typeName, std::wifstream& stream, JGameObject& owner)
		{
			return JCF::Instance().Load(typeName, stream, owner);
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
			return static_cast<T*>(JCF::Instance().Create(T::TypeName(), owner));
		}
		static T* Create(const size_t guid, JGameObject& owner)
		{ 
			return static_cast<T*>(JCF::Instance().Create(T::TypeName(), guid, OBJECT_FLAG_NONE, owner));
		}
		static T* Create(const size_t guid, const J_OBJECT_FLAG addedFlag, JGameObject& owner)
		{
			return static_cast<T*>(JCF::Instance().Create(T::TypeName(), guid, addedFlag, owner));
		}
		static T* Copy(T& ori, JGameObject& owner)
		{
			return static_cast<T*>(JCF::Instance().Copy(T::TypeName(), ori, owner));
		}
	private:
		static T* Load(std::wifstream& stream, JGameObject& owner)
		{
			return static_cast<T*>(JCF::Instance().Load(T::TypeName(), stream, owner));
		}
	private:
		static void Register(Default::Ptr defaultPtr, Init::Ptr initPtr, Load::Ptr loadPtr, Copy::Ptr copytPtr)
		{
			static Default::Callable defaultCallable{ defaultPtr };
			static Init::Callable initCallable{ initPtr };
			static Load::Callable loadCallable{ loadPtr };
			static Copy::Callable copyCallable{ copytPtr };

			JCF::Instance().Register(T::TypeName(), &defaultCallable);
			JCF::Instance().Register(T::TypeName(), &initCallable);
			JCF::Instance().Register(T::TypeName(), &loadCallable);
			JCF::Instance().Register(T::TypeName(), &copyCallable);
		}
	};

	using JCFIB = JComponentFactoryImplBase;
	template<typename T>
	using JCFI = JComponentFactoryImpl<T>;
}