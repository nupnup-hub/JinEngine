#include"JSkeletonAssetInterface.h"

namespace JinEngine
{
	JSkeletonAssetInterface::JSkeletonAssetInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JResourceObject(name, guid, flag, directory, formatIndex)
	{}
}