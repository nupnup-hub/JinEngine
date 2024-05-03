#include"JPostProcessHistogram.h"

namespace JinEngine::Graphic
{
	bool JPostProcessHistogram::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::MANAGE_POST_PROCESS_HISTOGRAM;
	}
}