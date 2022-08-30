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
		Core::JFactory<std::string, false, JDirectory*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JDirectory*> initFactory;
		Core::JFactory<std::string, false, JDirectory*, JDirectory*, const JDirectoryPathData&> loadFactory;
		Core::JFactory<std::string, false, JDirectory*, JDirectory*, JDirectory*> copyFactory;
	private:
		bool Register(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*>* callable)
		{
			return defaultFactory.Regist(iden, callable);
		}
		bool Register(const std::string& iden, Core::JCallableInterface<JDirectory*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JDirectory*>* callable)
		{
			return initFactory.Regist(iden, callable);
		}
		bool RegisterLoad(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*, const JDirectoryPathData&>* callable)
		{
			return loadFactory.Regist(iden, callable);
		}
		bool RegisterCopy(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*, JDirectory*>* callable)
		{
			return copyFactory.Regist(iden, callable);
		}

		JDirectory* Create(JDirectory& parent)
		{
			return static_cast<JDirectory*>(defaultFactory.Invoke(JDirectory::TypeName(), &parent));
		}
		JDirectory* CreateRoot(const std::wstring& name, size_t guid, J_OBJECT_FLAG flag)
		{
			return static_cast<JDirectory*>(initFactory.Invoke(JDirectory::TypeName(), name, std::move(guid), std::move(flag), nullptr));
		}
		JDirectory* Create(const std::wstring& name, size_t guid, J_OBJECT_FLAG flag, JDirectory& parent)
		{
			return static_cast<JDirectory*>(initFactory.Invoke(JDirectory::TypeName(), name, std::move(guid), std::move(flag), &parent));
		}
		JDirectory* Load(JDirectory& parent, const JDirectoryPathData& pathData)
		{
			return static_cast<JDirectory*>(loadFactory.Invoke(JDirectory::TypeName(), &parent, pathData));
		}
		JDirectory* LoadRoot(const JDirectoryPathData& pathData)
		{
			return static_cast<JDirectory*>(loadFactory.Invoke(JDirectory::TypeName(), nullptr, pathData));
		}
		JDirectory* Copy(JDirectory& ori, JDirectory& parent)
		{
			return static_cast<JDirectory*>(copyFactory.Invoke(JDirectory::TypeName(), &ori, &parent));
		}
	};
	using JDF = Core::JSingletonHolder<JDirectoryFactory>;

	JDirectoryFactoryImpl::AddStorageCallable* JDirectoryFactoryImpl::addStorage;

	JDirectory* JDirectoryFactoryImpl::Create(JDirectory& parent)
	{
		JDirectory* res = JDF::Instance().Create(parent);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), *res);
		return res;
	} 
	JDirectory* JDirectoryFactoryImpl::CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
	{
		JDirectory* res = JDF::Instance().CreateRoot(name, guid, flag);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), *res);
		return res;
	}
	JDirectory* JDirectoryFactoryImpl::Create(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory& parent)
	{
		JDirectory* res = JDF::Instance().Create(name, guid, flag, parent);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), *res);
		return res;
	}
	JDirectory* JDirectoryFactoryImpl::Load(JDirectory& parent, const JDirectoryPathData& pathData)
	{
		JDirectory* res = JDF::Instance().Load(parent, pathData);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), *res);
		return res;
	}
	JDirectory* JDirectoryFactoryImpl::LoadRoot(const JDirectoryPathData& pathData)
	{
		JDirectory* res = JDF::Instance().LoadRoot(pathData);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), *res);
		return res;
	}
	JDirectory* JDirectoryFactoryImpl::Copy(JDirectory& ori, JDirectory& parent)
	{
		JDirectory* res = JDF::Instance().Copy(ori, parent);
		if (res != nullptr)
			(*addStorage)(&JResourceManager::Instance(), *res);
		return res;
	}
	void JDirectoryFactoryImpl::Register(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopyPtr copyPtr)
	{
		static DefaultCallable defaultCallable{ defaultPtr };
		static InitCallable initCallable{ initPtr }; 
		static LoadCallable loadCallable{ loadPtr };
		static CopyCallable copyCallable{ copyPtr };

		JDF::Instance().Register(JDirectory::TypeName(), &defaultCallable);
		JDF::Instance().Register(JDirectory::TypeName(), &initCallable);
		JDF::Instance().RegisterLoad(JDirectory::TypeName(), &loadCallable);
		JDF::Instance().RegisterCopy(JDirectory::TypeName(), &copyCallable);
	}
	void JDirectoryFactoryImpl::RegisterAddStroage(AddStoragePtr addPtr)
	{
		static AddStorageCallable addStorage{ addPtr };
		JDirectoryFactoryImpl::addStorage = &addStorage;
	}
}