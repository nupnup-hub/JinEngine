#include"JLightCulling.h"

namespace JinEngine::Graphic
{
	bool JLightCulling::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::LIGHT_CULLING || taskType == J_GRAPHIC_TASK_TYPE::LIGHT_LIST_DRAW;
	}
}