#include"JGpuAcceleratorManager.h"
#include"JGpuAcceleratorInfo.h"

namespace JinEngine::Graphic
{
	JGpuAcceleratorHolder* JGpuAcceleratorManager::GetHolder(JGpuAcceleratorInfo* info)const noexcept
	{
		return info != nullptr ? info->holder.get() : nullptr;
	}
}