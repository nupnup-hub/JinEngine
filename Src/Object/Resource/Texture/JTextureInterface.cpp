#include"JTextureInterface.h"

namespace JinEngine
{
	JTextureInterface::JTextureInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const int formatIndex)
		:JResourceObject(name, guid, flag, directory, formatIndex)
	{}
}