#include"JCameraInterface.h"

namespace JinEngine
{
	JCameraStateInterface::JCameraStateInterface(const std::string& cTypeName, size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(cTypeName, guid, objFlag, owner)
	{}
	JCameraInterface::JCameraInterface(const std::string& cTypeName, const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		: JCameraStateInterface(cTypeName, guid, objFlag, owner)
	{}
}