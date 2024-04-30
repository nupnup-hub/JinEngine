#include"JAntialise.h"

namespace JinEngine::Graphic
{
	bool JAntialise::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::APPLY_ANTIALISE;
	}
}