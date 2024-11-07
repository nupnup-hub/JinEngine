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


#include"JDx12Ssr.h"
#include"JDx12ImageConstants.h"
#include"../JImageProcessingFilter.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../DataSet/Dx/JDx12GraphicTaskDataSet.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"
#include"../../FrameResource/Dx/JCameraConstants.h" 
#include"../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../JGraphicUpdateHelper.h"
#include"../../../Application/Engine/JApplicationEngine.h"
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h" 
#include"../../../Object/Component/Camera/JCamera.h" 
#include"../../../Object/Component/Transform/JTransform.h" 
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Resource/Material/JMaterial.h"
#include"../../../Object/Resource/Texture/JTexture.h"   
#include"../../../Object/Resource/Scene/JScene.h"

#define DOWN_SAMPLE4 L"DOWN_SAMPLE4"

namespace JinEngine::Graphic
{ 
	namespace Raymarching
	{
		ROOT_INDEX_CREATOR(, ssrCBIndex, srcMapIndex, viewZMapIndex, normalMapIndex, destIndex)
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Raycasting
	{
		ROOT_INDEX_CREATOR(, ssrCBIndex, accStructureIndex, srcMapIndex, viewZMapIndex, normalMapIndex, skyMapIndex, destIndex)
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace DownSample
	{ 
		ROOT_INDEX_CREATOR(, constantsIndex, srcMapIndex, mipmap00Index, mipmap01Index, mipmap02Index, mipmap03Index)
		
		static constexpr uint cb32BitCount = 2;
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(8, 8, 1);
		}
	}
	namespace Blur
	{
		ROOT_INDEX_CREATOR(, ssrCBIndex, srcMapIndex, viewZMapIndex, normalMapIndex, lightPropMapIndex, depthDerivativeMapIndex, destIndex)
			static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Apply
	{
		ROOT_INDEX_CREATOR(, ssrCBIndex, srcMapIndex, viewZMapIndex, albedoMapIndex, lightPropMapIndex, ssrMapIndex, destIndex)
			static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	 
	JDx12Ssr::UserPrivateData::UserPrivateData(JGraphicDevice* device)
		:frameBuffer(JDx12GraphicBufferT<SsrPassConstants>(L"SsrPass", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT))
	{
		SetWaitFrame(Constants::gNumFrameResources);
		SetClearTrigger();
		frameBuffer.Build(device, Constants::gNumFrameResources);
	}
	JDx12Ssr::UserPrivateData::~UserPrivateData()
	{
		frameBuffer.Clear();
	}
	void JDx12Ssr::UserPrivateData::Begin(const JSsrDesc& desc, const JDrawHelper& helper)
	{
		const JUserPtr<JCamera>& cam = helper.cam;
		if (!HasWaitFrame() && !cam->ModuleManagedData()->GetFrameUpdateUserInterface()->IsLastUpdated())
			return;

		const JVector2F camRtSize = cam->GetRenderTargetSize(); 
		//const JVector2<uint> quaterRtSize = camRtSize / 4.0f;
		const JUserPtr<JScene>& scene = helper.scene;

		static SsrPassConstants constants;
		const DirectX::XMMATRIX view = cam->GetView().LoadXM();
		constants.camView.StoreXM(DirectX::XMMatrixTranspose(view));
		constants.camProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetProj().LoadXM()));
		constants.camInvView.StoreXM(DirectX::XMMatrixTranspose(XMMatrixInverse(nullptr, view)));
		  
		constants.rtSize = camRtSize;
		constants.invRtSize = 1.0f / constants.rtSize;

		//constants.halfRtSize = camRtSize * 0.5f;
		//constants.halfInvRtSize = 1.0f / constants.halfRtSize;

		cam->GetUvToView(constants.uvToViewA, constants.uvToViewB);

		constants.camNearFar = JVector2F(cam->GetNear(), cam->GetFar());
		constants.camNearFarMul = constants.camNearFar.x * constants.camNearFar.y;
		constants.startOffset = desc.startOffset;

