#include"JSceneInterface.h"

namespace JinEngine
{
	JSceneCashInterface::JSceneCashInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JResourceObject(name, guid, flag, directory, formatIndex)
	{}
	JSceneGameObjInterface::JSceneGameObjInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JSceneCashInterface(name, guid, flag, directory, formatIndex)
	{}
	JSceneCompInterface::JSceneCompInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JSceneGameObjInterface(name, guid, flag, directory, formatIndex)
	{}
	JSceneRegisterInterface::JSceneRegisterInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JSceneCompInterface(name, guid, flag, directory, formatIndex)
	{}
	JSceneFrameInterface::JSceneFrameInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JSceneRegisterInterface(name, guid, flag, directory, formatIndex)
	{}
	JSceneSpaceSpatialInterface::JSceneSpaceSpatialInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JSceneFrameInterface(name, guid, flag, directory, formatIndex)
	{}
	JSceneInterface::JSceneInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JSceneSpaceSpatialInterface(name, guid, flag, directory, formatIndex)
	{}
}