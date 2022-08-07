#include"JLightInterface.h"

namespace JinEngine
{
	JLightInterface::JLightInterface(const std::string& cTypeName, const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(cTypeName, guid, objFlag, owner)
	{}
}