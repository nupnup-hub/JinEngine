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


#include"JDx12Antialise.h"
#include"../JImageProcessingFilter.h"
#include"../../Command/Dx/JDx12CommandContext.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"  
#include"../../Utility/Dx/JDx12ObjectCreation.h"  
#include"../../Utility/Dx/JDx12Utility.h"  
#include"../../JGraphicUpdateHelper.h"   
#include"../../../Core/Math/JVectorExtend.h"
#include<random>

#define USE_LUMA_INPUT_BUFFER L"USE_LUMA_INPUT_BUFFER"
#define VERTICAL_ORIENTATION L"VERTICAL_ORIENTATION"

namespace JinEngine::Graphic
{
	namespace FXAA
	{
		static constexpr uint firstSrvHandleIndex = 0;
		static constexpr uint secondSrvHandleIndex = firstSrvHandleIndex + 1;
		static constexpr uint thirdSrvHandleIndex = secondSrvHandleIndex + 1;
		static constexpr uint firstUavHandleIndex = thirdSrvHandleIndex + 1;
		static constexpr uint secondUavHandleIndex = firstUavHandleIndex + 1;
		static constexpr uint thirdUavHandleIndex = secondUavHandleIndex + 1; 
		static constexpr uint forthUavHandleIndex = thirdUavHandleIndex + 1;
		static constexpr uint cbPassIndex = forthUavHandleIndex + 1;
		static constexpr uint slotCount = cbPassIndex + 1;

		static constexpr uint cb32BitCount = 7;
		static constexpr uint splitCount = 2;

