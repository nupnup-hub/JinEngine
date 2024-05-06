/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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