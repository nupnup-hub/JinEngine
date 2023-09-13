#include"JCullingUserAccess.h"

namespace JinEngine::Graphic
{
	bool JCullingUserAccess::AllowOcclusionCulling()const noexcept
	{
		return AllowHzbOcclusionCulling() || AllowHdOcclusionCulling();
	}
}