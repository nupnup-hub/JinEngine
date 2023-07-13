#include"JAvatarJoint.h"

namespace JinEngine
{
	JAvatarJointGuide::JAvatarJointGuide(const std::string& guideName, const std::string& defaultJointName, uint8 index)
		:guideName(guideName), defaultJointName(defaultJointName), index(index)
	{}
}