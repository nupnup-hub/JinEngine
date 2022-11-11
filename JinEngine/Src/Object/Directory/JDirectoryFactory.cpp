#include"JDirectoryFactory.h"
#include"JDirectory.h"
#include"../Resource/JResourceManager.h"
#include"../Resource/JResourcePathData.h"
#include"../../Core/Factory/JFactory.h"

namespace JinEngine
{ 
	class JDirectoryFactory
	{
	private:
		friend class JDirectoryFactoryImpl;
	private:
		Core::JFactory<std::string, false, JDirectory*, JDirectory*> defaultFactory;
		Core::JFactory<std::string, false, JDirectory*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JDirectory*> initFactory;
		Core::JFactory<std::string, false, JDirectory*, JDirectory*, const Core::JAssetFileLoadPathData&> loadFactory;
		Core::JFactory<std::string, false, JDirectory*, JDirectory*, JDirectory*> copyFactory;
	private:
		bool Register(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*>* callable)
		{
			return defaultFactory.Register(iden, callable);
		}
		bool Register(const std::string& iden, Core::JCallableInterface<JDirectory*, const std::wstring&, const size_t, const J_OBJECT_FLAG, JDirectory*>* callable)
		{
			return initFactory.Register(iden, callable);
		}
		bool RegisterLoad(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*, const Core::JAssetFileLoadPathData&>* callable)
		{
			return loadFactory.Register(iden, callable);
		}
		bool RegisterCopy(const std::string& iden, Core::JCallableInterface<JDirectory*, JDirectory*, JDirectory*>* callable)
		{
			return copyFactory.Register(iden, callable);
		}

		JDirectory* Create(JDirectory& parent)
		{
			return static_cast<JDirectory*>(defaultFactory.Invoke(JDirectory::TypeName(), &parent));
		}
		JDirectory* CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
		{
			return static_cast<JDirectory*>(initFactory.Invoke(JDirectory::TypeName(), name, guid, flag, nullptr));
		}
		JDirectory* Create(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory& parent)
		{
			return static_cast<JDirectory*>(initFactory.Invoke(JDirectory::TypeName(), name, guid, flag, &parent));
		}
		JDirectory* Load(JDirectory& parent, const Core::JAssetFileLoadPathData& pathData)
		{
			return static_cast<JDirectory*>(loadFactory.Invoke(JDirectory::TypeName(), &parent, pathData));
		}
		JDirectory* LoadRoot(const Core::JAssetFileLoadPathData& pathData)
		{
			return static_cast<JDirectory*>(loadFactory.Invoke(JDirectory::TypeName(), nullptr, pathData));
		}
		JDirectory* Copy(JDirectory& ori, JDirectory& parent)
		{
			return static_cast<JDirectory*>(copyFactory.Invoke(JDirectory::TypeName(), &ori, &parent));
		}
	};
	using JDF = Core::JSingletonHolder<JDirectoryFactory>;

	JDirectory* JDirectoryFactoryImpl::Create(JDirectory& parent)
	{
		return JDF::Instance().Create(parent);
	} 
	JDirectory* JDirectoryFactoryImpl::CreateRoot(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
	{
		return JDF::Instance().CreateRoot(name, guid, flag);
	}
	JDirectory* JDirectoryFactoryImpl::Create(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory& parent)
	{
		return JDF::Instance().Create(name, guid, flag, parent);
	}
	JDirectory* JDirectoryFactoryImpl::Load(JDirectory& parent, const Core::JAssetFileLoadPathData& pathData)
	{
		return JDF::Instance().Load(parent, pathData);
	}
	JDirectory* JDirectoryFactoryImpl::LoadRoot(const Core::JAssetFileLoadPathData& pathData)
	{
		return JDF::Instance().LoadRoot(pathData);
	}
	JDirectory* JDirectoryFactoryImpl::Copy(JDirectory& ori, JDirectory& parent)
	{
		return JDF::Instance().Copy(ori, parent);
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
}