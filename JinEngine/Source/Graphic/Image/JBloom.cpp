#include"JBloom.h"

namespace JinEngine::Graphic
{
	bool JBloom::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return J_GRAPHIC_TASK_TYPE::APPLY_BLOOM == taskType;
	}
}