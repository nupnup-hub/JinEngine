#include"JSsao.h"

namespace JinEngine::Graphic
{
	bool JSsao::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::APPLY_SSAO;
	}
}