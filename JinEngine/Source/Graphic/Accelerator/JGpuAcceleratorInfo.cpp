#include"JGpuAcceleratorInfo.h"
#include"JGpuAcceleratorHolder.h"
namespace JinEngine::Graphic
{  
	int JGpuAcceleratorInfo::GetArrayIndex()const noexcept
	{
		return arrayIndex;
	}
	J_GPU_ACCELERATOR_BUILD_OPTION JGpuAcceleratorInfo::GetBuildOption()const noexcept
	{
		return buildOptionFlag;
	}
	void JGpuAcceleratorInfo::SetArrayIndex(const int newValue)noexcept
	{
		arrayIndex = newValue;
	} 
	JGpuAcceleratorInfo::JGpuAcceleratorInfo(JGpuAcceleratorManager* manager,
		J_GPU_ACCELERATOR_BUILD_OPTION buildOptionFlag,
		std::unique_ptr<JGpuAcceleratorHolder>&& holder)
		:am(manager), buildOptionFlag(buildOptionFlag), holder(std::move(holder))
	{}
	JGpuAcceleratorInfo::~JGpuAcceleratorInfo()
	{}
}