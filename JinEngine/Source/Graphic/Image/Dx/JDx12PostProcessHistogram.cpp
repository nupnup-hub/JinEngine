#include"JDx12PostProcessHistogram.h"  
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"  
#include"../../Command/Dx/JDx12CommandContext.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"  
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../Utility/Dx/JDx12ClearBuffer.h"
#include"../../JGraphicUpdateHelper.h" 
#include"../../../Core/Platform/JHardwareInfo.h" 
#include"../../../Core/Math/JMathHelper.h" 
#include"../../../Object/Component/Camera/JCamera.h"   
//#include"../../../Develop/Debug/JDevelopDebug.h"

#define HISTOGRAM_COUNT L"HISTOGRAM_COUNT"

namespace JinEngine::Graphic
{
	namespace Histogram
	{
		static constexpr uint lumaHandleIndex = 0;
		static constexpr uint histogramHandleIndex = lumaHandleIndex + 1;
		static constexpr uint passCBIndex = histogramHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;

		static constexpr uint cb32BitCount = 1;
		static constexpr uint threadDimX = 16;
		static constexpr uint threadDimY = 16;
		 
		static uint ThreadDimXY()
		{
			return sqrt(Constants::histogramBufferCount);
		} 
	}
	namespace Debug
	{
		static constexpr uint histogramHandleIndex = 0;
		static constexpr uint exposureHandleIndex = histogramHandleIndex + 1;
		static constexpr uint srcHandleIndex = exposureHandleIndex + 1;
		static constexpr uint slotCount = srcHandleIndex + 1;
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

	JDx12PostProcessHistogram::~JDx12PostProcessHistogram()
	{
		ClearResource();
	}
	void JDx12PostProcessHistogram::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM); 
	}
	void JDx12PostProcessHistogram::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12PostProcessHistogram::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12PostProcessHistogram::CreateHistogram(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12PostProcessComputeSet* set = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);
		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;
		 
		JDx12GraphicResourceComputeSet lumaSet = context->ComputeSet(imageShare->lumaLowResolutionUint);
		JDx12GraphicResourceComputeSet histogramSet = context->ComputeSet(imageShare->histogram);
 
		const JVector2<uint> lumaSize = lumaSet.info->GetResourceSize();
		context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(histogramSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
 
		clearHistogram->Execute(context, imageShare->histogram);
		context->SetComputeRootSignature(createHistogramRootSignature.Get());
		context->SetComputeRootDescriptorTable(Histogram::lumaHandleIndex, lumaSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Histogram::histogramHandleIndex, histogramSet.GetGpuUavHandle());
		context->SetComputeRoot32BitConstants(Histogram::passCBIndex, 0, lumaSize.y);

		auto shaderData = createHistogram.get();
		auto threadDim = shaderData->dispatchInfo.threadDim;
		   
		context->SetPipelineState(shaderData);
		context->Dispatch1D(lumaSize.x, threadDim.x);
	}
	void JDx12PostProcessHistogram::DrawHistogram(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
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
		JDx12GraphicResourceComputeSet exposureSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::POST_PROCESS_EXPOSURE, J_GRAPHIC_TASK_TYPE::APPLY_TONE_MAPPING);
		JDx12GraphicResourceComputeSet srcSet = context->ComputeSet(gUser, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::APPLY_POST_PROCESS_RESULT);
		if (helper.option.postProcess.exposureType == J_EXPOSURE_TYPE::DEFUALT)
			exposureSet = context->ComputeSet(imageShare->defaultExposure);

		context->Transition(histogramSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(exposureSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(srcSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
		 
		context->SetComputeRootSignature(drawHistogramRootSignature.Get());
		context->SetComputeRootDescriptorTable(Debug::histogramHandleIndex, histogramSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Debug::exposureHandleIndex, exposureSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Debug::srcHandleIndex, srcSet.GetGpuUavHandle());

		auto shaderData = drawHistogram.get();
		auto threadDim = shaderData->dispatchInfo.threadDim;

		context->SetPipelineState(shaderData);
		context->Dispatch(1, 32, 1);
	}
	void JDx12PostProcessHistogram::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());

		clearHistogram = std::make_unique<JDx12ClearBufferUseCs>();
		clearHistogram->Initialize(d3d12Device, JVector3<uint>(Constants::histogramBufferCount, 1, 1), sizeof(uint32), 0);
	}
	void JDx12PostProcessHistogram::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildHistogramRootSignature(device); 
	}
	void JDx12PostProcessHistogram::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildHistogramPso(device, info); 
	}
	void JDx12PostProcessHistogram::BuildHistogramRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<Histogram::slotCount> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushConstants(Histogram::cb32BitCount, 0);
		builder.Create(device, L"CreateHistogramRootSIgnature", createHistogramRootSignature.GetAddressOf());
	
		builder.Clear();
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); 
		builder.Create(device, L"DrawHistogramRootSIgnature", drawHistogramRootSignature.GetAddressOf());
	}
	void JDx12PostProcessHistogram::BuildHistogramPso(ID3D12Device* device, const JGraphicInfo& info)
	{
		auto createPsoLam = [](JDx12ComputeShaderDataHolder* holder, 
			ID3D12Device* device,
			ID3D12RootSignature* root, 
			const JVector3F dim,  
			const JCompileInfo& compileInfo)
		{
			JComputeShaderInitData initData; 
			initData.dispatchInfo.threadDim = dim;
			initData.PushThreadDimensionMacro();
			initData.macro.push_back({ HISTOGRAM_COUNT, std::to_wstring(Constants::histogramBufferCount) });
 
			holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;

			JDx12ComputePso::Create(device, holder, root);
		};

		createHistogram = std::make_unique<JDx12ComputeShaderDataHolder>();
		drawHistogram = std::make_unique<JDx12ComputeShaderDataHolder>();

		createPsoLam(createHistogram.get(),
			device,
			createHistogramRootSignature.Get(),
			JVector3F(Histogram::ThreadDimXY(), Histogram::ThreadDimXY(), 1),
			JCompileInfo(L"PostProcessHistogram.hlsl", L"CreateHistgram"));

		createPsoLam(drawHistogram.get(),
			device,
			drawHistogramRootSignature.Get(),
			JVector3F(Constants::histogramBufferCount, 1, 1),
			JCompileInfo(L"PostProcessDrawHistogram.hlsl", L"DrawHistogram"));
	}  
	void JDx12PostProcessHistogram::ClearResource()
	{
		ClearRootSignature();
		ClearPso();
		clearHistogram = nullptr;
	}
	void JDx12PostProcessHistogram::ClearRootSignature()
	{
		createHistogramRootSignature = nullptr;
		drawHistogramRootSignature = nullptr;
	}
	void JDx12PostProcessHistogram::ClearPso()
	{
		createHistogram = nullptr;
		drawHistogram = nullptr;
	}
}