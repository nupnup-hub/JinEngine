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

//#define DOWN_SAMPLE4 L"DOWN_SAMPLE4"
//#define TEXTURE_2D_COUNT L"TEXTURE_2D_COUNT"
#define VERTICAL_BLUR L"VERTICAL"
#define HORIZONTAL_BLUR L"HORIZONTAL" 
#define SKIP_SKY_COLOR L"SKIP_SKY_COLOR"

namespace JinEngine::Graphic
{ 
	namespace Raymarching
	{
		ROOT_INDEX_CREATOR(, ssrCBIndex, srcMapIndex, skyMapIndex, viewZMapIndex, normalMapIndex, lightPropMapIndex, destIndex)
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Raycasting
	{
		ROOT_INDEX_CREATOR(, ssrCBIndex, srcMapIndex, skyMapIndex, viewZMapIndex, normalMapIndex, lightPropMapIndex, accStructureIndex, destIndex)
		//ROOT_INDEX_CREATOR(, ssrCBIndex, accStructureIndex, instanceInfoIndex, materialDataIndex, srcMapIndex, viewZMapIndex, normalMapIndex, skyMapIndex, texture2DIndex, destIndex)
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	} 
	namespace Blur
	{
		ROOT_INDEX_CREATOR(, blurCBIndex, srcMapIndex, destIndex)

		static constexpr uint cb32BitCount = 5;
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace TemporalFilter
	{ 
		ROOT_INDEX_CREATOR(, ssrCBIndex, srcMapIndex, viewZMapIndex, preViewZMapIndex, normalMapIndex, preNormalMapIndex, lightPropMapIndex, preLightPropMapIndex, preHistoryIndex, curHistoryIndex)
		
		static constexpr uint haltonMax = 16; 
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Upsample
	{
		ROOT_INDEX_CREATOR(, passCBIndex, srcMapIndex, destIndex)
		static constexpr uint cb32BitCount = 8;
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Atrous
	{
		ROOT_INDEX_CREATOR(, passCBIndex, srcMapIndex, viewZMapIndex, lightPropMapIndex, destIndex)
		static constexpr uint cb32BitCount = 6;
		static constexpr uint defaultStepCount = 4;
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Apply
	{
		ROOT_INDEX_CREATOR(, ssrCBIndex, srcMapIndex, ssrMapIndex, albedoMapIndex, lightPropMapIndex, destIndex)
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Clear
	{
		ROOT_INDEX_CREATOR(, passCBIndex, curHistoryIndex, preHistoryIndex)
		static constexpr uint cb32BitCount = 2;
		static JVector3<uint> ThreadDim()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	namespace Common
	{
		static constexpr uint sampleMax = 64;
		static constexpr uint sampleSetSize = 8;

		template<typename T>
		static JVector4F CreateDimDataPack(const JVector2<T> rtSize)
		{
			return JVector4F(JVector2F(rtSize), rtSize.Inverse());
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
		//if (!HasWaitFrame() && !cam->ModuleManagedData()->GetFrameUpdateUserInterface()->IsLastUpdated())
		//	return;

		const JVector2F camRtSize = cam->GetRenderTargetSize(); 
		const JVector2F camHalfRtSize = camRtSize * 0.5f;
		//const JVector2<uint> quaterRtSize = camRtSize / 4.0f;
		const JUserPtr<JScene>& scene = helper.scene;

		static SsrPassConstants constants;
		 
		constants.ta.camInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetInvView()));
		constants.ta.camPreInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreInvView()));
		constants.ta.camPreViewProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreViewProj().LoadXM()));
		constants.ta.rtSize = camRtSize;
		constants.ta.invRtSize = camRtSize.Inverse();
		cam->GetUvToView(constants.ta.uvToViewA, constants.ta.uvToViewB);
		cam->GetPreUvToView(constants.ta.preUvToViewA, constants.ta.preUvToViewB);

		constants.ta.camNearFar = JVector2F(cam->GetNear(), cam->GetFar());
		constants.ta.camNearMulFar = constants.ta.camNearFar.x * constants.ta.camNearFar.y;
		
		//not use jitter
		//if (constants.ta.sampleNumber >= TemporalFilter::haltonMax)
		//	constants.ta.sampleNumber = 0;

		constants.camView.StoreXM(DirectX::XMMatrixTranspose(cam->GetView().LoadXM()));
		constants.camProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetProj().LoadXM()));

