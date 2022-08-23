#include"JGameObjectFactory.h"
#include"JGameObject.h"
#include"../../Core/Factory/JFactory.h"

namespace JinEngine
{
	class JGameObjectFactoryImpl;
	class JGameObjectFactory
	{
	private:
		friend class JGameObjectFactoryImpl;
	private:
		Core::JFactory<std::string, false, JGameObject*, JGameObject*> defaultFactory;
		Core::JFactory<std::string, false, JGameObject*, const std::string&, const size_t, const J_OBJECT_FLAG, JGameObject*, JScene*> initFactory;
		Core::JFactory<std::string, false, JGameObject*, std::wifstream&, JGameObject*> loadFactory;
		Core::JFactory<std::string, false, JGameObject*, JGameObject*, JGameObject*> copyFactory;
	private:
		bool RegisterDefault(const std::string& iden, Core::JCallableInterface<JGameObject*, JGameObject*>* callable)
		{
			return defaultFactory.Regist(iden, callable);
		}
		bool RegisterInit(const std::string& iden, Core::JCallableInterface<JGameObject*, const std::string&, const size_t, const J_OBJECT_FLAG, JGameObject*, JScene*>* callable)
		{
			return initFactory.Regist(iden, callable);
		}
		bool RegisterLoad(const std::string& iden, Core::JCallableInterface<JGameObject*, std::wifstream&, JGameObject*>* callable)
		{
			return loadFactory.Regist(iden, callable);
		}
		bool RegisterCopy(const std::string& iden, Core::JCallableInterface<JGameObject*, JGameObject*, JGameObject*>* callable)
		{
			return copyFactory.Regist(iden, callable);
		}

		JGameObject* Create(const std::string& iden, JGameObject& parent)
		{
			return defaultFactory.Invoke(iden, &parent);
		}
		JGameObject* Create(const std::string& iden, const std::string& name, size_t guid, J_OBJECT_FLAG flag, JGameObject& parent)
		{
			return initFactory.Invoke(iden, name, std::move(guid), std::move(flag), &parent, nullptr);
		}
		JGameObject* CreateRoot(const std::string& iden, const std::string& name, size_t guid, J_OBJECT_FLAG flag, JScene& ownerScene)
		{
			return initFactory.Invoke(iden, name, std::move(guid), std::move(flag), nullptr, &ownerScene);
		}
		JGameObject* Create(const std::string& iden, std::wifstream& stream, JGameObject& parent)
		{
			return loadFactory.Invoke(iden, stream, &parent);
		}
		JGameObject* Copy(const std::string& iden, JGameObject& ori, JGameObject& parent)
		{
			return copyFactory.Invoke(iden, &ori, &parent);
		}
	};
	using JGF = Core::JSingletonHolder<JGameObjectFactory>;

	JGameObjectFactoryImpl::AddStorageCallable* JGameObjectFactoryImpl::addStorage;

	JGameObject* JGameObjectFactoryImpl::Create(JGameObject& parent)
	{
		JGameObject* res = JGF::Instance().Create(JGameObject::TypeName(), parent);
		(*addStorage)(res->GetOwnerScene(), *res);
		return res;
	}
	JGameObject* JGameObjectFactoryImpl::Create(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject& parent)
	{
		JGameObject* res = JGF::Instance().Create(JGameObject::TypeName(), name, guid, flag, parent);
		(*addStorage)(res->GetOwnerScene(), *res);
		return res;
	}
	JGameObject* JGameObjectFactoryImpl::CreateRoot(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JScene& ownerScene)
	{
		JGameObject* res = JGF::Instance().CreateRoot(JGameObject::TypeName(), name, guid, flag, ownerScene);
		(*addStorage)(res->GetOwnerScene(), *res);
		return res;
	}
	JGameObject* JGameObjectFactoryImpl::Copy(JGameObject& ori, JGameObject& tarParent)
	{
		//Root GameObject don't copy
		if (ori.IsRoot())
			return nullptr;

		JGameObject* res = JGF::Instance().Copy(JGameObject::TypeName(), ori, tarParent);
		(*addStorage)(res->GetOwnerScene(), *res);
		return res;
	}
	JGameObject* JGameObjectFactoryImpl::Create(std::wifstream& stream, JGameObject* parent)
	{
		JGameObject* res = JGF::Instance().Create(JGameObject::TypeName(), stream, *parent);
		(*addStorage)(res->GetOwnerScene(), *res);
		return res;
	}
	void JGameObjectFactoryImpl::Regist(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr)
	{
		static DefaultCallable defaultCallable{ defaultPtr };
		static InitCallable initCallable{ initPtr };
		static LoadCallable loadCallable{ loadPtr };
		static CopyCallable copyCallable{ copytPtr };

		JGF::Instance().RegisterDefault(JGameObject::TypeName(), &defaultCallable);
		JGF::Instance().RegisterInit(JGameObject::TypeName(), &initCallable);
		JGF::Instance().RegisterLoad(JGameObject::TypeName(), &loadCallable);
		JGF::Instance().RegisterCopy(JGameObject::TypeName(), &copyCallable);
	}
	void JGameObjectFactoryImpl::RegisterAddStroage(AddStoragePtr addPtr)
	{
		static AddStorageCallable addStorage{ addPtr };
		JGameObjectFactoryImpl::addStorage = &addStorage;
	}
}