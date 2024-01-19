#include"JOutline.h"

namespace JinEngine::Graphic
{
	bool JOutline::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::OUT_LINE_DRAW;
	}
}