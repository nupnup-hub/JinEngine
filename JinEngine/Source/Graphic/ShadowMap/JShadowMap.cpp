#include"JShadowMap.h" 

namespace JinEngine::Graphic
{
	bool JShadowMap::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW;
	}
}