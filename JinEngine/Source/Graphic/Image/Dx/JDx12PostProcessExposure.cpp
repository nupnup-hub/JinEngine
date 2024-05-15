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


#include"JDx12PostProcessExposure.h"  
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"  
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Command/Dx/JDx12CommandContext.h" 
#include"../../Utility/Dx/JDx12ObjectCreation.h" 
#include"../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../JGraphicUpdateHelper.h" 
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Object/Component/Camera/JCamera.h"   
//#include"../../../Develop/Debug/JDevelopDebug.h"
 
#define HDR_DISPLAY_MAPPING L"HDR_DISPLAY_MAPPING"

namespace JinEngine::Graphic
{
	namespace Extract
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint exposureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint lumaHandleIndex = exposureHandleIndex + 1;
		static constexpr uint passCBIndex = lumaHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;

		static constexpr uint cb32BitCount = 2;
		static constexpr uint threadDimX = 8;
		static constexpr uint threadDimY = 8;  
	}
	namespace Update
	{
		static constexpr uint histogramHandleIndex = 0;
		static constexpr uint exposureHandleIndex = histogramHandleIndex + 1;
		static constexpr uint passCBIndex = exposureHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;

		static constexpr uint cb32BitCount = 5;
		static constexpr uint threadDimX = Constants::histogramBufferCount;
		static constexpr uint threadDimY = 1;
	}
	namespace Private
	{ 
		static void StuffComputeShaderDispatchInfo(_Inout_ JComputeShaderInitData& initHelper, 
			const JGraphicInfo& graphicInfo,
			const uint threadXCount,
			const uint threadYCount)
		{ 
			initHelper.dispatchInfo.threadDim = JVector3<uint>(threadXCount, threadYCount, 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>(1, 1, 1);
			initHelper.dispatchInfo.taskOriCount = 0;	//not valid
		}
	}

	JDx12PostProcessExposure::~JDx12PostProcessExposure()
	{
		ClearResource();
	}
	void JDx12PostProcessExposure::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12PostProcessExposure::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12PostProcessExposure::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12PostProcessExposure::ExtractExposure(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12PostProcessComputeSet* set = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);
		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;

		auto gUser = helper.cam->GraphicResourceUserInterface(); 
		JDx12GraphicResourceComputeSet srcSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		JDx12GraphicResourceComputeSet exposureSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE, J_GRAPHIC_TASK_TYPE::MANAGE_POST_PROCESS_EXPOSURE);
		JDx12GraphicResourceComputeSet lumaSet = context->ComputeSet(imageShare->lumaLowResolutionUint);

		context->Transition(srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(exposureSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
 
		const JVector2F lumaSize = lumaSet.info->GetResourceSize();
		const JVector2F lumaInvSize = 1.0f / lumaSize;

		context->SetComputeRootSignature(extractRootSignature.Get());
		context->SetComputeRootDescriptorTable(Extract::srcTextureHandleIndex, srcSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Extract::exposureHandleIndex, exposureSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Extract::lumaHandleIndex, lumaSet.GetGpuUavHandle());
		context->SetComputeRoot32BitConstants(Extract::passCBIndex, 0, lumaInvSize);

		auto shaderData = extract.get();
		auto threadDim = shaderData->dispatchInfo.threadDim;

		//always lumaSize % threadCount is 0 
		context->SetPipelineState(shaderData);
		context->Dispatch2D(lumaSize, threadDim.XY()); 
	}
	void JDx12PostProcessExposure::UpdateExposure(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12PostProcessComputeSet* set = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);
		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;

		auto gUser = helper.cam->GraphicResourceUserInterface();
		JDx12GraphicResourceComputeSet histogramSet = context->ComputeSet(imageShare->histogram);
		JDx12GraphicResourceComputeSet exposureSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE, J_GRAPHIC_TASK_TYPE::MANAGE_POST_PROCESS_EXPOSURE);
 
		context->Transition(histogramSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(exposureSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
 
		const JVector4F dataPack(helper.option.postProcess.targetLuminance, helper.option.postProcess.adaptationRate, helper.option.postProcess.minExposure, helper.option.postProcess.maxExposure);
		const uint pixelCount = imageShare->lumaLowResolutionUint->GetWidth() * imageShare->lumaLowResolutionUint->GetHeight();
		  
		context->SetComputeRootSignature(updateAdaptRootSignature.Get());
		context->SetComputeRootDescriptorTable(Update::histogramHandleIndex, histogramSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Update::exposureHandleIndex, exposureSet.GetGpuUavHandle());
		context->SetComputeRoot32BitConstants(Update::passCBIndex, 0, dataPack);
		context->SetComputeRoot32BitConstants(Update::passCBIndex, 4, pixelCount);

		auto shaderData = updateAdapt.get();
		auto threadDim = shaderData->dispatchInfo.threadDim;
  
		context->SetPipelineState(shaderData);
		context->Dispatch(1, 1, 1);
	}
	void JDx12PostProcessExposure::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12PostProcessExposure::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildExtractRootSignature(device);
		BuildUpdateAdaptRootSignature(device);
	}
	void JDx12PostProcessExposure::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildExtractPso(device, info);
		BuildUpdateAdaptPso(device, info);
	}
	void JDx12PostProcessExposure::BuildExtractRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<Extract::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushConstants(Extract::cb32BitCount, 0);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);	 
		builder.Create(device, L"LumaExtractRootSIgnature", extractRootSignature.GetAddressOf());
	}
	void JDx12PostProcessExposure::BuildExtractPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		extract = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputeShaderDataHolder* holder = extract.get();
		JComputeShaderInitData initData;
		initData.dispatchInfo.threadDim = JVector3F(Extract::threadDimX, Extract::threadDimY, 1); 	
		initData.PushThreadDimensionMacro();

		auto compileInfo = JCompileInfo(ShaderRelativePath::Postprocessing(L"ExtractLuma.hlsl"), L"ExtractLuma");

		holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
		holder->dispatchInfo = initData.dispatchInfo;

		JDx12ComputePso::Create(device, holder, extractRootSignature.Get()); 
	}
	void JDx12PostProcessExposure::BuildUpdateAdaptRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<Update::slotCount> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); 
		builder.PushConstants(Update::cb32BitCount, 0); 
		builder.Create(device, L"UpdateAdaptExposureRootSIgnature", updateAdaptRootSignature.GetAddressOf());
	}
	void JDx12PostProcessExposure::BuildUpdateAdaptPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		updateAdapt = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputeShaderDataHolder* holder = updateAdapt.get();
		JComputeShaderInitData initData;
		initData.dispatchInfo.threadDim = JVector3F(Update::threadDimX, Update::threadDimY, 1); 
		initData.PushThreadDimensionMacro();

		auto compileInfo = JCompileInfo(ShaderRelativePath::Postprocessing(L"AdaptExposure.hlsl"), L"AdaptExposure");
		holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
		holder->dispatchInfo = initData.dispatchInfo;

		JDx12ComputePso::Create(device, holder, updateAdaptRootSignature.Get());
	}
	void JDx12PostProcessExposure::ClearResource()
	{
		ClearRootSignature();
		ClearPso();
	}
	void JDx12PostProcessExposure::ClearRootSignature()
	{
		extractRootSignature = nullptr;
		updateAdaptRootSignature = nullptr;
	}
	void JDx12PostProcessExposure::ClearPso()
	{
		extract = nullptr;
		updateAdapt = nullptr;
	}
}