#include"JAnimatorInterface.h"

namespace JinEngine
{
	JAnimatorInterface::JAnimatorInterface(const std::string& cTypeName, const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(cTypeName, guid, objFlag, owner)
	{}

}