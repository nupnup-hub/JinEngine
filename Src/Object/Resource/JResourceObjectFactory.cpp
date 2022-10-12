#include"JResourceObjectFactory.h"

namespace JinEngine
{
	bool JResourceObjectFactory::Register(const std::string iden, Core::JCallableInterface<JResourceObject*, Core::JOwnerPtr<JResourceInitData>>* callable)
	{
		return defaultFactory.Register(iden, callable);
	}
	bool JResourceObjectFactory::Register(const std::string iden, Core::JCallableInterface<JResourceObject*, JDirectory*, const Core::JAssetFileLoadPathData&>* callable)
	{
		return loadFactory.Register(iden, callable);
	}
	bool JResourceObjectFactory::Register(const std::string iden, Core::JCallableInterface<JResourceObject*, JResourceObject*, JDirectory*>* callable)
	{
		return copyFactory.Register(iden, callable);
	}
	JResourceObject* JResourceObjectFactory::Create(const std::string& iden, Core::JOwnerPtr<JResourceInitData> initdata)
	{
		return defaultFactory.Invoke(iden, std::move(initdata));
	}
	JResourceObject* JResourceObjectFactory::Create(const std::string& iden, JDirectory& ownerDir, const Core::JAssetFileLoadPathData& pathData)
	{
		return loadFactory.Invoke(iden, &ownerDir, pathData);
	}
	JResourceObject* JResourceObjectFactory::Copy(const std::string& iden, JResourceObject& ori, JDirectory& ownerDir)
	{
		return copyFactory.Invoke(iden, &ori, &ownerDir);
	}
}