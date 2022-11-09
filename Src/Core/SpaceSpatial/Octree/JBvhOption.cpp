#include"JBvhOption.h"

namespace JinEngine
{
	namespace Core
	{
		JBvhOption::JBvhOption(bool isActivated, bool isDebugActivated, bool isDebugOnlyLeaf, J_BVH_BUILD_TYPE buildType, J_BVH_SPLIT_TYPE splitType)
			:isActivated(isActivated),
			isDebugActivated(isDebugActivated),
			isDebugOnlyLeaf(isDebugOnlyLeaf),
			buildType(buildType),
			splitType(splitType)
		{}
	}
}