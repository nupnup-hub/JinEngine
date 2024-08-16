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

#include"JGraphicObjectDataSet.h"
#include"../../Object/JObjectTypeStatistics.h"

namespace JinEngine::Graphic
{ 
	JGraphicObjectDataSetBase::JGraphicObjectDataSetBase(const JUserPtr<JObject>& object)
		:JGraphicModuleManagedDataFrame(object)
	{}
	JCsmHandleUserInterface* JGraphicObjectDataSetBase::GetCsmHandleUserInterface()const noexcept
	{
		return GetCsmHandleInterface();
	}
	JCsmTargetUserInterface* JGraphicObjectDataSetBase::GetCsmTargetUserInterface()const noexcept
	{
		return GetCsmTargetInterface();
	}
	JCullingUserInterface* JGraphicObjectDataSetBase::GetCullingUserInterface()const noexcept
	{ 
		return GetCullingInterface();
	}
	JFrameUpdateUserInterface* JGraphicObjectDataSetBase::GetFrameUpdateUserInterface()const noexcept
	{
		return GetFrameUpdateInterface();
	}
	JGpuAcceleratorUserInterface* JGraphicObjectDataSetBase::GetGpuAcceleratorUserInterface()const noexcept
	{
		return GetGpuAcceleratorInterface();
	}
	JGraphicResourceUserInterface* JGraphicObjectDataSetBase::GetGraphicResourceUserInterface()const noexcept
	{
		return GetGraphicResourceInterface();
	}

	/*
		JGraphicObjectDataSetAllInOne::JGraphicObjectDataSetAllInOne(const JUserPtr<JObject>& object,
		std::unique_ptr<JCullingInterface>&& cullingInterface,
		std::unique_ptr<JFrameUpdateInterface>&& frameInterface,
		std::unique_ptr<JGpuAcceleratorInterface>&& gpuAcceleratorInterface,
		std::unique_ptr<JGraphicResourceInterface>&& graphicResourceInterface)
		:JGraphicObjectDataSetBase(object),
		cullingInterface(std::move(cullingInterface)),
		frameInterface(std::move(frameInterface)),
		gpuAcceleratorInterface(std::move(gpuAcceleratorInterface)),
		graphicResourceInterface(std::move(graphicResourceInterface))
	{}  
	JCullingInterface* JGraphicObjectDataSetAllInOne::GetCullingInterface()const noexcept
	{
		return cullingInterface.get();
	}
	JFrameUpdateInterface* JGraphicObjectDataSetAllInOne::GetFrameUpdateInterface()const noexcept
	{
		return frameInterface.get();
	}
	JGpuAcceleratorInterface* JGraphicObjectDataSetAllInOne::GetGpuAcceleratorInterface()const noexcept
	{
		return gpuAcceleratorInterface.get();
	}
	JGraphicResourceInterface* JGraphicObjectDataSetAllInOne::GetGraphicResourceInterface()const noexcept
	{
		return graphicResourceInterface.get();
	}
	*/

	bool JObjectDataSetMetadata::IsComponentType()const noexcept
	{
		return uniqueIndex < totalCompVariation;
	}
	bool JObjectDataSetMetadata::IsResourceType()const noexcept
	{
		return uniqueIndex >= totalCompVariation;
	}
}