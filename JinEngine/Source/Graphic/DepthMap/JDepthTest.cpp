#include"JDepthTest.h"

namespace JinEngine::Graphic
{
	bool JDepthTest::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::DEPTH_MAP_DRAW;
	}
}