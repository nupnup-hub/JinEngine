#include"JCameraInterface.h"

namespace JinEngine
{
	JCameraInterface::JCameraInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(cTypeName, guid, objFlag, owner)
	{}
}