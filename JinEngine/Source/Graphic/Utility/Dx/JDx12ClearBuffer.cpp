#include"JDx12ClearBuffer.h"
#include"JDx12ObjectCreation.h" 
#include"JDx12Utility.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h" 
#include"../../Command/Dx/JDx12CommandContext.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../Core/Math/JMathHelper.h"

#define INDEX_PER_BYTE L"INDEX_PER_BYTE"
#define OFFER_BUFFER_CLEAR_VALUE L"OFFER_BUFFER_CLEAR_VALUE"
namespace JinEngine::Graphic
{
	namespace Private
	{
		static constexpr uint clearBufferHandleIndex = 0;
		static constexpr uint passCBIndex = clearBufferHandleIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;
		static constexpr uint constants32BitCount = 1;
	}
	JDx12ClearBufferUseCs::~JDx12ClearBufferUseCs()
	{
		Clear();
	}
	void JDx12ClearBufferUseCs::Initialize(ID3D12Device* device, 
		const JVector3<uint> threadDim,
		const uint dataSize,
		const size_t clearValue)
	{
		JDx12RootSignatureBuilder<Private::slotCount> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); 
		builder.PushConstants(Private::constants32BitCount, 0);
		builder.Create(device, L"ClearRootSignature", rootSignature.GetAddressOf());
		 
		clear = std::make_unique<JDx12ComputeShaderDataHolder>();
		JDx12ComputeShaderDataHolder* holder = clear.get();

		JComputeShaderInitData initData;
		initData.dispatchInfo.threadDim = threadDim;
		initData.PushThreadDimensionMacro();
		initData.macro.push_back({ INDEX_PER_BYTE, std::to_wstring(dataSize) });
		initData.macro.push_back({ OFFER_BUFFER_CLEAR_VALUE, std::to_wstring(clearValue) });

		auto compileInfo = JCompileInfo(L"ClearBuffer.hlsl", L"ClearBuffer");
		holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
		holder->dispatchInfo = initData.dispatchInfo;

		JDx12ComputePso::Create(device, holder, rootSignature.Get(), D3D12_PIPELINE_STATE_FLAG_NONE);
		JDx12ClearBufferUseCs::dataSize = dataSize;
	}
	void JDx12ClearBufferUseCs::Clear()
	{
		rootSignature = nullptr;
		clear = nullptr;
	}  
	void JDx12ClearBufferUseCs::Execute(JDx12CommandContext* context, const JUserPtr<JGraphicResourceInfo>& info)
	{
		if (context == nullptr || !info.IsValid())
			return;

		JDx12GraphicResourceComputeSet resourceSet = context->ComputeSet(info);
		uint range = JMathHelper::DivideByMultiple<uint>(info->GetWidth(), dataSize);
		 
		context->SetComputeRootSignature(rootSignature.Get());
		context->SetComputeRootDescriptorTable(Private::clearBufferHandleIndex, resourceSet.GetGpuUavHandle());
		context->SetComputeRoot32BitConstants(Private::passCBIndex, 0, range);
		context->SetPipelineState(clear.get());
		 	 
		context->Dispatch1D(range, clear->dispatchInfo.threadDim.x);
	}
	void JDx12ClearBufferUseCs::Execute(JDx12CommandContext* context, const JUserPtr<JGraphicResourceInfo>& info, const D3D12_RESOURCE_STATES beforeState)
	{
		if (context == nullptr || !info.IsValid())
			return;

		JDx12GraphicResourceComputeSet resourceSet = context->ComputeSet(info);
		context->Transition(resourceSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true); 
		Execute(context, info);
		context->Transition(resourceSet.holder, beforeState); 
	}
}