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


#include"JDx12ConvertColor.h"  
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"  
#include"../../Utility/Dx/JDx12ObjectCreation.h"  
#include"../../JGraphicUpdateHelper.h" 
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Core/Math/JMathHelper.h" 
#include"../../../Object/Component/Camera/JCamera.h"  
//#include"../../../Develop/Debug/JDevelopDebug.h"
  
#define HDR_DISPLAY_MAPPING L"HDR_DISPLAY_MAPPING"
#define REVERSE_Y L"REVERSE_Y"

namespace JinEngine::Graphic
{ 
	namespace Tlc
	{
		static constexpr uint srcTextureHandleIndex = 0; 
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint passCBIndex = destTextureHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;

		static constexpr uint cb32BitCount = 4;
		static constexpr uint threadDimX = 32;
		static constexpr uint threadDimY = 16; 
		static constexpr uint shaderCount = 2;
	}
	namespace Tdc
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint oriTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint destTextureHandleIndex = oriTextureHandleIndex + 1;
		static constexpr uint passCBIndex = destTextureHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;

		static constexpr uint cb32BitCount = 4;
		static constexpr uint threadDimX = 32;
		static constexpr uint threadDimY = 16;
		static constexpr uint shaderCount = 2;
	}
	namespace Convert
	{
		static constexpr uint srcTextureHandleIndex = 0; 
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint passCBIndex = destTextureHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;

		static constexpr uint cb32BitCount = 4;
		static constexpr uint threadDimX = 16;
		static constexpr uint threadDimY = 16; 
	}
	namespace Private
	{ 
		 
	}

	JDx12ConvertColor::~JDx12ConvertColor()
	{
		ClearResource();
	}
	void JDx12ConvertColor::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12ConvertColor::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12ConvertColor::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	void JDx12ConvertColor::ApplyToLinearColor(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12PostProcessComputeSet* tlcSet = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(tlcSet->context); 
		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;

		auto gUser = helper.cam->GraphicResourceUserInterface();
		JDx12GraphicResourceComputeSet srcSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		JDx12GraphicResourceComputeSet dstSet = context->ComputeSet(imageShare->GetUpdateWaitIntermediate());
	 
		const JVector2F dstSize = dstSet.info->GetResourceSize();
		const JVector2F dstInvSize = 1.0f / dstSize;

		context->Transition(dstSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
		context->SetComputeRootSignature(toLinearRootSignature.Get());
		context->SetComputeRootDescriptorTable(Tlc::srcTextureHandleIndex, srcSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Tlc::destTextureHandleIndex, dstSet.GetGpuUavHandle());
		context->SetComputeRoot32BitConstants(Tlc::passCBIndex, 0, dstSize);
		context->SetComputeRoot32BitConstants(Tlc::passCBIndex, 2, dstInvSize);

		auto shaderData = imageShare->applyHdr ? hdrToLienar.get() : sdrToLienar.get();
		auto threadDim = shaderData->dispatchInfo.threadDim;

		context->SetPipelineState(shaderData);
		context->Dispatch2D(dstSize, threadDim.XY());
		imageShare->AddUpdatedIndexCount();
	}
	void JDx12ConvertColor::ApplyToDisplayColor(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12PostProcessComputeSet* tdcSet = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(tdcSet->context);
		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;

		auto gUser = helper.cam->GraphicResourceUserInterface();
		JDx12GraphicResourceComputeSet srcSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		JDx12GraphicResourceComputeSet dstSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT);
		JDx12GraphicResourceComputeSet oriSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
  
		const JVector2F dstSize = dstSet.info->GetResourceSize();
		const JVector2F dstInvSize = 1.0f / dstSize;

		context->Transition(srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(oriSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(dstSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
 
		context->SetComputeRootSignature(toDisplayRootSignature.Get());
		context->SetComputeRootDescriptorTable(Tdc::srcTextureHandleIndex, srcSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Tdc::oriTextureHandleIndex, oriSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Tdc::destTextureHandleIndex, dstSet.GetGpuUavHandle());
		context->SetComputeRoot32BitConstants(Tdc::passCBIndex, 0, dstSize);
		context->SetComputeRoot32BitConstants(Tdc::passCBIndex, 2, dstInvSize); 

		auto shaderData = imageShare->applyHdr ? fromHdrToDisplay.get() : fromSdrToDisplay.get();
		auto threadDim = shaderData->dispatchInfo.threadDim;

		context->SetPipelineState(shaderData);
		context->Dispatch2D(dstSize, threadDim.XY());
		context->Transition(dstSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	} 
	void JDx12ConvertColor::ApplyConvertColor(JGraphicConvertColorComputeSet* computeSet)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12GraphicConvertColorComputeSet* set = static_cast<JDx12GraphicConvertColorComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context); 
		 
		JDx12GraphicResourceComputeSet srcSet = context->ComputeSet(set->srcInfo);
		JDx12GraphicResourceComputeSet dstSet = context->ComputeSet(set->destInfo);
		 
		const JVector2F dstSize = dstSet.info->GetResourceSize();
		const JVector2F dstInvSize = 1.0f / dstSize;

		context->Transition(srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		context->Transition(dstSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		context->SetComputeRootSignature(reverseYRootSignature.Get());
		context->SetComputeRootDescriptorTable(Convert::srcTextureHandleIndex, srcSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Convert::destTextureHandleIndex, dstSet.GetGpuUavHandle());
		context->SetComputeRoot32BitConstants(Convert::passCBIndex, 0, JVector2<uint>(dstSize));
		context->SetComputeRoot32BitConstants(Convert::passCBIndex, 2, dstInvSize);
		  
		context->SetPipelineState(reverseY.get());
		context->Dispatch2D(dstSize, reverseY->dispatchInfo.threadDim.XY());
		context->Transition(dstSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	void JDx12ConvertColor::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12ConvertColor::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildToLinearColorRootSignature(device);
		BuildToDisplayColorRootSignature(device);
		BuildReverseColorRootSignature(device);
	}
	void JDx12ConvertColor::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildToLinearColorPso(device, info);
		BuildToDisplayColorPso(device, info);
		BuildReverseColorPso(device, info);
	}
	void JDx12ConvertColor::BuildToLinearColorRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<Tlc::slotCount> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushConstants(Tlc::cb32BitCount, 0);
		builder.Create(device, L"ToLinearColorRootSignature", toLinearRootSignature.GetAddressOf());
	} 
	void JDx12ConvertColor::BuildToLinearColorPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		sdrToLienar = std::make_unique<JDx12ComputeShaderDataHolder>();
		hdrToLienar = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputeShaderDataHolder* holderVec[Tlc::shaderCount]
		{
			sdrToLienar.get(), hdrToLienar.get()
		};
		for (uint i = 0; i < Tlc::shaderCount; ++i)
		{
			JDx12ComputeShaderDataHolder* holder = holderVec[i]; 
			JComputeShaderInitData initData;
			initData.CalThreadAndGroupDim(Core::JHardwareInfo::GetGpuInfo()[0], Tlc::threadDimX, Tlc::threadDimY);
			initData.PushThreadDimensionMacro();
			if (i == 1)
				initData.macro.push_back({ HDR_DISPLAY_MAPPING, std::to_wstring(1) });

			//initData.macro.push_back({ COPY_TM_RESULT, std::to_wstring(1) });
			auto compileInfo = JCompileInfo(L"ConvertLinearColor.hlsl", L"main");

			holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;
			JDx12ComputePso::Create(device, holder, toLinearRootSignature.Get());
		} 
	}
	void JDx12ConvertColor::BuildToDisplayColorRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<Tdc::slotCount> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushConstants(Tdc::cb32BitCount, 0);
		builder.Create(device, L"ToDisplayColorRootSignature", toDisplayRootSignature.GetAddressOf());	 
	}
	void JDx12ConvertColor::BuildToDisplayColorPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		fromSdrToDisplay = std::make_unique<JDx12ComputeShaderDataHolder>();
		fromHdrToDisplay = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputeShaderDataHolder* holderVec[Tdc::shaderCount]
		{
			fromSdrToDisplay.get(), fromHdrToDisplay.get()
		};
		for (uint i = 0; i < Tdc::shaderCount; ++i)
		{ 
			JDx12ComputeShaderDataHolder* holder = holderVec[i];
			JComputeShaderInitData initData;
			initData.CalThreadAndGroupDim(Core::JHardwareInfo::GetGpuInfo()[0], Tdc::threadDimX, Tdc::threadDimY);
			initData.PushThreadDimensionMacro();
			if (i == 1)
				initData.macro.push_back({ HDR_DISPLAY_MAPPING, std::to_wstring(1) });

			//initData.macro.push_back({ COPY_TM_RESULT, std::to_wstring(1) });
			auto compileInfo = JCompileInfo(L"ConvertDisplayColor.hlsl", L"main");

			holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;
			JDx12ComputePso::Create(device, holder, toDisplayRootSignature.Get());
		}
	}
	void JDx12ConvertColor::BuildReverseColorRootSignature(ID3D12Device* device)
	{ 
		JDx12RootSignatureBuilder<Convert::slotCount> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushConstants(Convert::cb32BitCount, 0);
		builder.Create(device, L"ReverseYRootSignature", reverseYRootSignature.GetAddressOf());
	}
	void JDx12ConvertColor::BuildReverseColorPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		reverseY = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputePsoBulder<1> builder; 
		builder.PushHolder(reverseY.get());
		builder.PushCompileInfo(JCompileInfo(L"ConvertTextureComponent.hlsl", L"main"));
		builder.PushMacroSet({REVERSE_Y, L"1"});
		builder.PushThreadDim(JVector3<uint>(Convert::threadDimX, Convert::threadDimY, 1));
		builder.PushRootSignature(reverseYRootSignature.Get());
		builder.Next();
		builder.Create(device);
	}
	void JDx12ConvertColor::ClearResource()
	{
		ClearRootSignature();
		ClearPso();
	}
	void JDx12ConvertColor::ClearRootSignature()
	{
		toLinearRootSignature = nullptr;
		toDisplayRootSignature = nullptr;
		reverseYRootSignature = nullptr;
	}
	void JDx12ConvertColor::ClearPso()
	{ 
		sdrToLienar = nullptr;
		hdrToLienar = nullptr;
		fromSdrToDisplay = nullptr;
		fromHdrToDisplay = nullptr;
		reverseY = nullptr;
	}
}