#include"JPostProcessExposure.h"

namespace JinEngine::Graphic
{
	bool JPostProcessExposure::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::MANAGE_POST_PROCESS_EXPOSURE;
	}
}