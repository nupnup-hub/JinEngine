#include"JMaterialInterface.h"

namespace JinEngine
{
	JMaterialInterface::JMaterialInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JResourceObject(name, guid, flag, directory, formatIndex)
	{}
}