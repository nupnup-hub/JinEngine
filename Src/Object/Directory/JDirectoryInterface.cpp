#include"JDirectoryInterface.h"

namespace JinEngine
{
	JDirectoryInterface::JDirectoryInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag)
		:JObject(name, guid, flag)
	{}
}