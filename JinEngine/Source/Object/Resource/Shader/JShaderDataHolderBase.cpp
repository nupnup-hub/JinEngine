#include"JShaderDataHolderBase.h"

namespace JinEngine
{
	uint JComputeShaderDataHolderBase::DispatchInfo::GetTotalThreadCount()const noexcept
	{
		return (groupDim.x * groupDim.y * groupDim.z) * (threadDim.x * threadDim.y * threadDim.z);
	}
}