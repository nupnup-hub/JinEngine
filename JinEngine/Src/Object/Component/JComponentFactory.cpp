#include"JComponentFactory.h"

namespace JinEngine
{
	bool JComponentFactory::Register(const std::string& iden, Core::JCallableInterface<JComponent*, JGameObject*>* callable)
	{
		return defaultFactory.Register(iden, callable);
	}
	bool JComponentFactory::Register(const std::string& iden, Core::JCallableInterface<JComponent*, const size_t, const J_OBJECT_FLAG, JGameObject*>* callable)
	{
		return initFactory.Register(iden, callable);
	}
	bool JComponentFactory::Register(const std::string& iden, Core::JCallableInterface<JComponent*, std::wifstream&, JGameObject*>* callable)
	{
		return loadFactory.Register(iden, callable);
	}
	bool JComponentFactory::Register(const std::string& iden, Core::JCallableInterface<JComponent*, JComponent*, JGameObject*>* callable)
	{
		return copyFactory.Register(iden, callable);
	}

	JComponent* JComponentFactory::Create(const std::string& typeName, JGameObject& owner)
	{
		return defaultFactory.Invoke(typeName, &owner);
	}
	JComponent* JComponentFactory::Create(const std::string& typeName, const size_t guid, const J_OBJECT_FLAG flag, JGameObject& owner)
	{
		return initFactory.Invoke(typeName, guid, flag, &owner);
	}
	JComponent* JComponentFactory::Load(const std::string& typeName, std::wifstream& stream, JGameObject& owner)
	{
		return loadFactory.Invoke(typeName, stream, &owner);
	}
	JComponent* JComponentFactory::Copy(const std::string& typeName, JComponent& ori, JGameObject& owner)
	{
		return copyFactory.Invoke(typeName, &ori, &owner);
	}
}