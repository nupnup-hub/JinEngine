#include"JDx12RaytracingDenoiser.h"
#include"../../Dx/JDx12RaytracingUtility.h"
#include"../../Dx/JDx12RaytracingConstants.h"
#include"../../../Device/Dx/JDx12GraphicDevice.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"   
#include"../../../GraphicResource/Dx/JDx12GraphicResourceShareData.h" 
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../Command/Dx/JDx12CommandContext.h"
#include"../../../Utility/Dx/JDx12ObjectCreation.h"  
#include"../../../FrameResource/Dx/JDx12FrameResource.h"  
#include"../../../JGraphicUpdateHelper.h" 
#include"../../../../Object/Component/Camera/JCamera.h"  
#include"../../../../Object/Component/RenderItem/JRenderItem.h"   
#include"../../../../Object/Resource/Scene/JScene.h"       
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Core/Log/JLogMacro.h"  
#include"../../../../Core/Math/JVectorExtend.h"  

namespace JinEngine::Graphic
{
	//restir gi는 svgf사용시 분산의 부정확한 추정때문에 아티팩트가 발생한다(Correlated input)
	//ref 2017-07_Spatiotemporal-Variance-Guided-Filtering
	/*
	* Stem 1. Temporal accumulation     - compute shader     do Demodulate albedo -> Temporal accumulation
	* Step 2. Variance estimation		- compute shader
	* Step 3. A-trous					- compute shader
	* Step 4. Modulate	Albedo			- compute shader
	*/

	//Reblur
	//ref raytracing gem2 ch 49 
	//ref EXPLORING RAYTRACED FUTURE IN METRO EXODU denoise part
	/*
	* Step 1.  Pre-blur							- compute shader	can skip
	* Step 2.  Temporal-Accumulation			- compute shader
	* Step 3.  Mip Generation and History Fix	- compute shader
	* Step 4.  Blur								- compute shader
	* Step 5.  Post-blur						- compute shader
	* Step 6.  Temporal Stabilization			- compute shader
	* Extra 1. Clear							- compute shader
	*/

	//우선은 Temporal-Accumulation, Blur로 구성된 Denoiser를 구현하고
	//점진적으로 기능을 추가해 Reblur와 유사한 결과를 내는 Deoniser를 완성하고자한다.
	//.. Reblur대신 Relax를 참조해 구현하기로 한다.
	//SVGF를 사용함에 있어 고스팅현상은 상당힌 눈에 거슬리는 아티팩트이므로 
	//이를 해결해야하는데 A-SVGF는 재투영을 통한 이전프레임 과에 gradient를 통해 history 축적값(Alpha)를 컨트롤해서 완화했으며
	//Reblur는 위에 Step3과 Step6을 통해서 완화했으나 Step3에서 구한 Mipmap layer에서 보간한 값이 만족스럽지않아서 Relax를 구현
	//bicubic과 Fast history를 통해서 제법 괜찮은 결과를 보여줬다.
	
	//Relax
	//ref nvidia pdf
	/*
	* Stem 1. Temporal accumulation     - compute shader     do Demodulate albedo -> Temporal accumulation
	* Step 2. history  clamping			- compute shader
	* Step 3. AntiFirefly 			    - compute shader
	* Step 4. Spatial Variance Estimation  - compute shader
	* Step 5. A-trous					- compute shader 
	*/
	 

