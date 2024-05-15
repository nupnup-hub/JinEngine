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


#include"JDx12DownSampling.h"
#include"../JBlur.h"
#include"../JImageProcessingFilter.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"  
#include"../../Utility/Dx/JDx12ObjectCreation.h" 
#include"../../JGraphicUpdateHelper.h"   
//#include"../../../Develop/Debug/JDevelopDebug.h"
 
#define EXIST_SYMBOL L"EXIST"
#define WIDTH_DOWN_SYMBOL L"WIDTH_DOWN"
#define HEIGHT_DOWN_SYMBOL L"HEIGHT_DOWN"
#define WIDTH_HEIGHT_DOWN_SYMBOL L"WIDTH_HEIGHT_DOWN"
 
namespace JinEngine::Graphic
{ 
	namespace DownSample
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint downSamplePassCBIndex = destTextureHandleIndex + 1; 
		static constexpr uint slotCount = downSamplePassCBIndex + 1;
		static constexpr uint cbPass32BitCount = 5;

		static constexpr uint threadDimX = 16;
		static constexpr uint threadDimY = 16;
	} 
	namespace Private
	{ 
		static void StuffComputeShaderDispatchInfo(_Inout_ JComputeShaderInitData& initHelper, const JGraphicInfo& graphicInfo)
		{
			using GpuInfo = Core::JHardwareInfo::GpuInfo;
			std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();
			 
			initHelper.dispatchInfo.threadDim = JVector3<uint>(DownSample::threadDimX, DownSample::threadDimY, 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>::Zero();
			initHelper.dispatchInfo.taskOriCount = 0;	//unknown;
		}
		static std::vector<D3D12_INPUT_ELEMENT_DESC> SsaoInputLayout()
		{
			return
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
		}
	}
	 
	JDx12DownSampling::JDx12DownSampling()
		:guid(Core::MakeGuid())
	{}
	JDx12DownSampling::~JDx12DownSampling()
	{
		ClearResource();
	}
	void JDx12DownSampling::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12DownSampling::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12DownSampling::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	void JDx12DownSampling::ApplyMipmapGeneration(JGraphicDownSampleComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet))
			return;

		JDx12GraphicDownSampleComputeSet* set = static_cast<JDx12GraphicDownSampleComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);
 
		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = set->srcHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = set->destHandle;
		uint srvSize = context->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::SRV);
		uint uavSize = context->GetDescriptorSize(J_GRAPHIC_BIND_TYPE::UAV);

		JVector2<uint> srcSize = set->desc->imageSize;
		JVector2<uint> destSize = srcSize / 2.0f;
		auto rSet = context->ComputeSet(set->info);
		D3D12_RESOURCE_STATES beforeState = (D3D12_RESOURCE_STATES)-1;

		if (rSet.IsValid())
		{
			beforeState = rSet.holder->GetState();
			context->Transition(rSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
		}
		context->SetComputeRootSignature(downSampleRootSignature.Get());
		const uint mipLevelCount = set->desc->mipLevelCount;
		for (uint i = 1; i < mipLevelCount; ++i)
		{
			uint mipLevel = i - 1; 

			JDx12ComputeShaderDataHolder* holder = nullptr;
			if (srcSize.x == 1 && srcSize.y == 1)
				holder = downSample[(uint)DOWN_SAMPLE_STATE::EXIST].get();
			else if (srcSize.y == 1)
				holder = downSample[(uint)DOWN_SAMPLE_STATE::WIDTH_DOWN].get();
			else if (srcSize.x == 1)
				holder = downSample[(uint)DOWN_SAMPLE_STATE::HEIGHT_DOWN].get();
			else
				holder = downSample[(uint)DOWN_SAMPLE_STATE::WIDTH_HEIGHT_DOWN].get();

			auto dispatchInfo = holder->dispatchInfo; 
			context->SetPipelineState(holder);
	  
			context->SetComputeRootDescriptorTable(DownSample::srcTextureHandleIndex, srcHandle);
			context->SetComputeRootDescriptorTable(DownSample::destTextureHandleIndex, destHandle);
			context->SetComputeRoot32BitConstants(DownSample::downSamplePassCBIndex, 0, srcSize);
			context->SetComputeRoot32BitConstants(DownSample::downSamplePassCBIndex, 2, destSize);
			context->SetComputeRoot32BitConstants(DownSample::downSamplePassCBIndex, 4, mipLevel);

			context->Dispatch2D(destSize, dispatchInfo.threadDim.XY());
			srcHandle.Offset(srvSize);
			destHandle.Offset(uavSize);
			srcSize /= 2;
			destSize /= 2;

			srcSize = JVector2F::Clamp(srcSize, JVector2F(1.0f, 1.0f), srcSize);
			destSize = JVector2F::Clamp(destSize, JVector2F(1.0f, 1.0f), destSize);
		}

		if (rSet.IsValid())
		{
			context->Transition(rSet.holder, beforeState);
			context->InsertUAVBarrier(rSet.holder);
		}
	}
	void JDx12DownSampling::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSingnature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12DownSampling::BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildDownSampleRootSignature(device);
	}
	void JDx12DownSampling::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildDownSamplePso(device, info);
	} 
	void JDx12DownSampling::BuildDownSampleRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<DownSample::slotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); 
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushConstants(DownSample::cbPass32BitCount, 0);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		builder.Create(device, L"DownSampleRootSignature", downSampleRootSignature.GetAddressOf());
	}
	void JDx12DownSampling::BuildDownSamplePso(ID3D12Device* device, const JGraphicInfo& info)
	{
		for (uint i = 0; i < (uint)DOWN_SAMPLE_STATE::COUNT; ++i)
		{
			const DOWN_SAMPLE_STATE type = (DOWN_SAMPLE_STATE)i;
			downSample[i] = std::make_unique<JDx12ComputeShaderDataHolder>();
			JDx12ComputeShaderDataHolder* holder = downSample[i].get();
 
			JComputeShaderInitData initData;
			Private::StuffComputeShaderDispatchInfo(initData, info);
			initData.PushThreadDimensionMacro();
			if (type == DOWN_SAMPLE_STATE::EXIST)
				initData.macro.push_back({ EXIST_SYMBOL, std::to_wstring(1) });
			else if (type == DOWN_SAMPLE_STATE::WIDTH_DOWN)
				initData.macro.push_back({ WIDTH_DOWN_SYMBOL, std::to_wstring(1) });
			else if (type == DOWN_SAMPLE_STATE::HEIGHT_DOWN)
				initData.macro.push_back({ HEIGHT_DOWN_SYMBOL, std::to_wstring(1) });
			else if (type == DOWN_SAMPLE_STATE::WIDTH_HEIGHT_DOWN)
				initData.macro.push_back({ WIDTH_HEIGHT_DOWN_SYMBOL, std::to_wstring(1) });

			auto compileInfo = JCompileInfo(ShaderRelativePath::Image(L"DownSampling.hlsl"), L"DownSample");

			holder->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initData.macro, compileInfo.functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;
			JDx12ComputePso::Create(device, holder, downSampleRootSignature.Get());
		}
	}
	void JDx12DownSampling::ClearResource()
	{
		ClearRootSignature();
		ClearPso(); 
	}
	void JDx12DownSampling::ClearRootSignature()
	{
		downSampleRootSignature = nullptr;
	}
	void JDx12DownSampling::ClearPso()
	{ 
		for (uint i = 0; i < SIZE_OF_ARRAY(downSample); ++i)
			downSample[i] = nullptr; 
	} 
}
 