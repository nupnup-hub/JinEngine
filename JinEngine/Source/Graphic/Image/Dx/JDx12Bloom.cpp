#include"JDx12Bloom.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"   
#include"../../GraphicResource/Dx/JDx12GraphicResourceHolder.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"  
#include"../../Utility/Dx/JDx12Utility.h"  
#include"../../JGraphicUpdateHelper.h"  
#include"../../../Core/Platform/JHardwareInfo.h" 
#include"../../../Core/Math/JMathHelper.h" 
#include"../../../Core/Math/JVectorExtend.h" 
#include"../../../Object/Component/Camera/JCamera.h"

#define EXTRACT_LUMA L"EXTRACT_LUMA"
#define DOWN_SAMPLE_2 L"DOWN_SAMPLE_2"
#define DOWN_SAMPLE_4 L"DOWN_SAMPLE_4"

namespace JinEngine::Graphic
{
	namespace Bloom
	{
		static constexpr uint firstSrvHandleIndex = 0;
		static constexpr uint secondSrvHandleIndex = firstSrvHandleIndex + 1;
		static constexpr uint firstUavHandleIndex = secondSrvHandleIndex + 1;
		static constexpr uint secondUavHandleIndex = firstUavHandleIndex + 1;
		static constexpr uint thirdUavHandleIndex = secondUavHandleIndex + 1;
		static constexpr uint forthUavHandleIndex = thirdUavHandleIndex + 1;
		static constexpr uint cbPassIndex = forthUavHandleIndex + 1;
		static constexpr uint slotCount = cbPassIndex + 1;

		static constexpr uint cb32BitCount = 4;
		static constexpr uint threadDimX = 8;
		static constexpr uint threadDimY = 8;
	}

	JDx12Bloom::BloomDataSet::BloomDataSet(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
		:helper(helper), useHighQuality(helper.option.postProcess.bloomQuality == J_BLOOM_QUALITY::HIGH)
	{
		JDx12PostProcessComputeSet* set = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		context = static_cast<JDx12CommandContext*>(set->context);
		imageShare = static_cast<ImageProcessingShareData*>(set->imageShareData);
		if (imageShare == nullptr)
			return;

		auto gUser = helper.cam->GraphicResourceUserInterface();
		exposureSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE, J_GRAPHIC_TASK_TYPE::APPLY_TONE_MAPPING);
		srcSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		lumaSet = context->ComputeSet(imageShare->lumaLowResolutionUint);
 
		for (uint i = 0; i < Constants::bloomSampleCount; ++i)
		{
			bloomSet[i][0] = context->ComputeSet(imageShare->bloom[i][0]);
			bloomSet[i][1] = context->ComputeSet(imageShare->bloom[i][1]);
		} 
		bloomSize = lumaSet.info->GetResourceSize();
		isValid = true;
	}
	bool JDx12Bloom::BloomDataSet::IsValid() const noexcept
	{
		return isValid;
	}