		static JVector3<uint> Pass1ThreadDim()
		{
			return JVector3<uint>(8, 8, 1);
		}
		static JVector3<uint> Pass2ThreadDim()
		{
			return JVector3<uint>(64, 1, 1);
		}
		static JVector3<uint> ResolveThreadDim()
		{
			return JVector3<uint>(64, 1, 1);
		}
	} 
	JDx12Antialise::~JDx12Antialise()
	{
		ClearResource();
	}
	void JDx12Antialise::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		BuildResource(dx12Device, gM);
	}
	void JDx12Antialise::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Antialise::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12Antialise::ApplyAA(JGraphicAAComputeSet* computeSet, const JDrawHelper& helper)
	{ 
	}
	void JDx12Antialise::ApplyFxaa(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		JDx12PostProcessComputeSet* set = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);

		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;

		const bool isInputLuma = helper.option.postProcess.useToneMapping;

		JDx12GraphicResourceComputeSet targetSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		JDx12GraphicResourceComputeSet workCounterSet = context->ComputeSet(imageShare->fxaaWorkCounter);
		JDx12GraphicResourceComputeSet indirectSet = context->ComputeSet(imageShare->fxaaIndirectParameters);
		JDx12GraphicResourceComputeSet workerQueueSet = context->ComputeSet(imageShare->fxaaWorkerQueue);
		JDx12GraphicResourceComputeSet colorQueueSet = context->ComputeSet(imageShare->fxaaColorQueue);
		JDx12GraphicResourceComputeSet lumaSet = context->ComputeSet(imageShare->lumaUnorm);

		const JVector2F targetSize = targetSet.info->GetResourceSize();
		const JVector2F invTargetSize = 1.0f / targetSize;
		//const uint lastQueueIndex = (workerQueueSet.info->GetWidth() / 4) - 1;
		const uint lastQueueIndex = workerQueueSet.info->GetElementCount() - 1;

		context->SetComputeRootSignature(fxaaRootSignature.Get());
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 0, invTargetSize);
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 2, helper.option.postProcess.fxaaContrastThreshold);
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 3, helper.option.postProcess.fxaaSubPixelRemoval);
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 4, lastQueueIndex);
		
		// Apply algorithm to each quarter of the screen separately to reduce maximum size of work buffers.
		const uint blockWidth = targetSize.x / 2;
		const uint blockHeight = targetSize.y / 2;
		const JVector2<uint> blockSize(blockWidth, blockHeight);

		for (uint x = 0; x < 2; x++)
		{
			for (uint y = 0; y < 2; y++)
			{
				JVector2<uint> startPixel(x * blockWidth, y * blockHeight);
				context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 5, startPixel);
		 
				// Begin by analysing the luminance buffer and setting aside high-contrast pixels in
				// work queues to be processed later.  There are horizontal edge and vertical edge work
				// queues so that the shader logic is simpler for each type of edge.
				// Counter values do not need to be reset because they are read and cleared at once.

				context->Transition(targetSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				context->Transition(workerQueueSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				context->Transition(colorQueueSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
 
				JVector3<uint> groupDim = JVector3<uint>::Zero();
				if (isInputLuma)
				{
					context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true); 
					context->SetPipelineState(fxaaPass1InputLuma.get());
					context->SetComputeRootDescriptorTable(FXAA::firstSrvHandleIndex, targetSet.GetGpuSrvHandle());
					context->SetComputeRootDescriptorTable(FXAA::secondSrvHandleIndex, lumaSet.GetGpuSrvHandle());
					context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, workCounterSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::secondUavHandleIndex, workerQueueSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::thirdUavHandleIndex, colorQueueSet.GetGpuUavHandle()); 
					context->Dispatch2D(blockSize, fxaaPass1InputLuma->dispatchInfo.threadDim.XY());
				}
				else
				{
					context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true); 
					context->SetPipelineState(fxaaPass1InputLinearColor.get());
					context->SetComputeRootDescriptorTable(FXAA::firstSrvHandleIndex, targetSet.GetGpuSrvHandle());
					context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, workCounterSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::secondUavHandleIndex, workerQueueSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::thirdUavHandleIndex, colorQueueSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::forthUavHandleIndex, lumaSet.GetGpuUavHandle());
					context->Dispatch2D(blockSize, fxaaPass1InputLinearColor->dispatchInfo.threadDim.XY());
					context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				}    			 
			 
				context->SetPipelineState(fxaaResolveWork.get());
				context->Transition(indirectSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); 
				context->InsertUAVBarrier(workCounterSet.holder, true);

				context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, indirectSet.GetGpuUavHandle());
				context->SetComputeRootDescriptorTable(FXAA::secondUavHandleIndex, workerQueueSet.GetGpuUavHandle());
				context->SetComputeRootDescriptorTable(FXAA::thirdUavHandleIndex, workCounterSet.GetGpuUavHandle());
				context->Dispatch(1, 1, 1);

				context->InsertUAVBarrier(workCounterSet.holder);
				context->Transition(indirectSet.holder, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
				context->Transition(workerQueueSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				context->Transition(colorQueueSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				context->Transition(targetSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
			 
				context->SetComputeRootDescriptorTable(FXAA::firstSrvHandleIndex, lumaSet.GetGpuSrvHandle());
				context->SetComputeRootDescriptorTable(FXAA::secondSrvHandleIndex, workerQueueSet.GetGpuSrvHandle());
				context->SetComputeRootDescriptorTable(FXAA::thirdSrvHandleIndex, colorQueueSet.GetGpuSrvHandle());
				context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, targetSet.GetGpuUavHandle());
				
				context->SetPipelineState(fxaaPass2Horizontal.get());
				context->DispatchIndirect(fxaaCommandSignature.Get(), indirectSet, 0); 
				context->SetPipelineState(fxaaPass2Vertical.get());
				context->DispatchIndirect(fxaaCommandSignature.Get(), indirectSet, 12);

				context->InsertUAVBarrier(targetSet.holder);
			}
		} 
	}
	void JDx12Antialise::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		ID3D12Device* device = static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice();
		ClearPso();
		BuildPso(device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12Antialise::BuildResource(JDx12GraphicDevice* device, JGraphicResourceManager* gM)
	{
		BuildRootSingnature(device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(device->GetDevice(), GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12Antialise::BuildRootSingnature(JDx12GraphicDevice* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildFxaaRootSignature(device->GetDevice());
		BuildFxaaCommandSignature(device->GetDevice());
	}
	void JDx12Antialise::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		BuildFxaaPso(device, info);
	}
	void JDx12Antialise::BuildFxaaRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<FXAA::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		builder.PushConstants(FXAA::cb32BitCount, 0);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		builder.Create(device, L"FxaaMappingRootSignature", fxaaRootSignature.GetAddressOf());
	}
	void JDx12Antialise::BuildFxaaCommandSignature(ID3D12Device* device)
	{ 
		uint byteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
		  
		D3D12_INDIRECT_ARGUMENT_DESC indirectDesc;
		indirectDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc;
		commandSignatureDesc.ByteStride = byteStride;
		commandSignatureDesc.NumArgumentDescs = 1;
		commandSignatureDesc.pArgumentDescs = &indirectDesc;
		commandSignatureDesc.NodeMask = 1;
		 
		ThrowIfFailedG(device->CreateCommandSignature(&commandSignatureDesc, nullptr,
			IID_PPV_ARGS(&fxaaCommandSignature)));

		fxaaCommandSignature->SetName(L"FxaaCommandSignature");
	}
	void JDx12Antialise::BuildFxaaPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		fxaaPass1InputLinearColor = std::make_unique<JDx12ComputeShaderDataHolder>();
		fxaaPass1InputLuma = std::make_unique<JDx12ComputeShaderDataHolder>();
		fxaaPass2Vertical = std::make_unique< JDx12ComputeShaderDataHolder>();
		fxaaPass2Horizontal = std::make_unique<JDx12ComputeShaderDataHolder>();
		fxaaResolveWork = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = 5;
		JDx12ComputeShaderDataHolder* holderSet[shaderCount]
		{
			fxaaPass1InputLinearColor.get(),
			fxaaPass1InputLuma.get(),
			fxaaPass2Vertical.get(),
			fxaaPass2Horizontal.get(),
			fxaaResolveWork.get()
		};
		JCompileInfo compileInfoSet[shaderCount]
		{ 
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass1.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass1.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass2.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass2.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaResolveWork.hlsl"), L"main")
		};
		std::vector<JMacroSet> macroSet[shaderCount]
		{
			std::vector<JMacroSet>{},
			std::vector<JMacroSet>{JMacroSet{USE_LUMA_INPUT_BUFFER, std::to_wstring(1) }},
			std::vector<JMacroSet>{JMacroSet{VERTICAL_ORIENTATION, std::to_wstring(1) }},
			std::vector<JMacroSet>{},
			std::vector<JMacroSet>{}
		};
		JVector3<uint> threadDim[shaderCount]
		{
			FXAA::Pass1ThreadDim(),
			FXAA::Pass1ThreadDim(),
			FXAA::Pass2ThreadDim(),
			FXAA::Pass2ThreadDim(),
			FXAA::ResolveThreadDim()
		};

		for (uint i = 0; i < shaderCount; ++i)
		{
			JDx12ComputeShaderDataHolder* holder = holderSet[i];
			JComputeShaderInitData initData;
			initData.dispatchInfo.threadDim = threadDim[i];
			initData.macro = macroSet[i];
			initData.PushThreadDimensionMacro();

			holder->cs = JDxShaderDataUtil::CompileShader(compileInfoSet[i].filePath, initData.macro, compileInfoSet[i].functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;

			JDx12ComputePso::Create(device, holder, fxaaRootSignature.Get(), D3D12_PIPELINE_STATE_FLAG_NONE);
		}
	}
	void JDx12Antialise::ClearResource()
	{
		ClearRootSignature();
		ClearPso();
	}
	void JDx12Antialise::ClearRootSignature()
	{
		fxaaRootSignature = nullptr;
	}
	void JDx12Antialise::ClearPso()
	{
		fxaaPass1InputLinearColor = nullptr;
		fxaaPass1InputLuma = nullptr;

		fxaaPass2Vertical = nullptr;
		fxaaPass2Horizontal = nullptr;
		fxaaResolveWork = nullptr;
	}
}