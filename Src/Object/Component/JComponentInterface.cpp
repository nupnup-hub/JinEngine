#include"JComponentInterface.h"

namespace JinEngine
{
	JComponentInterface::JComponentInterface(const std::string& classTypeName, const size_t guid, JOBJECT_FLAG flag)
		:JObject(classTypeName, guid, flag)
	{}
}