		constants.camPosW = cam->GetOwner()->GetTransform()->GetWorldPosition();
		constants.stepScale = desc.stepScale;

		constants.maxStepCount = desc.maxStepCount;
		constants.rayDistance = desc.rayDistance;
		constants.thickness = desc.thickness;
		constants.objectViewZBias = desc.objectViewZBias;

		constants.fadeDistance = desc.fadeDistance * 2.0f;
		constants.fadeOneRate = 1.0f / constants.fadeDistance;

		frameBuffer.CopyData(helper.info.frame.currIndex, constants);
	}
	void JDx12Ssr::UserPrivateData::End(const JDrawHelper& helper)
	{
		AddUpdateCount();
	}

	JDx12Ssr::SsrComputeSet::SsrComputeSet(JDx12PostProcessComputeSet* set, const JDrawHelper& helper)
	{
		imageShare = static_cast<ImageProcessingShareData*>(set->imageShareData);
		drawSceneShare = static_cast<DrawSceneShareData*>(set->drawSceneShareData);
		if (!drawSceneShare->IsValid())
			return;

		context = static_cast<JDx12CommandContext*>(set->context);
		device = static_cast<JDx12GraphicDevice*>(set->device);
		ssrDesc = helper.cam->GetSsrDesc();

		auto gInterface = helper.GetResourceInterface();
		auto aInterface = helper.GetGpuAcceleratorInterface();

		rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		viewZSet = context->ComputeSet(drawSceneShare->viewZMap);
		albedoSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP);
		normalSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		lightPropSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
		ssrSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SSR_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSR);
		depthDerivativeMap = context->ComputeSet(drawSceneShare->depthDerivativeMap);
		accelSet = context->ComputeSet(aInterface);
		 
		if (imageShare->HasUpdated())
			srcSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		else
			srcSet = rtSet;
		 
		for (uint i = 0; i < Constants::ssrMipCount; ++i)
			ssrMip[i] = context->ComputeSet(imageShare->ssrMip[i]);

		destSet = context->ComputeSet(imageShare->GetUpdateWaitIntermediate());
		imageShare->AddUpdatedIndexCount();

		auto skyObj = helper.scene->GetGameObjectVec(J_RENDER_LAYER::SKY, Core::J_MESHGEOMETRY_TYPE::STATIC);
		skyMapSrvHeapIndex = invalidIndex;
		if (skyObj.size() > 0)
		{
			auto albedoMap = skyObj[0]->GetRenderItem()->GetValidMaterial(0)->GetAlbedoMap();
			if (albedoMap != nullptr)
				skyMapSrvHeapIndex = albedoMap->ModuleManagedData()->GetGraphicResourceUserInterface()->GetFirstResourceHeapStart(J_GRAPHIC_BIND_TYPE::SRV);
		}

