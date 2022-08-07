#include"JModelInterface.h"

namespace JinEngine
{
	JModelInterface::JModelInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JResourceObject(name, guid, flag, directory, formatIndex)
	{

	}
}