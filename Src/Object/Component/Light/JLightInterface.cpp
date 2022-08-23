#include"JLightInterface.h"

namespace JinEngine
{
	JLightInterface::JLightInterface(const std::string& cTypeName, const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(cTypeName, guid, objFlag, owner)
	{}
}