	ROOT_INDEX_CREATOR(Prepare, passCBIndex, depthMapIndex, preDepthMapIndex, viewZMapIndex, preViewZMapIndex, depthDerivativeMapIndex)
	ROOT_INDEX_CREATOR(TA, passCBIndex, colorMapIndex, viewZMapIndex, normalMapIndex, preViewZMapIndex, preNormalMapIndex, depthDerivativeMapIndex, preColorHistoryIndex, preFastColorHistoryIndex, preHistroyLengthIndex, colorHistoryIndex, fastColorHistoryIndex, histroyLengthIndex)
	ROOT_INDEX_CREATOR(Clamp, passCBIndex, srcColorHistoryIndex, srcFastColorHistoryIndex, historyLengthIndex, destColorHistoryIndex, destFastColorHistoryIndex)
	ROOT_INDEX_CREATOR(AntiFirefly, passCBIndex, srcColorHistoryIndex, destColorHistoryIndex)
	ROOT_INDEX_CREATOR(HOT, passCBIndex, srcColorHistoryIndex, momentHistoryIndex, histroyLengthIndex, viewZMapIndex, normalMapIndex, depthDerivativeMapIndex, destColorHistoryIndex)

	//ROOT_INDEX_CREATOR(DownSampling, passCBIndex, srcMapIndex, mipmap00Index, mipmap01Index, mipmap02Index, mipmap03Index)
	//ROOT_INDEX_CREATOR(Reconstruct, passCBIndex, mipmapIndex, viewZMapIndex, depthDerivativeMapIndex, targetIndex)
	ROOT_INDEX_CREATOR(Blur, passCBIndex, colorHistoryIndex, viewZMapIndex, normalMapIndex, histroyLengthIndex, depthDerivativeMapIndex, colorMapIndex)
 
	ROOT_INDEX_CREATOR(Atorus, passCBIndex, atrousCBIndex, srcColorHistoryIndex, viewZMapIndex, normalMapIndex, histroyLengthIndex, depthDerivativeMapIndex, destColorHistoryIndex)
	ROOT_INDEX_CREATOR(Clear, passCBIndex, colorHistoryIndex, momentHistoryIndex, histroyLengthIndex, preColorHistoryIndex, preMomentHistoryIndex, preHistroyLengthIndex)
	namespace Common
	{
		static constexpr uint denoiseRange = 16;
		static constexpr float baseRadius = 2.0f;
		static constexpr float radiusRange = 8.0f;
		static constexpr uint clearUserDataFrequency = 7680;
		static constexpr uint sampleNumberMax = 64;