		resolution = rtSet.info->GetResourceSize();
		//resolution = resolution * 0.5f;
		isValid = true;
	}
	void JDx12Ssr::SsrComputeSet::SetUserPrivate(UserPrivateData* data, const JDrawHelper& helper)
	{
		userPrivate = data;
	}
	bool JDx12Ssr::SsrComputeSet::IsValid()const noexcept
	{
		return rtSet.IsValid() && drawSceneShare != nullptr && drawSceneShare->IsValid();
	}

	JDx12Ssr::JDx12Ssr()
		:guid(Core::MakeGuid())
	{}
	JDx12Ssr::~JDx12Ssr()
	{
		ClearResource();
	}
	void JDx12Ssr::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResouce(device, gM);
	}
	void JDx12Ssr::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Ssr::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12Ssr::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		return false;
	}
	bool JDx12Ssr::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		if (type == JGraphicOption::TYPE::POST_PROCESS || type == JGraphicOption::TYPE::DEBUGGING)
			return true;
		else
			return false;
	}
	bool JDx12Ssr::HasDrawSequencePostProcessing()const noexcept
	{
		return true;
	}
	void JDx12Ssr::DrawSequencePostProcessing()
	{
		GraphicVolatileStorageInterface::UpdateEnd(userPrivate);
	}
	void JDx12Ssr::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{}
	void JDx12Ssr::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.changedPart == JGraphicOption::TYPE::POST_PROCESS)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));

		if (set.changedPart == JGraphicOption::TYPE::DEBUGGING && set.newOption.debugging.requestRecompileSsrShader)
			RecompileShader(JGraphicShaderCompileSet(dx12Set.device));
	}
	void JDx12Ssr::ApplySsr(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet) || helper.cam == nullptr || !helper.allowSsao)
			return;

		JDx12PostProcessComputeSet* ssrSet = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		SsrComputeSet set(ssrSet, helper);
		if (!set.isValid)
			return;

		Begin(set, helper);
		if (set.userPrivate->HasWaitFrame())
			set.userPrivate->MinusWaitFrame();
		else
		{
			if (set.accelSet.IsValid() && helper.option.debugging.testTrigger00)
				ComputeWithRaycasting(set, helper);
			else
				ComputeWithRaymarching(set, helper);

			DownSample(set, helper);
			Blur(set, helper);
			Apply(set, helper);
		}
		End(set, helper);
	}
	void JDx12Ssr::ComputeWithRaymarching(SsrComputeSet& set, const JDrawHelper& helper)
	{
		static constexpr uint typeIndex = (uint)J_SSR_TYPE::RAYMARCHING;

		set.context->Transition(set.srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(computeRootSignature[typeIndex].Get());
		set.context->SetComputeRootConstantBufferView(Raymarching::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);
		set.context->SetComputeRootDescriptorTable(Raymarching::srcMapIndex, set.srcSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raymarching::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raymarching::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raymarching::destIndex, set.ssrMip[0].GetGpuUavHandle());

		set.context->SetPipelineState(computeShader[typeIndex].get());
		set.context->Dispatch2D(set.resolution, computeShader[typeIndex]->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::ComputeWithRaycasting(SsrComputeSet& set, const JDrawHelper& helper)
	{
		static constexpr uint typeIndex = (uint)J_SSR_TYPE::RAYCASTING;

		set.context->Transition(set.srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[0].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(computeRootSignature[typeIndex].Get());
		set.context->SetComputeRootConstantBufferView(Raycasting::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);
		set.context->SetTlasView(Raycasting::accStructureIndex, set.accelSet);
		set.context->SetComputeRootDescriptorTable(Raycasting::srcMapIndex, set.srcSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raycasting::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raycasting::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		if(set.skyMapSrvHeapIndex != invalidIndex)
			set.context->SetComputeRootDescriptorTable(Raycasting::skyMapIndex, set.skyMapSrvHeapIndex);
		set.context->SetComputeRootDescriptorTable(Raycasting::destIndex, set.ssrMip[0].GetGpuUavHandle());

		set.context->SetPipelineState(computeShader[typeIndex].get());
		set.context->Dispatch2D(set.resolution, computeShader[typeIndex]->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::DownSample(SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.ssrMip[0].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[1].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.ssrMip[2].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.ssrMip[3].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.ssrMip[4].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		JVector2F rtSize = set.resolution;
		JVector2F invSize = 1.0f / rtSize;
		set.context->SetComputeRootSignature(downSampleRootsignature.Get());
		set.context->SetComputeRoot32BitConstants(DownSample::constantsIndex, 0, invSize);
		set.context->SetComputeRootDescriptorTable(DownSample::srcMapIndex, set.ssrMip[0].GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(DownSample::mipmap00Index, set.ssrMip[1].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(DownSample::mipmap01Index, set.ssrMip[2].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(DownSample::mipmap02Index, set.ssrMip[3].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(DownSample::mipmap03Index, set.ssrMip[4].GetGpuUavHandle());

		set.context->SetPipelineState(downSample4Shader.get());
		set.context->Dispatch2D(rtSize, downSample4Shader->dispatchInfo.threadDim.XY());

		set.context->Transition(set.ssrMip[4].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[5].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.ssrMip[6].holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); 
		set.context->FlushResourceBarriers();

		rtSize /= 16.0f; 
		invSize = 1.0f / rtSize;
		set.context->SetComputeRoot32BitConstants(DownSample::constantsIndex, 0, invSize);
		set.context->SetComputeRootDescriptorTable(DownSample::srcMapIndex, set.ssrMip[4].GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(DownSample::mipmap00Index, set.ssrMip[5].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(DownSample::mipmap01Index, set.ssrMip[6].GetGpuUavHandle()); 

		set.context->SetPipelineState(downSample2Shader.get());
		set.context->Dispatch2D(rtSize, downSample2Shader->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::Blur(SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.ssrMip[1].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[2].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[3].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[5].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMip[6].holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		set.context->Transition(set.ssrSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(blurRootsignature.Get());
		set.context->SetComputeRootConstantBufferView(Blur::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);
		set.context->SetComputeRootDescriptorTable(Blur::srcMapIndex, set.ssrMip[0].GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle()); 
		set.context->SetComputeRootDescriptorTable(Blur::depthDerivativeMapIndex, set.depthDerivativeMap.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::destIndex, set.ssrSet.GetGpuUavHandle());

		set.context->SetPipelineState(blurShader.get());
		set.context->Dispatch2D(set.resolution, blurShader->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::Apply(SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.albedoSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		set.context->Transition(set.ssrSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.destSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(applyRootsignature.Get());
		set.context->SetComputeRootConstantBufferView(Apply::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);
		set.context->SetComputeRootDescriptorTable(Apply::srcMapIndex, set.srcSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Apply::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Apply::albedoMapIndex, set.albedoSet.GetGpuSrvHandle());
		//set.context->SetComputeRootDescriptorTable(Apply::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Apply::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Apply::ssrMapIndex, set.ssrSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Apply::destIndex, set.destSet.GetGpuUavHandle());

		set.context->SetPipelineState(applyShader.get());
		set.context->Dispatch2D(set.resolution, applyShader->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::Begin(SsrComputeSet& set, const JDrawHelper& helper)
	{
		auto data = userPrivate.find(helper.cam->GetGuid());
		if (data == userPrivate.end())
			data = userPrivate.emplace(helper.cam->GetGuid(), std::make_unique<UserPrivateData>(set.device)).first;

		set.SetUserPrivate(data->second.get(), helper);
		set.userPrivate->Begin(set.ssrDesc, helper);
	}
	void JDx12Ssr::End(const SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.userPrivate->End(helper);
	}
	void JDx12Ssr::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		ClearRootSignature();
		ClearPso();

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(dataSet.device)->GetDevice();
		BuildRootSignature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12Ssr::BuildResouce(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSignature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());

		cachedDevice = dx12Device;
		cachedGm = dx12Gm;
	}
	void JDx12Ssr::BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		JDx12RootSignatureBuilder2<Raymarching::rootSlotCount, 2> raymarchingBuilder;
		raymarchingBuilder.PushConstantsBuffer(Raymarching::ssrCBIndex);
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//depthMap
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//normalMap
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		raymarchingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raymarchingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raymarchingBuilder.Create(device, L"Ssr raymarching rootsignature", computeRootSignature[(uint)J_SSR_TYPE::RAYMARCHING].GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignatureBuilder2<Raycasting::rootSlotCount, 2> raycastingBuilder;
		raycastingBuilder.PushConstantsBuffer(Raymarching::ssrCBIndex);
		raycastingBuilder.PushShaderResource(0);
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//srcMap
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//depthMap
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//normalMap
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);		//skyMap
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		raycastingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raycastingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raycastingBuilder.Create(device, L"Ssr raycasting rootsignature", computeRootSignature[(uint)J_SSR_TYPE::RAYCASTING].GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
 
		JDx12RootSignatureBuilder2<DownSample::rootSlotCount, 1> downSampleBuilder;
		downSampleBuilder.PushConstants(DownSample::cb32BitCount, DownSample::constantsIndex);
		downSampleBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap  
		downSampleBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//mip00
		downSampleBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);		//mip01
		downSampleBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);		//mip02
		downSampleBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);		//mip03
		downSampleBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		downSampleBuilder.Create(device, L"Ssr DownSample rootsignature", downSampleRootsignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignatureBuilder2<Blur::rootSlotCount, 1> blurBuilder;
		blurBuilder.PushConstantsBuffer(Blur::ssrCBIndex);
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Constants::ssrMipCount, 0, 1);		//mipCount
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//depthMap
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//normalMap
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//lightPropMap
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);		//depthDerivativeMap
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		blurBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		blurBuilder.Create(device, L"Ssr blur rootsignature", blurRootsignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignatureBuilder2<Apply::rootSlotCount, 1> finalColorBuilder;
		finalColorBuilder.PushConstantsBuffer(Apply::ssrCBIndex);
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//depthMap
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//albedoMap
		//finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//normalMap
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//lightPropMap
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);		//ssrMap
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		finalColorBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		finalColorBuilder.Create(device, L"Ssr final color rootsignature", applyRootsignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);	 
	}
	void JDx12Ssr::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		computeShader[(uint)J_SSR_TYPE::RAYMARCHING] = std::make_unique<JDx12ComputeShaderDataHolder>();
		computeShader[(uint)J_SSR_TYPE::RAYCASTING] = std::make_unique<JDx12ComputeShaderDataHolder>();
		downSample4Shader = std::make_unique<JDx12ComputeShaderDataHolder>();
		downSample2Shader = std::make_unique<JDx12ComputeShaderDataHolder>();
		applyShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		blurShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = 6; 
		JDx12ComputePsoBulder<shaderCount> psoBuilder("JDx12Ssr");

		psoBuilder.PushHolder(computeShader[(uint)J_SSR_TYPE::RAYMARCHING].get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"ComputeWithRaymarching.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Raymarching::ThreadDim());
		psoBuilder.PushRootSignature(computeRootSignature[(uint)J_SSR_TYPE::RAYMARCHING].Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(computeShader[(uint)J_SSR_TYPE::RAYCASTING].get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"ComputeWithRaycasting.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Raycasting::ThreadDim());
		psoBuilder.PushRootSignature(computeRootSignature[(uint)J_SSR_TYPE::RAYCASTING].Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(downSample4Shader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"DownSample.hlsl"), L"main"));
		psoBuilder.PushThreadDim(DownSample::ThreadDim());
		psoBuilder.PushMacroSet(JMacroSet{ DOWN_SAMPLE4, std::to_wstring(1)});
		psoBuilder.PushRootSignature(downSampleRootsignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(downSample2Shader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"DownSample.hlsl"), L"main"));
		psoBuilder.PushThreadDim(DownSample::ThreadDim());
		psoBuilder.PushRootSignature(downSampleRootsignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(blurShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"Blur.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Blur::ThreadDim());
		psoBuilder.PushRootSignature(blurRootsignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(applyShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"ApplySsrColor.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Apply::ThreadDim());
		psoBuilder.PushRootSignature(applyRootsignature.Get());
		psoBuilder.Next();
		psoBuilder.Create(device);
 
	}
	void JDx12Ssr::ClearResource()
	{
		ClearRootSignature();
		ClearPso();
		cachedDevice = nullptr;
		cachedGm = nullptr;
	}
	void JDx12Ssr::ClearRootSignature()
	{
		applyRootsignature = nullptr;
		blurRootsignature = nullptr;
		downSampleRootsignature = nullptr;
		for (uint i = 0; i < SIZE_OF_ARRAY(computeRootSignature); ++i)
			computeRootSignature[i] = nullptr;
	}
	void JDx12Ssr::ClearPso()
	{
		applyShader = nullptr;
		blurShader = nullptr;
		downSample4Shader = nullptr;
		downSample2Shader = nullptr;
		for (uint i = 0; i < SIZE_OF_ARRAY(computeShader); ++i)
			computeShader[i] = nullptr;
	}
}
