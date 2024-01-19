#include"JImageProcessing.h"

namespace JinEngine::Graphic
{
	bool JImageProcessing::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::APPLY_BLUR || taskType == J_GRAPHIC_TASK_TYPE::APPLY_DOWN_SAMPLING;
	}
}