	JDx12Bloom::~JDx12Bloom()
	{
		ClearResource();
	}
	void JDx12Bloom::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12Bloom::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Bloom::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12Bloom::ApplyBloom(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;
  
		BloomDataSet set(computeSet, helper);
		if (!set.IsValid())
			return;

		BeginBloom(set);
		ExtractLumaAndDonwSample(set);
		DownSample(set);
		UpSample(set);
		EndBloom(set);
	}
	void JDx12Bloom::BeginBloom(const BloomDataSet& set)
	{
		set.context->SetComputeRootSignature(rootSignature.Get());
	}
	void JDx12Bloom::ExtractLumaAndDonwSample(const BloomDataSet& set)
	{ 
		bool canExractLuma = set.helper.option.postProcess.exposureType == J_EXPOSURE_TYPE::AUTO;
		set.context->Transition(set.srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.exposureSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.bloomSet[0][0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		if (canExractLuma)
			set.context->Transition(set.lumaSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		const JVector2F invSize = 1.0f / set.bloomSize;
		const float threadHold = set.helper.option.postProcess.bloomThreadHold;
		set.context->SetComputeRootDescriptorTable(Bloom::firstSrvHandleIndex, set.srcSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Bloom::secondSrvHandleIndex, set.exposureSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Bloom::firstUavHandleIndex, set.bloomSet[0][0].GetGpuUavHandle());
		if (canExractLuma)
			set.context->SetComputeRootDescriptorTable(Bloom::secondUavHandleIndex, set.lumaSet.GetGpuUavHandle());
		set.context->SetComputeRoot32BitConstants(Bloom::cbPassIndex, 0, invSize);
		set.context->SetComputeRoot32BitConstants(Bloom::cbPassIndex, 2, threadHold);
		
		auto shaderData = canExractLuma ? bloomExtractAndExtractLuma.get() : bloomExtract.get(); 
		set.context->SetPipelineState(shaderData);
		set.context->Dispatch2D(set.bloomSize, shaderData->dispatchInfo.threadDim.XY());

		//JDx12Utility::ResourceTransition(set.cmdList, set.srcSet.resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
		//JDx12Utility::ResourceTransition(set.cmdList, set.exposureSet.resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON);
		//JDx12Utility::ResourceTransition(set.cmdList, set.bloomSet[0][0].resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		//if (canExractLuma)
		//	JDx12Utility::ResourceTransition(set.cmdList, set.lumaSet.resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
	}
	void JDx12Bloom::DownSample(const BloomDataSet& set)
	{
		set.context->Transition(set.bloomSet[0][0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->SetComputeRootDescriptorTable(Bloom::firstSrvHandleIndex, set.bloomSet[0][0].GetGpuSrvHandle());
		if (set.useHighQuality)
		{
			set.context->Transition(set.bloomSet[1][0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			set.context->Transition(set.bloomSet[2][0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			set.context->Transition(set.bloomSet[3][0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			set.context->Transition(set.bloomSet[4][0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
 
			set.context->SetComputeRootDescriptorTable(Bloom::firstUavHandleIndex, set.bloomSet[1][0].GetGpuUavHandle());
			set.context->SetComputeRootDescriptorTable(Bloom::secondUavHandleIndex, set.bloomSet[2][0].GetGpuUavHandle());
			set.context->SetComputeRootDescriptorTable(Bloom::thirdUavHandleIndex, set.bloomSet[3][0].GetGpuUavHandle());
			set.context->SetComputeRootDescriptorTable(Bloom::forthUavHandleIndex, set.bloomSet[4][0].GetGpuUavHandle());

			auto shaderData = downSample4.get(); 
			set.context->SetPipelineState(shaderData);
			set.context->Dispatch2D(set.bloomSize / 2, shaderData->dispatchInfo.threadDim.XY());
		}
		else
		{ 
			set.context->Transition(set.bloomSet[2][0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); 
			set.context->Transition(set.bloomSet[4][0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
			 
			set.context->SetComputeRootDescriptorTable(Bloom::firstUavHandleIndex, set.bloomSet[2][0].GetGpuUavHandle());
			set.context->SetComputeRootDescriptorTable(Bloom::secondUavHandleIndex, set.bloomSet[4][0].GetGpuUavHandle());
		 
			auto shaderData = downSample2.get(); 
			set.context->SetPipelineState(shaderData);
			set.context->Dispatch2D(set.bloomSize / 2, shaderData->dispatchInfo.threadDim.XY());
		}
	}
	void JDx12Bloom::UpSample(const BloomDataSet& set)
	{
		if (set.useHighQuality)
		{
			set.context->Transition(set.bloomSet[4][0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			set.context->Transition(set.bloomSet[3][0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			set.context->Transition(set.bloomSet[2][0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			set.context->Transition(set.bloomSet[1][0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			float upsampleBlendFactor = set.helper.option.postProcess.bloomUpsampleFactor;
			UpSample(set, 4, set.bloomSet[4][0], 1.0f);
			UpSample(set, 3, set.bloomSet[4][1], upsampleBlendFactor);
			UpSample(set, 2, set.bloomSet[3][1], upsampleBlendFactor);
			UpSample(set, 1, set.bloomSet[2][1], upsampleBlendFactor);
			UpSample(set, 0, set.bloomSet[1][1], upsampleBlendFactor); 
		}
		else
		{
			set.context->Transition(set.bloomSet[4][0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
			set.context->Transition(set.bloomSet[2][0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			float upsampleBlendFactor = set.helper.option.postProcess.bloomUpsampleFactor * 2.0f / 3.0f;
			UpSample(set, 4, set.bloomSet[4][0], 1.0f); 
			UpSample(set, 2, set.bloomSet[4][1], upsampleBlendFactor); 
			UpSample(set, 0, set.bloomSet[2][1], upsampleBlendFactor);
		} 
	}
	void JDx12Bloom::UpSample(const BloomDataSet& set, const uint index, const JDx12GraphicResourceComputeSet& lowerRefBuf, float upsampleBlendFactor)
	{
		const JVector2F size = set.bloomSet[index][0].info->GetResourceSize();
		const JVector2F invSize = 1.0f / size;

		set.context->SetComputeRoot32BitConstants(Bloom::cbPassIndex, 0, invSize);
		set.context->SetComputeRoot32BitConstants(Bloom::cbPassIndex, 2, set.helper.option.postProcess.bloomThreadHold);

		set.context->Transition(set.bloomSet[index][1].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
 
		set.context->SetComputeRootDescriptorTable(Bloom::firstSrvHandleIndex, set.bloomSet[index][0].GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Bloom::secondSrvHandleIndex, lowerRefBuf.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Bloom::firstUavHandleIndex, set.bloomSet[index][1].GetGpuUavHandle());
		   
		auto shaderData = set.bloomSet[index][0].info == lowerRefBuf.info ? blur.get() : upSample.get();
		 
		set.context->SetPipelineState(shaderData);
		set.context->Dispatch2D(size, shaderData->dispatchInfo.threadDim.XY());

		set.context->Transition(set.bloomSet[index][1].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	void JDx12Bloom::EndBloom(const BloomDataSet& set)
	{

	}
	void JDx12Bloom::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		ID3D12Device* device = static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice();
		ClearPso();
		BuildPso(device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12Bloom::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12Bloom::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildBloomRootSignature(device);
	}
	void JDx12Bloom::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildBloomPso(device, info);
	}
	void JDx12Bloom::BuildBloomRootSignature(ID3D12Device* device)
	{   
		JDx12RootSignatureBuilder2<Bloom::slotCount, 2> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		builder.PushConstants(Bloom::cb32BitCount, 0);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
		builder.Create(device, L"BloomRootSignature", rootSignature.GetAddressOf());
	}
	void JDx12Bloom::BuildBloomPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		bloomExtract = std::make_unique<JDx12ComputeShaderDataHolder>();
		bloomExtractAndExtractLuma = std::make_unique<JDx12ComputeShaderDataHolder>();
		blur = std::make_unique< JDx12ComputeShaderDataHolder>();
		upSample = std::make_unique<JDx12ComputeShaderDataHolder>();
		downSample2 = std::make_unique<JDx12ComputeShaderDataHolder>();
		downSample4 = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputeShaderDataHolder* holderSet[6]
		{
			bloomExtract.get(), 
			bloomExtractAndExtractLuma.get(),
			blur.get(), 
			upSample.get(), 
			downSample2.get(),
			downSample4.get()
		};
		JCompileInfo compileInfoSet[6]
		{
			JCompileInfo(L"PostProcessBloomExtract.hlsl", L"main"), 
			JCompileInfo(L"PostProcessBloomExtract.hlsl", L"main"),
			JCompileInfo(L"PostProcessBloomBlur.hlsl", L"main"),
			JCompileInfo(L"PostProcessBloomUpSample.hlsl", L"main"),
			JCompileInfo(L"PostProcessBloomDownSample.hlsl", L"main"),
			JCompileInfo(L"PostProcessBloomDownSample.hlsl", L"main")
		};
		std::vector<JMacroSet> macroSet[6]
		{
			std::vector<JMacroSet>{},
			std::vector<JMacroSet>{JMacroSet{EXTRACT_LUMA, std::to_wstring(1) }},
			std::vector<JMacroSet>{},
			std::vector<JMacroSet>{},
			std::vector<JMacroSet>{JMacroSet{DOWN_SAMPLE_2, std::to_wstring(1) }},
			std::vector<JMacroSet>{JMacroSet{DOWN_SAMPLE_4, std::to_wstring(1) }},
		};

		for (uint i = 0; i < 6; ++i)
		{
			JDx12ComputeShaderDataHolder* holder = holderSet[i];
			JComputeShaderInitData initData;
			initData.dispatchInfo.threadDim = JVector3F(Bloom::threadDimX, Bloom::threadDimY, 1);
			initData.macro = macroSet[i];
			initData.PushThreadDimensionMacro();
			  
			holder->cs = JDxShaderDataUtil::CompileShader(compileInfoSet[i].filePath, initData.macro, compileInfoSet[i].functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;

			JDx12ComputePso::Create(device, holder, rootSignature.Get(), D3D12_PIPELINE_STATE_FLAG_NONE);
		}
		 
	}
	void JDx12Bloom::ClearResource()
	{
		ClearRootSignature();
		ClearPso();
	}
	void JDx12Bloom::ClearRootSignature()
	{
		rootSignature = nullptr;
	}
	void JDx12Bloom::ClearPso()
	{
		bloomExtract = nullptr;
		bloomExtractAndExtractLuma = nullptr;
		blur = nullptr;
		upSample = nullptr;
		downSample2 = nullptr;
		downSample4 = nullptr;
	}
}
