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


#include"JDx12ToneMapping.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"   
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h" 
#include"../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../JGraphicUpdateHelper.h" 
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Object/Component/Camera/JCamera.h"    
 
#define HDR_DISPLAY_MAPPING_SYMBOL L"HDR_DISPLAY_MAPPING"
#define OUT_LUMA_SYMBOL L"OUT_LUMA"
#define APPLY_EXPOSURE_SYMBOL L"APPLY_EXPOSURE"
#define APPLY_BLOOM_SYMBOL L"APPLY_BLOOM"
#define COPY_TM_RESULT L"COPY_TM_RESULT" 

namespace JinEngine::Graphic
{
	namespace Tm
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint exposureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint bloomHandleIndex = exposureHandleIndex + 1;
		static constexpr uint destTextureHandleIndex = bloomHandleIndex + 1;
		static constexpr uint outLumaHandleIndex = destTextureHandleIndex + 1;
		static constexpr uint passCBIndex = outLumaHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;

		static constexpr uint cb32BitCount = 7;
		static constexpr uint threadDimX = 8;
		static constexpr uint threadDimY = 8;
	} 
	namespace Private
	{ 
	}

	JDx12ToneMapping::~JDx12ToneMapping()
	{
		ClearResource();
	}
	void JDx12ToneMapping::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12ToneMapping::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12ToneMapping::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}  
	bool JDx12ToneMapping::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		return false;
	}
	bool JDx12ToneMapping::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::POST_PROCESS || type == JGraphicOption::TYPE::DEBUGGING)
			return true;
		else
			return false;
	}
	void JDx12ToneMapping::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{}
	void JDx12ToneMapping::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.changedPart == JGraphicOption::TYPE::POST_PROCESS && set.preOption.postProcess.useHdr != set.newOption.postProcess.useHdr)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));
		if (set.changedPart == JGraphicOption::TYPE::DEBUGGING && set.newOption.debugging.requestRecompileToneMappingShader)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));
	}
	uint JDx12ToneMapping::CalIndex(const JGraphicOption& option)const noexcept
	{
		TM_TYPE type = option.postProcess.useHdr ? TM_TYPE::HDR : TM_TYPE::SDR;
		uint addOption = TM_ADDITIONAL_OPTION_NONE;
		if (option.postProcess.useFxaa)
			addOption |= TM_ADDITIONAL_OPTION_OUT_LUMA;
		if (option.postProcess.exposureType != J_EXPOSURE_TYPE::NONE)
			addOption |= TM_ADDITIONAL_OPTION_EXPOSURE;
		if (option.postProcess.useBloom)
			addOption |= TM_ADDITIONAL_OPTION_BLOOM;
		return (uint)type + addOption * (uint)TM_TYPE::COUNT;
	}
	void JDx12ToneMapping::ApplyToneMapping(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;
 
		JDx12PostProcessComputeSet* tmSet = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(tmSet->context);
		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;

		auto gUser = helper.cam->GraphicResourceUserInterface();
		JDx12GraphicResourceComputeSet srcSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		JDx12GraphicResourceComputeSet exposureSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE, J_GRAPHIC_TASK_TYPE::APPLY_TONE_MAPPING);
		JDx12GraphicResourceComputeSet bloomSet = context->ComputeSet(imageShare->bloom[0][1]);
		JDx12GraphicResourceComputeSet dstSet = context->ComputeSet(imageShare->GetUpdateWaitIntermediate());
		JDx12GraphicResourceComputeSet lumaSet = context->ComputeSet(imageShare->lumaUnorm);
		if (helper.option.postProcess.exposureType == J_EXPOSURE_TYPE::DEFUALT)
			exposureSet = context->ComputeSet(imageShare->defaultExposure);

		const JVector2F srcSize = srcSet.info->GetResourceSize();
		const JVector2F srcInvSize = 1.0f / srcSize;

		context->Transition(srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(dstSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); 
		if (helper.option.postProcess.exposureType != J_EXPOSURE_TYPE::NONE)
			context->Transition(exposureSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		if (helper.option.postProcess.useBloom)
			context->Transition(bloomSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		if(helper.option.postProcess.useFxaa)
			context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); 
		context->FlushResourceBarriers();

		float paperWhiteRatio = helper.option.postProcess.hdrPaperWhite / helper.option.postProcess.maxBrightness;
		context->SetComputeRootSignature(tmRootSignature.Get());
		context->SetComputeRootDescriptorTable(Tm::srcTextureHandleIndex, srcSet.GetGpuSrvHandle());
		if (helper.option.postProcess.exposureType != J_EXPOSURE_TYPE::NONE)
			context->SetComputeRootDescriptorTable(Tm::exposureHandleIndex, exposureSet.GetGpuSrvHandle());
		if (helper.option.postProcess.useBloom)
			context->SetComputeRootDescriptorTable(Tm::bloomHandleIndex, bloomSet.GetGpuSrvHandle());

		context->SetComputeRootDescriptorTable(Tm::destTextureHandleIndex, dstSet.GetGpuUavHandle());
		if (helper.option.postProcess.useFxaa)
			context->SetComputeRootDescriptorTable(Tm::outLumaHandleIndex, lumaSet.GetGpuUavHandle());
		 
		context->SetComputeRoot32BitConstants(Tm::passCBIndex, 0, srcSize);
		context->SetComputeRoot32BitConstants(Tm::passCBIndex, 2, srcInvSize);
		context->SetComputeRoot32BitConstants(Tm::passCBIndex, 4, paperWhiteRatio);
		context->SetComputeRoot32BitConstants(Tm::passCBIndex, 5, helper.option.postProcess.maxBrightness);
		context->SetComputeRoot32BitConstants(Tm::passCBIndex, 6, helper.option.postProcess.bloomStrength);

		const uint shaderIndex = CalIndex(helper.option);
		auto shaderData = tm[shaderIndex].get();
		auto threadDim = shaderData->dispatchInfo.threadDim;
		   
		context->SetPipelineState(shaderData);
		context->Dispatch(JMathHelper::DivideByMultiple<uint>(srcSize.x, threadDim.x),
			JMathHelper::DivideByMultiple<uint>(srcSize.y, threadDim.y), 1);
		 
		imageShare->applyHdr = helper.option.postProcess.useHdr;
		imageShare->AddUpdatedIndexCount();
	}
	void JDx12ToneMapping::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		ClearRootSignature();
		ClearPso();

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice();
		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12ToneMapping::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption()); 
	}
	void JDx12ToneMapping::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildToneMappingRootSignature(device);
	}
	void JDx12ToneMapping::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildToneMappingPso(device, info);
	} 
	void JDx12ToneMapping::BuildToneMappingRootSignature(ID3D12Device* device)
	{ 
		JDx12RootSignatureBuilder2<Tm::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		builder.PushConstants(Tm::cb32BitCount, 0);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		builder.Create(device, L"ToneMappingRootSignature", tmRootSignature.GetAddressOf());
	}
	void JDx12ToneMapping::BuildToneMappingPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)TM_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j <= TM_ADDITIONAL_OPTION_VARIATION_COUNT; ++j)
			{
				const TM_TYPE type = (TM_TYPE)i;
				const TM_ADDITIONAL_OPTION addType = (TM_ADDITIONAL_OPTION)j;

				const uint index = JCUtil::CalDimIndex(i, j, (uint)TM_TYPE::COUNT);
				tm[index] = std::make_unique<JDx12ComputeShaderDataHolder>();
				JDx12ComputeShaderDataHolder* tmHoloder = tm[index].get();
				 
				JComputeShaderInitData initData;
				initData.dispatchInfo.threadDim = JVector3F(Tm::threadDimX, Tm::threadDimY, 1);
				initData.PushThreadDimensionMacro();
				if (type == TM_TYPE::HDR)
					initData.macro.push_back({ HDR_DISPLAY_MAPPING_SYMBOL, std::to_wstring(1) });
				if(Core::HasSQValueEnum(addType, TM_ADDITIONAL_OPTION_OUT_LUMA))
					initData.macro.push_back({ OUT_LUMA_SYMBOL, std::to_wstring(1) });
				if (Core::HasSQValueEnum(addType, TM_ADDITIONAL_OPTION_EXPOSURE))
					initData.macro.push_back({ APPLY_EXPOSURE_SYMBOL, std::to_wstring(1) });
				if (Core::HasSQValueEnum(addType, TM_ADDITIONAL_OPTION_BLOOM))
					initData.macro.push_back({ APPLY_BLOOM_SYMBOL, std::to_wstring(1) });

				auto compileInfo = JCompileInfo(L"ToneMapping.hlsl", L"ToneMapping");
				tmHoloder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
				tmHoloder->dispatchInfo = initData.dispatchInfo;
				 
				JDx12ComputePso::Create(device, tmHoloder, tmRootSignature.Get(), D3D12_PIPELINE_STATE_FLAG_NONE);
			}
		}
	}
	void JDx12ToneMapping::ClearResource()
	{
		ClearRootSignature();
		ClearPso(); 
	}
	void JDx12ToneMapping::ClearRootSignature()
	{
		tmRootSignature = nullptr;
	}
	void JDx12ToneMapping::ClearPso()
	{ 
		for (uint i = 0; i < SIZE_OF_ARRAY(tm); ++i)
			tm[i] = nullptr; 
	} 
}