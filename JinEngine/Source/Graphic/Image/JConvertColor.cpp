#include"JConvertColor.h"

namespace JinEngine::Graphic
{
	bool JConvertColor::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT || taskType == J_GRAPHIC_TASK_TYPE::APPLY_CONVERT_COLOR;
	}
}