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
		Core::JFactory<std::string, false, JGameObject*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JGameObject*, JScene*> initFactory;
		Core::JFactory<std::string, false, JGameObject*, std::wifstream&, JGameObject*, JScene*> loadFactory;
		Core::JFactory<std::string, false, JGameObject*, JGameObject*, JGameObject*> copyFactory;
	private:
		bool RegisterDefault(const std::string& iden, Core::JCallableInterface<JGameObject*, JGameObject*>* callable)
		{
			return defaultFactory.Register(iden, callable);
		}
		bool RegisterInit(const std::string& iden, Core::JCallableInterface<JGameObject*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JGameObject*, JScene*>* callable)
		{
			return initFactory.Register(iden, callable);
		}
		bool RegisterLoad(const std::string& iden, Core::JCallableInterface<JGameObject*, std::wifstream&, JGameObject*, JScene*>* callable)
		{
			return loadFactory.Register(iden, callable);
		}
		bool RegisterCopy(const std::string& iden, Core::JCallableInterface<JGameObject*, JGameObject*, JGameObject*>* callable)
		{
			return copyFactory.Register(iden, callable);
		}

		JGameObject* Create(const std::string& iden, JGameObject& parent)
		{
			return defaultFactory.Invoke(iden, &parent);
		}
		JGameObject* Create(const std::string& iden, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject& parent)
		{
			return initFactory.Invoke(iden, name, guid, flag, &parent, nullptr);
		}
		JGameObject* CreateRoot(const std::string& iden, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JScene& ownerScene)
		{
			return initFactory.Invoke(iden, name, guid, flag, nullptr, &ownerScene);
		}
		JGameObject* Create(const std::string& iden, std::wifstream& stream, JGameObject& parent, JScene& owner)
		{
			return loadFactory.Invoke(iden, stream, &parent, &owner);
		}
		JGameObject* Copy(const std::string& iden, JGameObject& ori, JGameObject& parent)
		{
			return copyFactory.Invoke(iden, &ori, &parent);
		}
	};
	using JGF = Core::JSingletonHolder<JGameObjectFactory>;

	JGameObject* JGameObjectFactoryImpl::Create(JGameObject& parent)
	{
		return JGF::Instance().Create(JGameObject::TypeName(), parent);
	}
	JGameObject* JGameObjectFactoryImpl::Create(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject& parent)
	{
		return JGF::Instance().Create(JGameObject::TypeName(), name, guid, flag, parent);
	}
	JGameObject* JGameObjectFactoryImpl::CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JScene& ownerScene)
	{
		return JGF::Instance().CreateRoot(JGameObject::TypeName(), name, guid, flag, ownerScene);
	}
	JGameObject* JGameObjectFactoryImpl::Copy(JGameObject& ori, JGameObject& tarParent)
	{
		//Root GameObject don't copy
		if (ori.IsRoot())
			return nullptr;

		return JGF::Instance().Copy(JGameObject::TypeName(), ori, tarParent);
	}
	JGameObject* JGameObjectFactoryImpl::Create(std::wifstream& stream, JGameObject* parent, JScene* ownerScene)
	{
		return JGF::Instance().Create(JGameObject::TypeName(), stream, *parent, *ownerScene);
	}
	void JGameObjectFactoryImpl::Register(DefaultPtr defaultPtr, InitPtr initPtr, LoadPtr loadPtr, CopytPtr copytPtr)
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
}