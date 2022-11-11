#include"JTransformInterface.h"


namespace JinEngine
{
	JTransformInterface::JTransformInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		:JComponent(name, guid, flag, owner)
	{}
}