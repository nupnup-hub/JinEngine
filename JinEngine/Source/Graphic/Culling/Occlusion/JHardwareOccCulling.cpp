#include"JHardwareOccCulling.h" 

namespace JinEngine::Graphic
{
	bool JHardwareOccCulling::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::HD_CULLING;
	}
}
