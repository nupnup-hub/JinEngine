#include"JGameObjectInterface.h"

namespace JinEngine
{
	JGameObjectInterface::JGameObjectInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag)
		: JObject(name, guid, flag)
	{}
}