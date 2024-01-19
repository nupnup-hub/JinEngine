#include"JSceneDraw.h"

namespace JinEngine::Graphic
{
	bool JSceneDraw::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{	
		return taskType == J_GRAPHIC_TASK_TYPE::SCENE_DRAW;
	}
}