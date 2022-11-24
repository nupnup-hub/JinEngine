#include"JShaderData.h"

namespace JinEngine
{
	uint JComputeShaderData::DispatchInfo::GetTotalThreadCount()const noexcept
	{
		return (groupDim.x * groupDim.y * groupDim.z) * (threadDim.x * threadDim.y * threadDim.z);
	}
}