#include"JRaytracingDenoiser.h"

namespace JinEngine::Graphic
{
	bool JRaytracingDenoiser::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_DENOISE;
	}
}