		static constexpr uint shdaderCount = 7;
		static JVector3<uint> GetThreadDim8()noexcept
		{
			return JVector3<uint>(8, 8, 1);
		}
		static JVector3<uint> GetThreadDim16()noexcept
		{
			return JVector3<uint>(16, 16, 1);
		}
	}
	JDx12RaytracingDenoiser::UserPrivateData::UserPrivateData(JGraphicDevice* device)
		:frameBuffer(JDx12GraphicBufferT<GIDenoiserPassConstants>(L"GiDenoisePass", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT)),
		gen(rd()),
		disUNorm(0.0f, 1.0f)
	{
		SetWaitFrame(Constants::gNumFrameResources);
		SetClearTrigger();
		frameBuffer.Build(device, Constants::gNumFrameResources);
	}
	JDx12RaytracingDenoiser::UserPrivateData::~UserPrivateData()
	{ 
		for (uint i = 0; i < historyCount; ++i)
		{
			JGraphicResourceInfo::Destroy(colorHistory[i].Release());
			JGraphicResourceInfo::Destroy(fastColorHistory[i].Release());
			JGraphicResourceInfo::Destroy(historyLength[i].Release());
		} 
		frameBuffer.Clear();
	}
	void JDx12RaytracingDenoiser::UserPrivateData::Begin(const JDrawHelper& helper)
	{
		const JUserPtr<JCamera>& cam = helper.cam;
		const JVector2F camRtSize = cam->GetRenderTargetSize();
		//const JVector2<uint> quaterRtSize = camRtSize / 4.0f;
		const JUserPtr<JScene>& scene = helper.scene;
		const size_t sceneGuid = scene->GetGuid();
		 
		GIDenoiserPassConstants constants;
		constants.camInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetInvView()));
		constants.camPreViewProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreViewProj()));
		constants.rtSize = camRtSize;
		constants.invRtSize = 1.0f / camRtSize;
		cam->GetUvToView(constants.uvToViewA, constants.uvToViewB);
		constants.camNearFar = JVector2F(cam->GetNear(), cam->GetFar());
		constants.camNearMulFar = constants.camNearFar.x * constants.camNearFar.y;
		constants.denoiseRange = Common::denoiseRange; 
		constants.baseRadius = Common::baseRadius;
		constants.radiusRange = Common::radiusRange;
		++constants.sampleNumber;
		if (constants.sampleNumber >= Common::sampleNumberMax)
			constants.sampleNumber = 0;

		frameBuffer.CopyData(helper.info.frame.currIndex, constants); 
	}
	void JDx12RaytracingDenoiser::UserPrivateData::End(const JDrawHelper& helper)
	{
		AddUpdateCount();
		SetAliveTrigger();
		++historyIndex;
		if (historyIndex >= historyCount)
			historyIndex = 0;
		preHistoryIndex = (historyCount - 1) - historyIndex;
	}

	JDx12RaytracingDenoiser::DenoiseDataSet::DenoiseDataSet(const JGraphicRtDenoiseComputeSet* computeSet, const JDrawHelper& helper)
	{
		const JDx12GraphicRtDenoiseComputeSet* set = static_cast<const JDx12GraphicRtDenoiseComputeSet*>(computeSet);
		context = static_cast<JDx12CommandContext*>(set->context);
		device = static_cast<JDx12GraphicDevice*>(set->device);
		gm = static_cast<JDx12GraphicResourceManager*>(set->gm);
		cam = helper.cam;

		auto gInterface = helper.cam->GraphicResourceUserInterface();
		auto aInterface = helper.scene->GpuAcceleratorUserInterface();

		rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		dsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		if (!rtSet.IsValid() || !dsSet.IsValid())
			return;

		const size_t sceneGuid = helper.scene->GetGuid();

		normalSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		//velocitySet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::VELOCITY);

		auto preRsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
		preDepthSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);
		preNormalSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		 
		colorSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);

		resolution = rtSet.info->GetResourceSize();
		currFrameIndex = helper.info.frame.currIndex;

		sharedata = static_cast<JDx12GraphicResourceShareData*>(set->shareData)->GetRestirTemporalAccumulationData(resolution.x, resolution.y);
		if (sharedata == nullptr)
			return;

		viewZSet = context->ComputeSet(sharedata->viewZ);
		preViewZSet = context->ComputeSet(sharedata->preViewZ);

		colorHistoryIntermediateSet00 = context->ComputeSet(sharedata->restirColorHistoryIntermediate00);
		colorHistoryIntermediateSet01 = context->ComputeSet(sharedata->restirColorHistoryIntermediate01);

		depthDerivative = context->ComputeSet(sharedata->restirDepthDerivative);
		for (uint i = 0; i < SIZE_OF_ARRAY(denoiseMipmapSet); ++i)
			denoiseMipmapSet[i] = context->ComputeSet(sharedata->restirDenoiseMipmap[i]);
	}
	void JDx12RaytracingDenoiser::DenoiseDataSet::SetUserPrivate(UserPrivateData* data, const JDrawHelper& helper)
	{
		userPrivate = data;
		if (userPrivate->colorHistory[0] == nullptr)
		{
			requestCreateDependencyData = true;
			userPrivate->SetAliveTrigger();
		}
		else
		{
			userPrivate->Begin(helper); 
			colorHistorySet = context->ComputeSet(userPrivate->colorHistory[userPrivate->historyIndex]);
			fastColorHistorySet = context->ComputeSet(userPrivate->fastColorHistory[userPrivate->historyIndex]);
			historyLengthSet = context->ComputeSet(userPrivate->historyLength[userPrivate->historyIndex]);

			preColorHistorySet = context->ComputeSet(userPrivate->colorHistory[userPrivate->preHistoryIndex]);
			preFastColorHistorySet = context->ComputeSet(userPrivate->fastColorHistory[userPrivate->preHistoryIndex]);
			preHistoryLengthSet = context->ComputeSet(userPrivate->historyLength[userPrivate->preHistoryIndex]);
		}
	}
	bool JDx12RaytracingDenoiser::DenoiseDataSet::IsValid()const noexcept
	{
		return rtSet.IsValid() && dsSet.IsValid() && sharedata != nullptr;
	}

	JDx12RaytracingDenoiser::JDx12RaytracingDenoiser(PushGraphicEventPtr pushGraphicEvPtr)
		:pushGraphicEvPtr(pushGraphicEvPtr)
	{}
	JDx12RaytracingDenoiser::~JDx12RaytracingDenoiser()
	{
		Clear();
	}
	void JDx12RaytracingDenoiser::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device))
			return;

		BuildResource(device, gM);
	}
	void JDx12RaytracingDenoiser::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12RaytracingDenoiser::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12RaytracingDenoiser::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		return false;
	}
	bool JDx12RaytracingDenoiser::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		return type == JGraphicOption::TYPE::DEBUGGING;
	}
	void JDx12RaytracingDenoiser::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{
	}
	void JDx12RaytracingDenoiser::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.newOption.debugging.requestRecompileRtDenoiseShader)
		{
			Clear();
			BuildResource(dx12Set.device, dx12Set.gm);
		}
	}
	void JDx12RaytracingDenoiser::ApplyGIDenoise(const JGraphicRtDenoiseComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet) || !helper.allowRtGi || !helper.option.rendering.restir.useDenoiser)
			return;

		DenoiseDataSet set(computeSet, helper);
		Begin(set, helper);
		if (set.requestCreateDependencyData)
		{
			using CreateDependencyF = Core::JMFunctorType<JDx12RaytracingDenoiser, void, JGraphicDevice*, JGraphicResourceManager*, UserPrivateData*, JVector2<uint>>;
			pushGraphicEvPtr(Core::UniqueBind(std::make_unique<CreateDependencyF::Functor>(&JDx12RaytracingDenoiser::CreateDependencyData, this),
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
				ClearDenoiseResource(set, helper);
			Prepare(set, helper);
			GiTemporalAccumulation(set, helper);
			GiHistoryClamping(set, helper);	 
			GiAnitiFirefly(set, helper);
			GiAtorus(set, helper);
			//GiDownSampling(set, helper);
			//GiReconstructHistory(set, helper);
			GiBlur(set, helper);
		}
		End(set, helper);
	}
	void JDx12RaytracingDenoiser::Begin(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		auto data = userPrivate.find(helper.cam->GetGuid());
		if (data == userPrivate.end())
		{
			userPrivate.emplace(helper.cam->GetGuid(), std::make_unique<UserPrivateData>(set.device));
			data = userPrivate.find(helper.cam->GetGuid());
		}
		set.SetUserPrivate(data->second.get(), helper);
	}
	void JDx12RaytracingDenoiser::Prepare(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.dsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preDepthSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.preViewZSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.depthDerivative.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(giPrepareRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Prepare::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Prepare::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Prepare::preDepthMapIndex, set.preDepthSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Prepare::viewZMapIndex, set.viewZSet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Prepare::preViewZMapIndex, set.preViewZSet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Prepare::depthDerivativeMapIndex, set.depthDerivative.GetGpuUavHandle());

		set.context->SetPipelineState(giPrepareShader.get());
		set.context->Dispatch2D(set.resolution, giPrepareShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::GiTemporalAccumulation(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.colorSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		//set.context->Transition(set.velocitySet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preViewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preNormalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.depthDerivative.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preColorHistorySet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preFastColorHistorySet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preHistoryLengthSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.colorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.fastColorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.historyLengthSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->InsertUAVBarrier(set.preColorHistorySet.holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(giTaRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(TA::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(TA::colorMapIndex, set.colorSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::normalMapIndex, set.normalSet.GetGpuSrvHandle()); 
		set.context->SetComputeRootDescriptorTable(TA::preViewZMapIndex, set.preViewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preNormalMapIndex, set.preNormalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TA::preColorHistoryIndex, set.preColorHistorySet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preFastColorHistoryIndex, set.preFastColorHistorySet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preHistroyLengthIndex, set.preHistoryLengthSet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TA::colorHistoryIndex, set.colorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(TA::fastColorHistoryIndex, set.fastColorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(TA::histroyLengthIndex, set.historyLengthSet.GetGpuUavHandle());

		set.context->SetPipelineState(giTaShader.get());
		set.context->Dispatch2D(set.resolution, giTaShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::GiHistoryClamping(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.colorHistorySet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.fastColorHistorySet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.historyLengthSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.colorHistoryIntermediateSet00.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.colorHistoryIntermediateSet01.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.colorHistoryset.holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(giHistoryClampingRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Clamp::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Clamp::srcColorHistoryIndex, set.colorHistorySet.GetGpuSrvHandle()); 
		set.context->SetComputeRootDescriptorTable(Clamp::srcFastColorHistoryIndex, set.fastColorHistorySet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Clamp::historyLengthIndex, set.historyLengthSet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(Clamp::destColorHistoryIndex, set.colorHistoryIntermediateSet00.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Clamp::destFastColorHistoryIndex, set.colorHistoryIntermediateSet01.GetGpuSrvHandle());

		set.context->SetPipelineState(giHistoryClampingShader.get());
		set.context->Dispatch2D(set.resolution, giHistoryClampingShader->dispatchInfo.threadDim.XY());

		set.context->CopyResource(set.colorHistoryIntermediateSet01.holder, set.fastColorHistorySet.holder);
	}
	void JDx12RaytracingDenoiser::GiAnitiFirefly(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.colorHistoryIntermediateSet00.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.colorHistoryIntermediateSet01.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.colorHistoryset.holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(giAntiFireflyRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(AntiFirefly::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(AntiFirefly::srcColorHistoryIndex, set.colorHistoryIntermediateSet00.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(AntiFirefly::destColorHistoryIndex, set.colorHistoryIntermediateSet01.GetGpuSrvHandle());

		set.context->SetPipelineState(giAntiFireflyShader.get());
		set.context->Dispatch2D(set.resolution, giAntiFireflyShader->dispatchInfo.threadDim.XY());
	}	 
	/*
	void JDx12RaytracingDenoiser::GiBlurHotHistory(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.colorHistoryIntermediateSet01.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.colorHistoryIntermediateSet00.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.colorHistoryset.holder);
		set.context->FlushResourceBarriers();
		
		set.context->SetComputeRootSignature(giBlurHotHistoryRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(HOT::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(HOT::srcColorHistoryIndex, set.colorHistoryIntermediateSet01.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(HOT::momentHistoryIndex, set.momentHistorySet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(HOT::histroyLengthIndex, set.historyLengthSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(HOT::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(HOT::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(HOT::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(HOT::destColorHistoryIndex, set.colorHistoryIntermediateSet00.GetGpuUavHandle());
	  
		set.context->SetPipelineState(giBlurHotHistoryShader.get());
		set.context->Dispatch2D(set.resolution, giBlurHotHistoryShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::GiDownSampling(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		if (!GetGraphicOption().debugging.testTrigger01)
			return;

		set.context->Transition(set.colorHistoryIntermediateSet00.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.denoiseMipmapSet, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, false, std::make_index_sequence<Constants::restirDenoiseMipmapCount>());
		//set.context->InsertUAVBarrier(set.colorHistoryIntermediateSet.holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(giDownSamplingRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(DownSampling::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(DownSampling::srcMapIndex, set.colorHistoryIntermediateSet00.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(DownSampling::mipmap00Index, set.denoiseMipmapSet[0].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(DownSampling::mipmap01Index, set.denoiseMipmapSet[1].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(DownSampling::mipmap02Index, set.denoiseMipmapSet[2].GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(DownSampling::mipmap03Index, set.denoiseMipmapSet[3].GetGpuUavHandle());

		set.context->SetPipelineState(giDownSamplingShader.get());
		set.context->Dispatch2D(set.resolution, giDownSamplingShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::GiReconstructHistory(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		if (!GetGraphicOption().debugging.testTrigger01)
			return;

		set.context->Transition(set.denoiseMipmapSet, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false, std::make_index_sequence<Constants::restirDenoiseMipmapCount>());
		set.context->Transition(set.colorHistoryIntermediateSet00.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.denoiseMipmapSet[0].holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(giReconstructRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Reconstruct::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Reconstruct::mipmapIndex, set.denoiseMipmapSet[0].GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reconstruct::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reconstruct::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Reconstruct::targetIndex, set.colorHistoryIntermediateSet00.GetGpuUavHandle());

		set.context->SetPipelineState(giReconstructShader.get());
		set.context->Dispatch2D(set.resolution, giReconstructShader->dispatchInfo.threadDim.XY());
	}
	*/
	void JDx12RaytracingDenoiser::GiAtorus(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.colorHistoryIntermediateSet01.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.historyLengthSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.colorHistoryIntermediateSet00.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.colorHistoryIntermediateSet.holder);
		set.context->FlushResourceBarriers();
 
		set.context->SetComputeRootSignature(giAtorusRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Atorus::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Atorus::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Atorus::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Atorus::histroyLengthIndex, set.historyLengthSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Atorus::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());
		set.context->SetPipelineState(giAtorusShader.get());

		JDx12GraphicResourceComputeSet* srcSet = &set.colorHistoryIntermediateSet01;
		JDx12GraphicResourceComputeSet* destSet = &set.colorHistoryIntermediateSet00;

		static constexpr uint stepCount = 4;
		for (uint i = 0; i < stepCount; ++i)
		{ 
			uint stepSize = 1 << i;
			set.context->SetComputeRoot32BitConstants(Atorus::atrousCBIndex, 0, stepSize);
			set.context->SetComputeRootDescriptorTable(Atorus::srcColorHistoryIndex, srcSet->GetGpuSrvHandle());
			set.context->SetComputeRootDescriptorTable(Atorus::destColorHistoryIndex, destSet->GetGpuUavHandle());
			set.context->Dispatch2D(set.resolution, giAtorusShader->dispatchInfo.threadDim.XY());
			if (i == 1)
				set.context->CopyResource(destSet->holder, set.colorHistorySet.holder);
			std::swap(srcSet, destSet);
		}
	}
	void JDx12RaytracingDenoiser::GiBlur(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		const JDx12GraphicResourceComputeSet* colorTA = &set.colorHistoryIntermediateSet01;

		set.context->Transition(colorTA->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);  
		set.context->Transition(set.colorSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->InsertUAVBarrier(colorTA->holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(giBlurRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Blur::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Blur::colorHistoryIndex, colorTA->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::histroyLengthIndex, set.historyLengthSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Blur::colorMapIndex, set.colorSet.GetGpuUavHandle());

		set.context->SetPipelineState(giBlurShader.get());
		set.context->Dispatch2D(set.resolution, giBlurShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::ClearDenoiseResource(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.colorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.fastColorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.historyLengthSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		set.context->Transition(set.preColorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.preFastColorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.preHistoryLengthSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(clearRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Clear::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Clear::colorHistoryIndex, set.colorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::momentHistoryIndex, set.fastColorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::histroyLengthIndex, set.historyLengthSet.GetGpuUavHandle());

		set.context->SetComputeRootDescriptorTable(Clear::preColorHistoryIndex, set.preColorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::preMomentHistoryIndex, set.preFastColorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::preHistroyLengthIndex, set.preHistoryLengthSet.GetGpuUavHandle());

		set.context->SetPipelineState(clearShader.get());
		set.context->Dispatch2D(set.resolution, clearShader->dispatchInfo.threadDim.XY());

		set.context->InsertUAVBarrier(set.colorHistorySet.holder);
		set.context->InsertUAVBarrier(set.fastColorHistorySet.holder);
		set.context->InsertUAVBarrier(set.historyLengthSet.holder);
		set.context->InsertUAVBarrier(set.preColorHistorySet.holder);
		set.context->InsertUAVBarrier(set.preFastColorHistorySet.holder);
		set.context->InsertUAVBarrier(set.preHistoryLengthSet.holder);
		set.context->FlushResourceBarriers();

		set.userPrivate->OffClearTrigger();
	}
	void JDx12RaytracingDenoiser::End(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.userPrivate->End(helper);
		++computeCount;
		if (computeCount >= Common::clearUserDataFrequency)
		{
			for (auto& data : userPrivate)
			{
				if (data.second->CanAlive())
					data.second->OffAliveTrigger();
				else
					userPrivate.erase(data.first);
			}
			computeCount = 0;
		}
	}
	void JDx12RaytracingDenoiser::CreateDependencyData(JGraphicDevice* device, JGraphicResourceManager* gm, UserPrivateData* userPrivate, JVector2<uint> rtSize)
	{
		JGraphicResourceCreationDesc desc;
		desc.width = rtSize.x;
		desc.height = rtSize.y;
		desc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
		desc.bindDesc.useEngineDefinedBindType = false;
		desc.textureDesc = std::make_unique<JTextureCreationDesc>();
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE;

		desc.formatHint = std::make_unique<JGraphicFormatHint>();

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;
		for (uint i = 0; i < userPrivate->historyCount; ++i)
			userPrivate->colorHistory[i] = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;
		for (uint i = 0; i < userPrivate->historyCount; ++i)
			userPrivate->fastColorHistory[i] = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32_FLOAT;
		for (uint i = 0; i < userPrivate->historyCount; ++i)
			userPrivate->historyLength[i] = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);
	}
	void JDx12RaytracingDenoiser::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		BuildRootSignature(static_cast<JDx12GraphicDevice*>(device));
		BuildPso(static_cast<JDx12GraphicDevice*>(device));
	}
	void JDx12RaytracingDenoiser::BuildRootSignature(JDx12GraphicDevice* device)
	{
		JDx12RootSignatureBuilder<Prepare::rootSlotCount> pBuilder;
		pBuilder.PushConstantsBuffer(0);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		pBuilder.Create(device->GetDevice(), L"prepareRootSignature", giPrepareRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<TA::rootSlotCount, 2> tBuilder;
		tBuilder.PushConstantsBuffer(0);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8); 
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		tBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		tBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		tBuilder.Create(device->GetDevice(), L"GiTaRootSignature", giTaRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Clamp::rootSlotCount, 1> clampBuilder;
		clampBuilder.PushConstantsBuffer(0);
		clampBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		clampBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		clampBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		clampBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		clampBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		clampBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		clampBuilder.Create(device->GetDevice(), L"GiClampingRootSignature", giHistoryClampingRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<AntiFirefly::rootSlotCount, 1> anitiBuilder;
		anitiBuilder.PushConstantsBuffer(0);
		anitiBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		anitiBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		anitiBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		anitiBuilder.Create(device->GetDevice(), L"GiAnitiFireflyRootSignature", giAntiFireflyRootSignature.GetAddressOf());
 
		/*
		* 
		JDx12RootSignatureBuilder2<HOT::rootSlotCount, 1> hBuilder;
		hBuilder.PushConstantsBuffer(0);
		hBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		hBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		hBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		hBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		hBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		hBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
		hBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); 
		hBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		hBuilder.Create(device->GetDevice(), L"GiHotRootSignature", giBlurHotHistoryRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<DownSampling::rootSlotCount, 1> dBuilder;
		dBuilder.PushConstantsBuffer(0);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		dBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		dBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		dBuilder.Create(device->GetDevice(), L"GiDownSampling01RootSignature", giDownSamplingRootSignature.GetAddressOf());
 
		JDx12RootSignatureBuilder2<Reconstruct::rootSlotCount, 1> rBuilder;
		rBuilder.PushConstantsBuffer(0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Constants::restirDenoiseMipmapCount, 0, 0);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2);
		rBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		rBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		rBuilder.Create(device->GetDevice(), L"GiReconstructHistoryRootSignature", giReconstructRootSignature.GetAddressOf());
		*/

		JDx12RootSignatureBuilder2<Atorus::rootSlotCount, 1> aBuilder;
		aBuilder.PushConstantsBuffer(0);
		aBuilder.PushConstants(1, 1);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); 
		aBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		aBuilder.Create(device->GetDevice(), L"GiAtrousRootSignature", giAtorusRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Blur::rootSlotCount, 1> bBuilder;
		bBuilder.PushConstantsBuffer(0);
		bBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		bBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		bBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		bBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		bBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		bBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		bBuilder.PushSampler(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		bBuilder.Create(device->GetDevice(), L"GiBlurRootSignature", giBlurRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder<Clear::rootSlotCount> cBuilder;
		cBuilder.PushConstantsBuffer(0);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);
		cBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);
		cBuilder.Create(device->GetDevice(), L"ClearRootSignature", clearRootSignature.GetAddressOf());
	}
	void JDx12RaytracingDenoiser::BuildPso(JDx12GraphicDevice* device)
	{
		giPrepareShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		giTaShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		giHistoryClampingShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		giAntiFireflyShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		//giBlurHotHistoryShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		//giDownSamplingShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		//giReconstructShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		giAtorusShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		giBlurShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		clearShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputePsoBulder<Common::shdaderCount> psoBuilder("JDx12RaytracingDenoiser");

		psoBuilder.PushHolder(giPrepareShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoisePrepare.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giPrepareRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(giTaShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirTemporalAccumulation.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giTaRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(giHistoryClampingShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoiseHistoryClamping.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giHistoryClampingRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(giAntiFireflyShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoiseAntifirefly.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giAntiFireflyRootSignature.Get());
		psoBuilder.Next();
 
		/*
		* 
		psoBuilder.PushHolder(giBlurHotHistoryShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoiserBlurHotHistory.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giBlurHotHistoryRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(giDownSamplingShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoiseDownSampling.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim8());
		psoBuilder.PushRootSignature(giDownSamplingRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(giReconstructShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirReconstruct.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giReconstructRootSignature.Get());
		psoBuilder.Next();
		*/

		psoBuilder.PushHolder(giAtorusShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoiseAtorus.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giAtorusRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(giBlurShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoiseBlur.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(giBlurRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(clearShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(L"RestirDenoiserClear.hlsl", L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(clearRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.Create(device->GetDevice());
	}
	void JDx12RaytracingDenoiser::ClearResource()
	{
		ClearPso();
		ClearRootSignature();
	}
	void JDx12RaytracingDenoiser::ClearRootSignature()
	{
		giPrepareRootSignature = nullptr;
		giTaRootSignature = nullptr;
		giHistoryClampingRootSignature = nullptr;
		giAntiFireflyRootSignature = nullptr;
		//giBlurHotHistoryRootSignature = nullptr;
		//giDownSamplingRootSignature = nullptr;
		//giReconstructRootSignature = nullptr;
		giAtorusRootSignature = nullptr;
		giBlurRootSignature = nullptr;
		clearRootSignature = nullptr;
	}
	void JDx12RaytracingDenoiser::ClearPso()
	{
		giPrepareShader = nullptr;
		giTaShader = nullptr;
		giHistoryClampingShader = nullptr;
		giAntiFireflyShader = nullptr;
		//giBlurHotHistoryShader = nullptr;
		//giDownSamplingShader = nullptr; 
		//giReconstructShader = nullptr;
		giAtorusShader = nullptr;
		giBlurShader = nullptr;
		clearShader = nullptr;
	}
}