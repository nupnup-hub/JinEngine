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


#include"JDx12SceneDependencyData.h"  
#include"../../JGraphicUpdateHelper.h"
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h"   
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../Culling/JCullingInfo.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"      
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"  
#include"../../../Object/Component/Camera/JCamera.h"

namespace JinEngine::Graphic
{
	namespace Private
	{
		static constexpr uint useFullscreenQuad = false;
	}
	namespace Depth
	{
		ROOT_INDEX_CREATOR(, camCBIndex, depthMapIndex, preDepthMapIndex, viewZMapIndex, preViewZMapIndex, depthDerivativeMapIndex)
 
		static constexpr int cb32BitCount = 16;
		static JVector3<uint> GetThreadDim()
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Velocity
	{
		ROOT_INDEX_CREATOR(, camCBIndex, passCBIndex, viewZMapIndex, velocityMapIndex)
 
		static constexpr int cb32BitCount = 16;
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
 
	JDx12SceneDependencyData::DepthReleativeDataSet::DepthReleativeDataSet(const JDx12GraphicSceneDependencyDataComputeSet* set, const JDrawHelper& helper)
	{
		device = static_cast<JDx12GraphicDevice*>(set->device);
		gm = static_cast<JDx12GraphicResourceManager*>(set->gm);

		auto gInterface = helper.GetResourceInterface();
		auto cInterface = helper.GetCullInterface();

		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);
		shareData = static_cast<JDx12GraphicResourceShareData*>(set->shareData);
		drawSceneShareData = shareData->GetDrawSceneData(helper.cam->GetGuid(), helper.cam->GetRenderTargetSize());
		if (drawSceneShareData == nullptr || !drawSceneShareData->IsValid())
			return;
 
		dsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		preDsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA);

		viewZSet = context->ComputeSet(drawSceneShareData->viewZMap);
		preViewZSet = context->ComputeSet(drawSceneShareData->preViewZMap);
		depthDerivativeSet = context->ComputeSet(drawSceneShareData->depthDerivativeMap);  

		resolution = helper.cam->GetRenderTargetSize();
		camFrameIndex = helper.GetCamFrameIndex(J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA);

		drawSceneShareData->SetLife(); 
	}
	bool JDx12SceneDependencyData::DepthReleativeDataSet::IsValid()const noexcept
	{
		return drawSceneShareData != nullptr && drawSceneShareData->IsValid() && dsSet.IsValid() && preDsSet.IsValid();
	}

	JDx12SceneDependencyData::VelocityDataSet::VelocityDataSet(const JDx12GraphicSceneDependencyDataComputeSet* set, const JDrawHelper& helper)
	{
		device = static_cast<JDx12GraphicDevice*>(set->device);
		gm = static_cast<JDx12GraphicResourceManager*>(set->gm);

		auto gInterface = helper.GetResourceInterface();
		auto cInterface = helper.GetCullInterface();

		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);
		shareData = static_cast<JDx12GraphicResourceShareData*>(set->shareData);
		drawSceneShareData = shareData->GetDrawSceneData(helper.cam->GetGuid(), helper.cam->GetRenderTargetSize());
		if (drawSceneShareData == nullptr || !drawSceneShareData->IsValid())
			return;

		camFrameIndex = helper.GetCamFrameIndex(J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA);
		 
		rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		viewZSet = context->ComputeSet(drawSceneShareData->viewZMap);
		velocitySet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);
	
		camPreViewProj = DirectX::XMMatrixTranspose(helper.cam->GetPreViewProj().LoadXM());

		drawSceneShareData->SetLife(); 
	}
	bool JDx12SceneDependencyData::VelocityDataSet::IsValid()const noexcept
	{
		return drawSceneShareData != nullptr && drawSceneShareData->IsValid() && rtSet.IsValid() && velocitySet.IsValid();
	}

	JDx12SceneDependencyData::JDx12SceneDependencyData(PushGraphicEventPtr pushGraphicEvPtr)
		:pushGraphicEvPtr(pushGraphicEvPtr)
	{}
	JDx12SceneDependencyData::~JDx12SceneDependencyData()
	{ 
		ClearResource();
	}
	void JDx12SceneDependencyData::Initialize(JGraphicDevice* device, JGraphicResourceManager* gm)
	{ 
		if (!IsSameDevice(device) || !IsSameDevice(gm))
			return;

		BuildResource(device, gm);
	}
	void JDx12SceneDependencyData::Clear()
	{ 
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12SceneDependencyData::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}   
	void JDx12SceneDependencyData::ComputeDepthRelative(const JGraphicSceneDependencyDataComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet) || !helper.allowTemporalProcess)
			return;

		const JDx12GraphicSceneDependencyDataComputeSet* dx12Set = static_cast<const JDx12GraphicSceneDependencyDataComputeSet*>(computeSet);
		DepthReleativeDataSet set(dx12Set, helper);
		if (!set.IsValid())
		{
			if (set.drawSceneShareData == nullptr)
			{
				using CreateDependencyF = Core::JMFunctorType<JDx12SceneDependencyData, void, JGraphicDevice*, JGraphicResourceManager*, JDx12GraphicResourceShareData*, JWeakPtr<JCamera>>;
				pushGraphicEvPtr(Core::UniqueBind(std::make_unique<CreateDependencyF::Functor>(&JDx12SceneDependencyData::RequestShareDataCreation, this),
					std::move(set.device),
					std::move(set.gm),
					std::move(set.shareData),
					JWeakPtr<JCamera>(helper.cam)));
			}
			return;
		}

		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12Set->context);
		context->Transition(set.dsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(set.preDsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->Transition(set.preViewZSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->Transition(set.depthDerivativeSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->FlushResourceBarriers();

		context->SetComputeRootSignature(depthRootsignature.Get());
		context->SetComputeRootConstantBufferView(Depth::camCBIndex, J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA, set.camFrameIndex);
		context->SetComputeRootDescriptorTable(Depth::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Depth::preDepthMapIndex, set.preDsSet.GetGpuSrvHandle());

		context->SetComputeRootDescriptorTable(Depth::viewZMapIndex, set.viewZSet.GetGpuUavHandle());
		context->SetComputeRootDescriptorTable(Depth::preViewZMapIndex, set.preViewZSet.GetGpuUavHandle());
		context->SetComputeRootDescriptorTable(Depth::depthDerivativeMapIndex, set.depthDerivativeSet.GetGpuUavHandle());

		context->SetPipelineState(depthShader.get());
		context->Dispatch2D(set.resolution, depthShader->dispatchInfo.threadDim.XY());
	}
	void JDx12SceneDependencyData::ComputeVelocity(const JGraphicSceneDependencyDataComputeSet* computeSet, const JDrawHelper& helper)
	{ 
		if (!IsSameDevice(computeSet) || !helper.allowTemporalProcess)
			return;

		const JDx12GraphicSceneDependencyDataComputeSet* dx12Set = static_cast<const JDx12GraphicSceneDependencyDataComputeSet*>(computeSet);
		VelocityDataSet set(dx12Set, helper);
		if (!set.IsValid())
		{
			if (set.drawSceneShareData == nullptr)
			{
				using CreateDependencyF = Core::JMFunctorType<JDx12SceneDependencyData, void, JGraphicDevice*, JGraphicResourceManager*, JDx12GraphicResourceShareData*, JWeakPtr<JCamera>>;
				pushGraphicEvPtr(Core::UniqueBind(std::make_unique<CreateDependencyF::Functor>(&JDx12SceneDependencyData::RequestShareDataCreation, this),
					std::move(set.device),
					std::move(set.gm),
					std::move(set.shareData),
					JWeakPtr<JCamera>(helper.cam)));
			}
			return;
		}
		 
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12Set->context);
		context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(set.velocitySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->FlushResourceBarriers();

		context->SetComputeRootSignature(velocityRootsignature.Get());
		context->SetPipelineState(velocityShader.get());

		context->SetComputeRootConstantBufferView(Velocity::camCBIndex, J_FRAME_RESOURCE_UPLOAD_TYPE::CAMERA, set.camFrameIndex);
		context->SetComputeRoot32BitConstants(Velocity::passCBIndex, 0, set.camPreViewProj);
		context->SetComputeRootDescriptorTable(Velocity::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Velocity::velocityMapIndex, set.velocitySet.GetGpuUavHandle());
		 
		context->Dispatch2D(set.rtSet.info->GetResourceSize(), velocityShader->dispatchInfo.threadDim.XY());
	}
	void JDx12SceneDependencyData::RequestShareDataCreation(JGraphicDevice* device, JGraphicResourceManager* gm, JDx12GraphicResourceShareData* shareData, JWeakPtr<JCamera> cam)
	{ 
		if (!IsAllSameDevice(device, gm, shareData) || cam == nullptr)
			return;
		 
		shareData->RequestShareDataCreation(device, gm, J_GRAPHIC_REQUEST_TYPE::DRAW_SCENE, cam->GetGuid(), cam->GetRenderTargetSize());
	}
	void JDx12SceneDependencyData::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSignature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());;
	}
	void JDx12SceneDependencyData::BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		depthRootsignature = nullptr;
		velocityRootsignature = nullptr;

		JDx12RootSignatureBuilder<Depth::rootSlotCount> dBuilder;
		dBuilder.PushConstantsBuffer(Depth::camCBIndex);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		dBuilder.Create(device, L"DepthRelative RootSignature", depthRootsignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignatureBuilder<Velocity::rootSlotCount> vBuilder;
		vBuilder.PushConstantsBuffer(Velocity::camCBIndex);
		vBuilder.PushConstants(Velocity::cb32BitCount, Velocity::passCBIndex);
		vBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		vBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		vBuilder.Create(device, L"Velocity RootSignature", velocityRootsignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12SceneDependencyData::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		depthShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		velocityShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = 2;
		JDx12ComputePsoBulder<shaderCount> psoBuilder("JDx12SceneDependencyData");

		psoBuilder.PushHolder(depthShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::SceneRasterize(L"DepthRelative.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Depth::GetThreadDim());
		psoBuilder.PushRootSignature(depthRootsignature.Get());
		psoBuilder.Create(device);

		psoBuilder.PushHolder(velocityShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::SceneRasterize(L"VelocityBuffer.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Velocity::GetThreadDim());
		psoBuilder.PushRootSignature(velocityRootsignature.Get());
		psoBuilder.Create(device);
	}
	void JDx12SceneDependencyData::ClearResource()
	{
		velocityShader = nullptr;
		depthShader = nullptr;

		velocityRootsignature = nullptr;
		depthRootsignature = nullptr;
	}
}