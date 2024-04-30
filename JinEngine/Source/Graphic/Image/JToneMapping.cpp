#include"JToneMapping.h"

namespace JinEngine::Graphic
{
	bool JToneMapping::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return J_GRAPHIC_TASK_TYPE::APPLY_BLOOM == taskType || 
			J_GRAPHIC_TASK_TYPE::APPLY_TONE_MAPPING == taskType;
	}
}