		constants.camPosW = cam->GetOwner()->GetTransform()->GetWorldPosition();
		constants.rayTMin = desc.rayqueryTmin;

		constants.halfRtSize = camHalfRtSize;
		constants.halfInvRtSize = camHalfRtSize.Inverse();
 
		constants.stepScale = desc.stepScale;
		constants.startOffset = desc.startOffset;
		constants.maxStepCount = desc.maxStepCount;
		constants.rayDistance = desc.rayDistance;

		constants.thickness = desc.thickness;
		constants.objectViewZBias = desc.objectViewZBias;
		constants.fadeDistance = desc.fadeDistance * 2.0f;
		constants.fadeOneRate = constants.fadeDistance == 0 ? 0 :  1.0f / constants.fadeDistance;
		
		if (constants.sampleNumber > Common::sampleMax)
			constants.sampleNumber = 0;
		constants.sampleSetSize = Common::sampleSetSize;
		constants.sampleMax = Common::sampleMax;
 
		frameBuffer.CopyData(helper.info.frame.currIndex, constants); 
		constants.sampleNumber += 4;
	}
	void JDx12Ssr::UserPrivateData::End(const JDrawHelper& helper)
	{
		AddUpdateCount();
		++historyIndex;
		if (historyIndex >= historyCount)
			historyIndex = 0;
		preHistoryIndex = (historyCount - 1) - historyIndex;
	}

	JDx12Ssr::SsrComputeSet::SsrComputeSet(JDx12PostProcessComputeSet* set, const JDrawHelper& helper)
	{
		imageShare = static_cast<ImageProcessingShareData*>(set->imageShareData);
		drawSceneShare = static_cast<DrawSceneShareData*>(set->drawSceneShareData);
		if (!drawSceneShare->IsValid())
			return;

		context = static_cast<JDx12CommandContext*>(set->context);
		device = static_cast<JDx12GraphicDevice*>(set->device);
		gm = static_cast<JDx12GraphicResourceManager*>(set->gm);
		ssrDesc = helper.cam->GetSsrDesc();

		auto gInterface = helper.GetResourceInterface();
		auto aInterface = helper.GetGpuAcceleratorInterface();

		rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		auto preRsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA);

		viewZSet = context->ComputeSet(drawSceneShare->viewZMap); 
		preViewZSet = context->ComputeSet(drawSceneShare->preViewZMap);

		albedoSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::ALBEDO_MAP);

		normalSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		preNormalSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);

		lightPropSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
		preLightPropSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);

		accelSet = context->ComputeSet(aInterface);

		ssrSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SSR_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSR); 

		if (imageShare->HasUpdated())
			srcSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		else
			srcSet = rtSet;
		 
		for (uint i = 0; i < Constants::ssrMipCount; ++i)
		{
			ssrMipPing[i] = context->ComputeSet(imageShare->ssrMipPing[i]);
			ssrMipPong[i] = context->ComputeSet(imageShare->ssrMipPong[i]);
		}

		ssrIntermediateSet = context->ComputeSet(imageShare->ssrIntermediate);
		ssrHalfIntermediateSet = &ssrMipPing[0];
		 
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
		halfResolution = resolution * 0.5f; 
		camFar = helper.cam->GetFar();

		dimPack = Common::CreateDimDataPack(resolution); 
		halfDimPack = Common::CreateDimDataPack(halfResolution);
	 
		//resolution = resolution * 0.5f;
		isValid = true; 
	}
	void JDx12Ssr::SsrComputeSet::SetUserPrivate(UserPrivateData* data, const JSsrDesc& desc, const JDrawHelper& helper)
	{
		userPrivate = data;
		if (userPrivate->colorHistory[0] == nullptr)
			requestCreateDependencyData = true;
		else
		{
			userPrivate->Begin(desc, helper);
			ssrCurHistorySet = context->ComputeSet(userPrivate->colorHistory[userPrivate->historyIndex]);
			ssrPreHistorySet = context->ComputeSet(userPrivate->colorHistory[userPrivate->preHistoryIndex]);
		}
	}
	bool JDx12Ssr::SsrComputeSet::IsValid()const noexcept
	{
		return rtSet.IsValid() && drawSceneShare != nullptr && drawSceneShare->IsValid();
	}

	JDx12Ssr::JDx12Ssr(PushGraphicEventPtr pushGraphicEvPtr)
		:guid(Core::MakeGuid()), pushGraphicEvPtr(pushGraphicEvPtr)
	{}
	JDx12Ssr::~JDx12Ssr()
	{
		ClearResource();
	}
	void JDx12Ssr::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device);
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
		if (set.requestCreateDependencyData)
		{
			using CreateDependencyF = Core::JMFunctorType<JDx12Ssr, void, JGraphicDevice*, JGraphicResourceManager*, UserPrivateData*, JVector2<uint>>;
			pushGraphicEvPtr(Core::UniqueBind(std::make_unique<CreateDependencyF::Functor>(&JDx12Ssr::CreateDependencyData, this),
				std::move(set.device),
				std::move(set.gm),
				std::move(set.userPrivate),
				JVector2<uint>(set.resolution)));
			return;
		}
		 
		if (set.userPrivate->HasWaitFrame())
			set.userPrivate->MinusWaitFrame();
		else
		{ 
			if (set.userPrivate->HasClearRequest())
				Clear(set, helper);

			BlurSourceMap(set, helper);

			if (set.accelSet.IsValid() && helper.option.debugging.testTrigger00)
				ComputeWithRaycasting(set, helper);
			else
				ComputeWithRaymarching(set, helper);

			Upsample(set, helper);
			TemporalFilter(set, helper);
			Atrous(set, helper, Atrous::defaultStepCount);
			Apply(set, helper);
		}
		End(set, helper);
	}
	void JDx12Ssr::BlurSourceMap(SsrComputeSet& set, const JDrawHelper& helper)
	{ 
		set.context->Transition(set.srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMipPing, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, false, std::make_index_sequence<Constants::ssrMipCount>());
		set.context->Transition(set.ssrMipPong, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, false, std::make_index_sequence<Constants::ssrMipCount>());
		set.context->FlushResourceBarriers();

		//ROOT_INDEX_CREATOR(, blurCBIndex, srcMapIndex, destIndex)
		set.context->SetComputeRootSignature(blurRootSignature.Get());
		 
		JVector2<uint> destResolution = set.halfResolution;
		set.context->SetComputeRoot32BitConstants(Blur::blurCBIndex, 4, set.ssrDesc.blurScale);
		 
		for (uint i = 0; i < Constants::ssrMipCount; ++i)
		{  
			const JVector4F dataPack(JVector2F(destResolution), destResolution.Inverse());
			set.context->SetComputeRoot32BitConstants(Blur::blurCBIndex, 0, dataPack);

			if (i == 0)
				set.context->SetComputeRootDescriptorTable(Blur::srcMapIndex, set.srcSet.GetGpuSrvHandle());
			else
				set.context->SetComputeRootDescriptorTable(Blur::srcMapIndex, set.ssrMipPong[i - 1].GetGpuSrvHandle());

			set.context->SetComputeRootDescriptorTable(Blur::destIndex, set.ssrMipPing[i].GetGpuUavHandle());
			set.context->SetPipelineState(blurShader[SSR_BLUR_VERTICAL].get());
			set.context->Dispatch2D(destResolution, blurShader[SSR_BLUR_VERTICAL]->dispatchInfo.threadDim.XY());

			set.context->SetComputeRootDescriptorTable(Blur::srcMapIndex, set.ssrMipPing[i].GetGpuSrvHandle());
			set.context->SetComputeRootDescriptorTable(Blur::destIndex, set.ssrMipPong[i].GetGpuUavHandle());

			set.context->SetPipelineState(blurShader[SSR_BLUR_HORIZONTAL].get());
			set.context->Dispatch2D(destResolution, blurShader[SSR_BLUR_HORIZONTAL]->dispatchInfo.threadDim.XY());

			destResolution *= 0.5f;
		}  
	}
	void JDx12Ssr::ComputeWithRaymarching(SsrComputeSet& set, const JDrawHelper& helper)
	{  
		static constexpr uint rayTypeIndex = (uint)J_SSR_TYPE::RAYMARCHING;
		const bool hasSky = set.skyMapSrvHeapIndex != invalidIndex;
		const SSR_RAY_COMPUTE_SHADER option = hasSky ? SSR_RAY_COMPUTE_SHADER_NORMAL : SSR_RAY_COMPUTE_SHADER_SKIP_SKY_COLOR;

		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.lightPropSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMipPong, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false, std::make_index_sequence<Constants::ssrMipCount>());
		set.context->Transition(set.ssrHalfIntermediateSet->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(computeRootSignature[rayTypeIndex].Get());
		set.context->SetComputeRootConstantBufferView(Raymarching::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);
		set.context->SetComputeRootDescriptorTable(Raymarching::srcMapIndex, set.ssrMipPong[0].GetGpuSrvHandle());
		if (hasSky)
			set.context->SetComputeRootDescriptorTable(Raymarching::skyMapIndex, set.skyMapSrvHeapIndex);

		set.context->SetComputeRootDescriptorTable(Raymarching::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raymarching::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raymarching::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raymarching::destIndex, set.ssrHalfIntermediateSet->GetGpuUavHandle());

		set.context->SetPipelineState(computeShader[rayTypeIndex][option].get());
		set.context->Dispatch2D(set.halfResolution, computeShader[rayTypeIndex][option]->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::ComputeWithRaycasting(SsrComputeSet& set, const JDrawHelper& helper)
	{
		static constexpr uint rayTypeIndex = (uint)J_SSR_TYPE::RAYCASTING;
		const bool hasSky = set.skyMapSrvHeapIndex != invalidIndex;
		const SSR_RAY_COMPUTE_SHADER option = hasSky ? SSR_RAY_COMPUTE_SHADER_NORMAL : SSR_RAY_COMPUTE_SHADER_SKIP_SKY_COLOR;

		set.context->Transition(set.srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.lightPropSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrMipPong, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false, std::make_index_sequence<Constants::ssrMipCount>());
		set.context->Transition(set.ssrHalfIntermediateSet->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(computeRootSignature[rayTypeIndex].Get());
		set.context->SetComputeRootConstantBufferView(Raycasting::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);
		 
		set.context->SetComputeRootDescriptorTable(Raycasting::srcMapIndex, set.ssrMipPong[0].GetGpuSrvHandle());
		if (hasSky)
			set.context->SetComputeRootDescriptorTable(Raycasting::skyMapIndex, set.skyMapSrvHeapIndex);

		set.context->SetComputeRootDescriptorTable(Raycasting::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raycasting::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Raycasting::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetTlasView(Raycasting::accStructureIndex, set.accelSet);

		set.context->SetComputeRootDescriptorTable(Raycasting::destIndex, set.ssrHalfIntermediateSet->GetGpuUavHandle());

		set.context->SetPipelineState(computeShader[rayTypeIndex][option].get());
		set.context->Dispatch2D(set.halfResolution, computeShader[rayTypeIndex][option]->dispatchInfo.threadDim.XY());
	}  
	void JDx12Ssr::Upsample(SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.ssrHalfIntermediateSet->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		set.context->Transition(set.ssrSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); 
		set.context->FlushResourceBarriers();
		  
		set.context->SetComputeRootSignature(upsampleRootSignature.Get());
		set.context->SetComputeRoot32BitConstants(Upsample::passCBIndex, 0, set.halfDimPack);
		set.context->SetComputeRoot32BitConstants(Upsample::passCBIndex, 4, set.dimPack);

		set.context->SetComputeRootDescriptorTable(Upsample::srcMapIndex, set.ssrHalfIntermediateSet->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Upsample::destIndex, set.ssrSet.GetGpuUavHandle());
		
		set.context->SetPipelineState(upsampleShader.get());
		set.context->Dispatch2D(set.resolution, upsampleShader->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::TemporalFilter(SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.ssrSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preViewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preNormalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preLightPropSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrPreHistorySet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrCurHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();
 
		set.context->SetComputeRootSignature(temporalFilterRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(TemporalFilter::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);

		set.context->SetComputeRootDescriptorTable(TemporalFilter::srcMapIndex, set.ssrSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TemporalFilter::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TemporalFilter::preViewZMapIndex, set.preViewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TemporalFilter::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TemporalFilter::preNormalMapIndex, set.preNormalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TemporalFilter::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TemporalFilter::preLightPropMapIndex, set.preLightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TemporalFilter::preHistoryIndex, set.ssrPreHistorySet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TemporalFilter::curHistoryIndex, set.ssrCurHistorySet.GetGpuUavHandle());

		set.context->SetPipelineState(temporalFilterShader.get());
		set.context->Dispatch2D(set.resolution, temporalFilterShader->dispatchInfo.threadDim.XY()); 
	}
	void JDx12Ssr::Atrous(SsrComputeSet& set, const JDrawHelper& helper, const uint stepCount)
	{
		set.context->Transition(set.ssrCurHistorySet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.ssrIntermediateSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();
		  
		set.context->SetComputeRootSignature(atorusRootSignature.Get());
		set.context->SetComputeRoot32BitConstants(Atrous::passCBIndex, 0, set.dimPack);
		set.context->SetComputeRoot32BitConstants(Atrous::passCBIndex, 4, set.camFar);
		set.context->SetComputeRootDescriptorTable(Atrous::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Atrous::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());

		set.context->SetPipelineState(atorusShader.get());

		JDx12GraphicResourceComputeSet* srcSet = &set.ssrCurHistorySet;
		JDx12GraphicResourceComputeSet* destSet = &set.ssrIntermediateSet;

		for (uint i = 0; i < stepCount; ++i)
		{ 
			uint stepSize = 1 << i; 
			set.context->SetComputeRoot32BitConstants(Atrous::passCBIndex, 5, stepSize);
			set.context->SetComputeRootDescriptorTable(Atrous::srcMapIndex, srcSet->GetGpuSrvHandle());
			set.context->SetComputeRootDescriptorTable(Atrous::destIndex, destSet->GetGpuUavHandle());

			set.context->Dispatch2D(set.resolution, atorusShader->dispatchInfo.threadDim.XY());

			JDx12GraphicResourceComputeSet* temp = srcSet;
			srcSet = destSet;
			destSet = temp; 
		} 
		set.context->CopyResource(srcSet->holder, set.ssrSet.holder);
	}
	void JDx12Ssr::Apply(SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.albedoSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		set.context->Transition(set.srcSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.destSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(applyRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Apply::ssrCBIndex, &set.userPrivate->frameBuffer, helper.info.frame.currIndex);
		set.context->SetComputeRootDescriptorTable(Apply::srcMapIndex, set.srcSet.GetGpuSrvHandle()); 
		set.context->SetComputeRootDescriptorTable(Apply::ssrMapIndex, set.ssrCurHistorySet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Apply::albedoMapIndex, set.albedoSet.GetGpuSrvHandle()); 
		set.context->SetComputeRootDescriptorTable(Apply::lightPropMapIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Apply::destIndex, set.destSet.GetGpuUavHandle());

		set.context->SetPipelineState(applyShader.get());
		set.context->Dispatch2D(set.resolution, applyShader->dispatchInfo.threadDim.XY());
	}
	void JDx12Ssr::Clear(SsrComputeSet& set, const JDrawHelper& helper)
	{ 
		set.context->Transition(set.ssrPreHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.ssrCurHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(clearRootSignature.Get());
		set.context->SetComputeRoot32BitConstants(Clear::passCBIndex, 0, set.resolution);
		set.context->SetComputeRootDescriptorTable(Clear::curHistoryIndex, set.ssrCurHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::preHistoryIndex, set.ssrPreHistorySet.GetGpuUavHandle());

		set.context->SetPipelineState(clearShader.get());
		set.context->Dispatch2D(set.resolution, clearShader->dispatchInfo.threadDim.XY());
		set.userPrivate->OffClearTrigger();
	}
	void JDx12Ssr::Begin(SsrComputeSet& set, const JDrawHelper& helper)
	{
		auto data = userPrivate.find(helper.cam->GetGuid());
		if (data == userPrivate.end())
			data = userPrivate.emplace(helper.cam->GetGuid(), std::make_unique<UserPrivateData>(set.device)).first;

		set.SetUserPrivate(data->second.get(), set.ssrDesc, helper);
	}
	void JDx12Ssr::End(const SsrComputeSet& set, const JDrawHelper& helper)
	{
		set.userPrivate->End(helper);
	}
	void JDx12Ssr::RecompileShader(const JGraphicShaderCompileSet& dataSet)
	{
		//SetOption.이 BuildResource 보다 먼저 호출되는지 검사하도록
		ClearResource();
		BuildResource(dataSet.device);
	}
	void JDx12Ssr::CreateDependencyData(JGraphicDevice* device, JGraphicResourceManager* gm, UserPrivateData* userPrivate, JVector2<uint> rtSize)
	{
		JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::SSR_MAP, J_GRAPHIC_TASK_TYPE::UNKNOWN);
		JGraphicResourceCreationDesc desc(typeSet);
		 
		desc.width = rtSize.x;
		desc.height = rtSize.y;
		desc.formatHint = nullptr;
		desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::SSR_MAP;
		 
		for (uint i = 0; i < userPrivate->historyCount; ++i)
			userPrivate->colorHistory[i] = gm->CreateResource(device, desc);

		userPrivate->device = device;
		userPrivate->gm = gm;
	}
	void JDx12Ssr::BuildResource(JGraphicDevice* device)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device); 
		ID3D12Device* d3d12Device = dx12Device->GetDevice();

		BuildRootSignature(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		BuildPso(d3d12Device, GetGraphicInfo(), GetGraphicOption());
		 
		for (auto& data : userPrivate)
			data.second->SetClearTrigger();
	}
	void JDx12Ssr::BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{  
		JDx12RootSignatureBuilder2<Raymarching::rootSlotCount, 2> raymarchingBuilder;
		raymarchingBuilder.PushConstantsBuffer(Raymarching::ssrCBIndex);
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Constants::ssrMipCount, 0, 1);		//srcMap
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//skyMapIndex
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//viewZMapIndex
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//normalMapIndex
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);		//lightPropMapIndex
		raymarchingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		raymarchingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raymarchingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raymarchingBuilder.Create(device, L"Ssr raymarching rootsignature", computeRootSignature[(uint)J_SSR_TYPE::RAYMARCHING].GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignatureBuilder2<Raycasting::rootSlotCount, 2> raycastingBuilder;
		raycastingBuilder.PushConstantsBuffer(Raymarching::ssrCBIndex);
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Constants::ssrMipCount, 0, 1);		//srcMap
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//skyMapIndex
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//viewZMapIndex
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//normalMapIndex
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);		//lightPropMapIndex
		raycastingBuilder.PushShaderResource(5);								//bvh
		raycastingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap	 
		raycastingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raycastingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		raycastingBuilder.Create(device, L"Ssr raycasting rootsignature", computeRootSignature[(uint)J_SSR_TYPE::RAYCASTING].GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
		
		JDx12RootSignatureBuilder2<Blur::rootSlotCount, 1> blurBuilder;
		blurBuilder.PushConstants(Blur::cb32BitCount, Blur::blurCBIndex);
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap 
		blurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		blurBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		blurBuilder.Create(device, L"Ssr blur rootsignature", blurRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
		  
		JDx12RootSignatureBuilder2<Upsample::rootSlotCount, 1> upSampleBuilder;
		upSampleBuilder.PushConstants(Upsample::cb32BitCount, Upsample::passCBIndex);
		upSampleBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap 
		upSampleBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		upSampleBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		upSampleBuilder.Create(device, L"Ssr upsample rootsignature", upsampleRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignatureBuilder2<TemporalFilter::rootSlotCount, 2> temporalFilterBuilder;
		temporalFilterBuilder.PushConstantsBuffer(TemporalFilter::ssrCBIndex);
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//viewZMap
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//preViewZMap
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//normalMap
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);		//preNormalMap
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);		//lightPropMap
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);		//preLightProp
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);		//preHistory
		temporalFilterBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//curHistory
		temporalFilterBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		temporalFilterBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		temporalFilterBuilder.Create(device, L"Ssr temporalFilter rootsignature", temporalFilterRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
 
		JDx12RootSignatureBuilder2<Atrous::rootSlotCount, 1> atrousBuilder;
		atrousBuilder.PushConstants(Atrous::cb32BitCount, Atrous::passCBIndex);
		atrousBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap 
		atrousBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//viewZMap 
		atrousBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//lightProp 
		atrousBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		atrousBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		atrousBuilder.Create(device, L"Ssr atrous rootsignature", atorusRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		JDx12RootSignatureBuilder2<Apply::rootSlotCount, 1> finalColorBuilder;
		finalColorBuilder.PushConstantsBuffer(Apply::ssrCBIndex);
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//srcMap
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//ssrMapIndex
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);		//albedoMapIndex 
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);		//lightPropMapIndex 
		finalColorBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//destMap
		finalColorBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		finalColorBuilder.Create(device, L"Ssr final color rootsignature", applyRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);	 

		JDx12RootSignatureBuilder<Clear::rootSlotCount> clearBuilder;
		clearBuilder.PushConstants(Clear::cb32BitCount, Clear::passCBIndex);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);		//curHistory
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);		//preHistory  
		clearBuilder.Create(device, L"Ssr clear rootsignature", clearRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
	}
	void JDx12Ssr::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{ 
		for (uint i = 0; i < (uint)J_SSR_TYPE::COUNT; ++i)
		{
			for(uint j = 0; j < SSR_RAY_COMPUTE_SHADER_COUNT; ++j)
				computeShader[i][j] = std::make_unique<JDx12ComputeShaderDataHolder>();
		}
		for(uint i = 0; i < SIZE_OF_ARRAY(blurShader); ++i)
			blurShader[i] = std::make_unique<JDx12ComputeShaderDataHolder>(); 

		upsampleShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		temporalFilterShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		atorusShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		applyShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		clearShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = ((uint)J_SSR_TYPE::COUNT * SSR_RAY_COMPUTE_SHADER_COUNT) + SIZE_OF_ARRAY(blurShader) + 5;
		JDx12ComputePsoBulder<shaderCount> psoBuilder("JDx12Ssr");
		 
		std::wstring shaderName[(uint)J_SSR_TYPE::COUNT]
		{
			L"ComputeWithRaymarching.hlsl",
			L"ComputeWithRaycasting.hlsl"
		};
		for (uint i = 0; i < (uint)J_SSR_TYPE::COUNT; ++i)
		{ 
			for (uint j = 0; j < SSR_RAY_COMPUTE_SHADER_COUNT; ++j)
			{
				psoBuilder.PushHolder(computeShader[i][j].get());
				psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(shaderName[i]), L"main"));
				psoBuilder.PushThreadDim(Raymarching::ThreadDim());
				psoBuilder.PushRootSignature(computeRootSignature[i].Get());
				if (j == SSR_RAY_COMPUTE_SHADER_SKIP_SKY_COLOR)
					psoBuilder.PushMacroSet(JMacroSet{ SKIP_SKY_COLOR,  std::to_wstring(1) });

				psoBuilder.Next();
			}
		} 

		for (uint i = 0; i < SIZE_OF_ARRAY(blurShader); ++i)
		{
			psoBuilder.PushHolder(blurShader[i].get());
			psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"Blur.hlsl"), L"main"));
			psoBuilder.PushThreadDim(Blur::ThreadDim());
			if (i == SSR_BLUR_HORIZONTAL)
				psoBuilder.PushMacroSet(JMacroSet{ HORIZONTAL_BLUR,  std::to_wstring(1) });
			else
				psoBuilder.PushMacroSet(JMacroSet{ VERTICAL_BLUR,  std::to_wstring(1)}); 
			psoBuilder.PushRootSignature(blurRootSignature.Get());
			psoBuilder.Next();
		}

		psoBuilder.PushHolder(upsampleShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Image(L"SimpleUpsample.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Upsample::ThreadDim());
		psoBuilder.PushRootSignature(upsampleRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(temporalFilterShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"TemporalFilter.hlsl"), L"main"));
		psoBuilder.PushThreadDim(TemporalFilter::ThreadDim());
		psoBuilder.PushRootSignature(temporalFilterRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(atorusShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"Atrous.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Atrous::ThreadDim());
		psoBuilder.PushRootSignature(atorusRootSignature.Get());
		psoBuilder.Next();
 
		psoBuilder.PushHolder(applyShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"Apply.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Apply::ThreadDim()); 
		psoBuilder.PushRootSignature(applyRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(clearShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::Ssr(L"Clear.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Clear::ThreadDim());
		psoBuilder.PushRootSignature(clearRootSignature.Get());
		psoBuilder.Next();
		psoBuilder.Create(device); 
	}
	void JDx12Ssr::ClearResource()
	{
		ClearRootSignature(); 
		ClearPso();
	}
	void JDx12Ssr::ClearRootSignature()
	{
		clearRootSignature = nullptr;
		applyRootSignature = nullptr;
		atorusRootSignature = nullptr;
		temporalFilterRootSignature = nullptr;
		upsampleRootSignature = nullptr;
		blurRootSignature = nullptr; 
		for (uint i = 0; i < SIZE_OF_ARRAY(computeRootSignature); ++i)
			computeRootSignature[i] = nullptr;
	}
	void JDx12Ssr::ClearPso()
	{ 
		clearShader = nullptr;
		applyShader = nullptr;
		atorusShader = nullptr;
		temporalFilterShader = nullptr;
		upsampleShader = nullptr;
		for (uint i = 0; i < SIZE_OF_ARRAY(blurShader); ++i)
			blurShader[i] = nullptr; 

		for (uint i = 0; i < (uint)J_SSR_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < SSR_RAY_COMPUTE_SHADER_COUNT; ++j)
				computeShader[i][j] = nullptr;
		} 
	}
}
