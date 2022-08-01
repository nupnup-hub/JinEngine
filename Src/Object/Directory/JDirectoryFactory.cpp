#include"JDirectoryFactory.h"
#include"JDirectory.h"
#include"../../Core/Factory/JFactory.h"
#include"../Resource/JResourceManager.h"

namespace JinEngine
{ 
	class JDirectoryFactory
	{
	private:
		friend class JDirectoryFactoryImpl;
	private:
		Core::JFactory<std::string, false, JDirectory*, JDirectory*> defaultFactory;
		Core::JFactory<std::string, false, JDirectory*, const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*> initFactory;
		Core::JFactory<std::string, false, JDirectory*, JDirectory*> copyFactory;
	private:
		bool Register(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*>* callable)
		{
			return defaultFactory.Regist(iden, callable);
		}
		bool Register(const std::string& iden, Core::JCallableInterface<JDirectory*, const std::string&, const size_t, const JOBJECT_FLAG, JDirectory*>* callable)
		{
			return initFactory.Regist(iden, callable);
		}
		bool RegisterCopy(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*>* callable)
		{
			return copyFactory.Regist(iden, callable);
		}

		JDirectory* Create(JDirectory& parent)
		{
			return static_cast<JDirectory*>(defaultFactory.Invoke(JDirectory::TypeName(), &parent));
		}
		JDirectory* CreateRoot(const std::string& name, size_t guid, JOBJECT_FLAG flag)
		{
			return static_cast<JDirectory*>(initFactory.Invoke(JDirectory::TypeName(), name, std::move(guid), std::move(flag), nullptr));
		}
		JDirectory* Create(const std::string& name, size_t guid, JOBJECT_FLAG flag, JDirectory& parent)
		{
			return static_cast<JDirectory*>(initFactory.Invoke(JDirectory::TypeName(), name, std::move(guid), std::move(flag), &parent));
		}
		JDirectory* Copy(JDirectory& ori)
		{
			return static_cast<JDirectory*>(copyFactory.Invoke(JDirectory::TypeName(), &ori));
		}
	};
	using JDF = Core::JSingletonHolder<JDirectoryFactory>;

	JDirectoryFactoryImpl::AddStorageCallable* JDirectoryFactoryImpl::addStorage;

	JDirectory* JDirectoryFactoryImpl::Create(JDirectory& parent)
	{
		JDirectory* res = JDF::Instance().Create(parent);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), &*res);
		return res;
	} 
	JDirectory* JDirectoryFactoryImpl::CreateRoot(const std::string& name, const size_t guid, const JOBJECT_FLAG flag)
	{
		JDirectory* res = JDF::Instance().CreateRoot(name, guid, flag);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), &*res);
		return res;
	}
	JDirectory* JDirectoryFactoryImpl::Create(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory& parent)
	{
		JDirectory* res = JDF::Instance().Create(name, guid, flag, parent);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), &*res);
		return res;
	}
	void JDirectoryFactoryImpl::Regist(DefaultPtr defaultPtr, InitPtr initPtr)
	{
		static DefaultCallable defaultCallable{ defaultPtr };
		static InitCallable initCallable{ initPtr }; 
		//static CopyCallable copyCallable{ copytPtr };

		JDF::Instance().Register(JDirectory::TypeName(), &defaultCallable);
		JDF::Instance().Register(JDirectory::TypeName(), &initCallable);
		//JDF::Instance().RegisterCopy(JDirectory::TypeName(), &copyCallable);
	}
	void JDirectoryFactoryImpl::RegistAddStroage(AddStoragePtr addPtr)
	{
		static AddStorageCallable addStorage{ addPtr };
		JDirectoryFactoryImpl::addStorage = &addStorage;
	}
}