#include"JDownSampling.h"

namespace JinEngine::Graphic
{
	bool JDownSampling::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::APPLY_DOWN_SAMPLING;
	}
}