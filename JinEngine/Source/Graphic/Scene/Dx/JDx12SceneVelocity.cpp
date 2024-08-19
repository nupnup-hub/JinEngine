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


#include"JDx12SceneVelocity.h"  
#include"../../JGraphicUpdateHelper.h"
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h"   
#include"../../Culling/JCullingInfo.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"      
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"  

namespace JinEngine::Graphic
{
	namespace Private
	{
		static constexpr uint useFullscreenQuad = false;
	}
	namespace Velocity
	{
		static constexpr int passCBIndex = 0;
		static constexpr int depthMapIndex = passCBIndex + 1;
		static constexpr int velocityMapIndex = depthMapIndex + 1;
		static constexpr int rootSlotCount = velocityMapIndex + 1;

		static JVector3<uint> GetThreadDim()
		{
			return JVector3<uint>(16, 16, 1);
		}
	}

	template<J_GRAPHIC_RESOURCE_TYPE ...type>
	static bool IsChanged(const const JGraphicInfoChangedSet& set)
	{
		return ((set.preInfo.resource.border[(uint)type] != set.newInfo.resource.border[(uint)type]) | ...);
	}
 
	JDx12SceneVelocity::~JDx12SceneVelocity()
	{
		ClearResource();
	}
	void JDx12SceneVelocity::Initialize(JGraphicDevice* device, JGraphicResourceManager* gm)
	{
		//수정전 024-08-19
		return;
		if (!IsSameDevice(device) || !IsSameDevice(gm))
			return;

		BuildResource(device, gm);
	}
	void JDx12SceneVelocity::Clear()
	{
		//수정전 024-08-19
		return;
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12SceneVelocity::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}   
	void JDx12SceneVelocity::Compute(const JGraphicVelocityComputeSet* set, const JDrawHelper& helper)
	{
		if (!IsSameDevice(set) || !helper.allowTemporalProcess)
			return;

		/*
				const JDx12GraphicVelocityComputeSet* dx12Set = static_cast<const JDx12GraphicVelocityComputeSet*>(set);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12Set->context);
		ResourceDataSet rSet(context, helper);
		if (!rSet.IsValid() || !rSet.velocitySet.IsValid())
			return;
		 
		context->Transition(set.dsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(set.velocitySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->FlushResourceBarriers();

		context->SetComputeRootSignature(velocityRootsignature.Get());
		context->SetPipelineState(velocityShader.get());

		context->SetComputeRootConstantBufferView(Velocity::passCBIndex, J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA, set.camFrameIndex);
		context->SetComputeRootDescriptorTable(Velocity::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Velocity::velocityMapIndex, set.velocitySet.GetGpuUavHandle());

		context->Dispatch2D(set.rtSet.info->GetResourceSize(), velocityShader->dispatchInfo.threadDim.XY());
		*/
	}
	void JDx12SceneVelocity::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSignature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());;
	}
	void JDx12SceneVelocity::BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		velocityRootsignature = nullptr;
		JDx12RootSignatureBuilder<Velocity::rootSlotCount> builder;
		builder.PushConstantsBuffer(Velocity::passCBIndex);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.Create(device, L"Velocity RootSignature", velocityRootsignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12SceneVelocity::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		velocityShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = 1;
		JDx12ComputePsoBulder<shaderCount> psoBuilder("JDx12SceneVelocity");

		psoBuilder.PushHolder(velocityShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::SceneRasterize(L"VelocityBuffer.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Velocity::GetThreadDim());
		psoBuilder.PushRootSignature(velocityRootsignature.Get());
		psoBuilder.Create(device);
	}
	void JDx12SceneVelocity::ClearResource()
	{ 
		velocityShader = nullptr; 
		velocityRootsignature = nullptr;
	}
}