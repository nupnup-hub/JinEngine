#include"JSkeletonAssetInterface.h"

namespace JinEngine
{
	JSkeletonAssetAvatarInterface::JSkeletonAssetAvatarInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JResourceObject(name, guid, flag, directory, formatIndex)
	{}
	JSkeletonAssetInterface::JSkeletonAssetInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		:JSkeletonAssetAvatarInterface(name, guid, flag, directory, formatIndex)
	{}
}