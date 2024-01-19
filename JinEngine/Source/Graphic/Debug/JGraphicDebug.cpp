#include"JGraphicDebug.h"

namespace JinEngine::Graphic
{
	bool JGraphicDebug::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE ||
			taskType == J_GRAPHIC_TASK_TYPE::NORMAL_MAP_VISUALIZE ||
			taskType == J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE;
	}
}