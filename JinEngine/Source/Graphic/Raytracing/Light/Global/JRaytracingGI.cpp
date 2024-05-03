#include"JRaytracingGI.h"

namespace JinEngine::Graphic
{
	bool JRaytracingGI::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_GI;
	}
}