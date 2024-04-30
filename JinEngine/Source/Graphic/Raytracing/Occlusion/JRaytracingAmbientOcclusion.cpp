#include"JRaytracingAmbientOcclusion.h"

namespace JinEngine::Graphic
{
	bool JRaytracingAmbientOcclusion::IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept
	{
		return taskType == J_GRAPHIC_TASK_TYPE::RAYTRACING_AMBIENT_OCCLUSION;
	}
}