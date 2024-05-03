#include"JGpuAcceleratorInterface.h"
#include"JGpuAcceleratorInfo.h"
#include"../JGraphic.h"
#include"../JGraphicPrivate.h" 

namespace JinEngine::Graphic
{ 
	bool JGpuAcceleratorInterface::CreateGpuAccelerator(const JGpuAcceleratorBuildDesc& desc)
	{
		info = JGraphicPrivate::AcceleratorInterface::CreateGpuAccelerator(desc);
		return true;
	}
	bool JGpuAcceleratorInterface::DestroyGpuAccelerator()
	{
		return JGraphicPrivate::AcceleratorInterface::DestroyGpuAccelerator(info.Release());
	}
	void JGpuAcceleratorInterface::UpdateTransform(const JUserPtr<JComponent>& comp)
	{ 
		JGraphicPrivate::AcceleratorInterface::UpdateTransform(info.Get(), comp);
	}
	void JGpuAcceleratorInterface::AddComponent(const JUserPtr<JComponent>& comp)
	{
		JGraphicPrivate::AcceleratorInterface::AddComponent(info.Get(), comp);
	}
	void JGpuAcceleratorInterface::RemoveComponent(const JUserPtr<JComponent>& comp)
	{
		JGraphicPrivate::AcceleratorInterface::RemoveComponent(info.Get(), comp);
	}
	int JGpuAcceleratorInterface::GetArrayIndex()const noexcept
	{
		return info != nullptr ? info->GetArrayIndex() : invalidIndex;
	}
	J_GPU_ACCELERATOR_BUILD_OPTION JGpuAcceleratorInterface::GetBuildOption()const noexcept
	{
		return info != nullptr ? info->GetBuildOption() : J_GPU_ACCELERATOR_BUILD_OPTION_NONE;
	}
	bool JGpuAcceleratorInterface::HasInfo()const noexcept
	{
		return info != nullptr;
	}
	bool JGpuAcceleratorInterface::CanBuildGpuAccelerator()noexcept
	{
		return _JGraphic::Instance().CanBuildGpuAccelerator();
	}

	JGpuAcceleratorUserInterface::JGpuAcceleratorUserInterface(JGpuAcceleratorInterface* gInterface)
		:gPtrWrapper(gInterface->GetPointerWrapper())
	{} 
	int JGpuAcceleratorUserInterface::GetArrayIndex()const noexcept
	{
		return gPtrWrapper->Get()->GetArrayIndex();
	}
	J_GPU_ACCELERATOR_BUILD_OPTION JGpuAcceleratorUserInterface::GetBuildOption()const noexcept
	{
		return gPtrWrapper->Get()->GetBuildOption();
	}
	bool JGpuAcceleratorUserInterface::HasInfo()const noexcept
	{
		return gPtrWrapper->Get()->HasInfo();
	}
}