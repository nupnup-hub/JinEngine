#include"JTransformInterface.h"


namespace JinEngine
{
	JTransformInterface::JTransformInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JGameObject* owner)
		:JComponent(name, guid, flag, owner)
	{}
}