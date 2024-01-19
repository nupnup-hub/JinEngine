#include"JHZBOccCulling.h" 

namespace JinEngine::Graphic
{ 
	bool JHZBOccCulling::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::HZB_CULLING;
	}
}
