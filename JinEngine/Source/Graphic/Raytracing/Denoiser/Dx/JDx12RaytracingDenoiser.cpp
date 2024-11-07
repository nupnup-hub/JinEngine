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


#include"JDx12RaytracingDenoiser.h"
#include"../../Dx/JDx12RaytracingUtility.h"
#include"../../Dx/JDx12RaytracingConstants.h"
#include"../../../Device/Dx/JDx12GraphicDevice.h" 
#include"../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"    
#include"../../../DataSet/Dx/JDx12GraphicTaskDataSet.h"
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

#include"../../../../Develop/Debug/JDevelopDebug.h"

#define RADIUS_RATE L"RADIUS_RATE"
namespace JinEngine::Graphic
{
	//restir gi는 svgf사용시 분산의 부정확한 추정때문에 아티팩트가 발생한다(Correlated src)
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

	/*
	TAA와 관계
	Reprojection과 Clamp, Clamping의 아이디어는 동일하다.
	그러나 TAA의 입력값은 일정하나 Raytracing denoiser에 입력값은 노이즈가 껴있는 상태이다.
	따라서 History clamping과 antiFireFly은 조금 결이 다르다.
	우선 History clamping은 이전 History를 참고하여 현재 History의 값을 제한하고
	History antiFireFly은 현재 History의 최소, 최댓값을 제한한다.
	두 개의 제한모두 YCoCg공간에서 이루어진다.


	2024-08-32
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
	* Stem 1. Temporal accumulation				- compute shader     do Demodulate albedo -> Temporal accumulation
	* Step 2. history clamping					- compute shader
	* Step 3. history antiFireFly 					- compute shader
	* Step 4. Spatial Variance Estimation		- compute shader
	* Step 5. A-trous							- compute shader
	* Step 6.  Temporal Stabilization			- compute shader
	*/


	ROOT_INDEX_CREATOR(Prepare, passCBIndex, depthMapIndex, preDepthMapIndex, viewZMapIndex, preViewZMapIndex, depthDerivativeMapIndex)
		ROOT_INDEX_CREATOR(PreBlur, passCBIndex, srcColorMapIndex, viewZMapIndex, normalMapIndex, histroyLengthIndex, depthDerivativeMapIndex, destColorMapIndex)
		ROOT_INDEX_CREATOR(TA, passCBIndex, colorMapIndex, viewZMapIndex, normalMapIndex, preViewZMapIndex, preNormalMapIndex, preColorHistoryIndex, preFastColorHistoryIndex, preHistroyLengthIndex, lightPropIndex, preLightPropIndex, colorHistoryIndex, fastColorHistoryIndex, histroyLengthIndex)
		ROOT_INDEX_CREATOR(Fix, passCBIndex, srcColorHistoryIndex, srcFastColorHistoryIndex, historyLengthIndex, viewZMapIndex, normalMapIndex, depthDerivativeMapIndex, destColorHistoryIndex, destFastColorHistoryIndex)
		ROOT_INDEX_CREATOR(Clamping, passCBIndex, srcColorHistoryIndex, srcFastColorHistoryIndex, historyLengthIndex, destColorHistoryIndex)
		ROOT_INDEX_CREATOR(AntiFireFly, passCBIndex, srcColorHistoryIndex, destColorHistoryIndex)
		ROOT_INDEX_CREATOR(HOT, passCBIndex, srcColorHistoryIndex, FastHistoryIndex, histroyLengthIndex, viewZMapIndex, normalMapIndex, depthDerivativeMapIndex, destColorHistoryIndex)

		//ROOT_INDEX_CREATOR(DownSampling, passCBIndex, srcMapIndex, mipmap00Index, mipmap01Index, mipmap02Index, mipmap03Index)
		//ROOT_INDEX_CREATOR(Reconstruct, passCBIndex, mipmapIndex, viewZMapIndex, depthDerivativeMapIndex, targetIndex)
		ROOT_INDEX_CREATOR(Stabilization, passCBIndex, colorHistoryIndex, viewZMapIndex, normalMapIndex, histroyLengthIndex, depthDerivativeMapIndex, colorMapIndex)
		ROOT_INDEX_CREATOR(Atrous, passCBIndex, atrousCBIndex, srcColorHistoryIndex, viewZMapIndex, normalMapIndex, histroyLengthIndex, depthDerivativeMapIndex, destColorHistoryIndex)
		ROOT_INDEX_CREATOR(Clear, passCBIndex, colorHistoryIndex, FastHistoryIndex, histroyLengthIndex, preColorHistoryIndex, preFastHistoryIndex, preHistroyLengthIndex)
		namespace Common
	{
		static constexpr uint denoiseRange = 16;
		static constexpr float baseRadius = 4.0f;
		static constexpr float radiusRange = 8.0f;
		static constexpr uint sampleNumberMax = 64;

		static constexpr uint recursiveCount = 1;
		static constexpr bool useFixedHistoryIndex = (recursiveCount % 2) == 0;

		static constexpr uint shdaderCount = 7 + 3;  // stabilization variation = 3
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
		:frameBuffer(JDx12GraphicBufferT<GIDenoiserPassConstants>(L"GiDenoisePass", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT))
	{
		SetClearTrigger();
		frameBuffer.Build(device, Constants::gNumFrameResources);
	}
	JDx12RaytracingDenoiser::UserPrivateData::~UserPrivateData()
	{
		for (uint i = 0; i < historyCount; ++i)
		{
			gm->DestroyGraphicTextureResource(device, colorHistory[i].Release());
			gm->DestroyGraphicTextureResource(device, fastColorHistory[i].Release());
			gm->DestroyGraphicTextureResource(device, historyLength[i].Release());
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

		static GIDenoiserPassConstants constants;

		constants.camInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetInvView()));
		constants.camPreInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreInvView()));
		constants.camPreViewProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreViewProj().LoadXM()));
		constants.rtSize = camRtSize;
		constants.invRtSize = 1.0f / camRtSize;
		cam->GetUvToView(constants.uvToViewA, constants.uvToViewB);
		cam->GetPreUvToView(constants.preUvToViewA, constants.preUvToViewB);

		constants.camNearFar = JVector2F(cam->GetNear(), cam->GetFar());
		constants.camNearMulFar = constants.camNearFar.x * constants.camNearFar.y;
		constants.denoiseRange = Common::denoiseRange;
		constants.baseRadius = Common::baseRadius;
		constants.radiusRange = Common::radiusRange;
		constants.sampleNumber = sampleNumber;

		if (sampleNumber >= Common::sampleNumberMax)
			sampleNumber = 0;
		frameBuffer.CopyData(helper.info.frame.currIndex, constants);
	}
	void JDx12RaytracingDenoiser::UserPrivateData::End(const JDrawHelper& helper)
	{
		AddUpdateCount();
		if constexpr (!Common::useFixedHistoryIndex)
		{
			++historyIndex;
			if (historyIndex >= historyCount)
				historyIndex = 0;
			preHistoryIndex = (historyCount - 1) - historyIndex;
		}
	}

	JDx12RaytracingDenoiser::DenoiseDataSet::DenoiseDataSet(const JGraphicRtDenoiseComputeSet* computeSet, const JDrawHelper& helper)
	{
		const JDx12GraphicRtDenoiseComputeSet* set = static_cast<const JDx12GraphicRtDenoiseComputeSet*>(computeSet);
		context = static_cast<JDx12CommandContext*>(set->context);
		device = static_cast<JDx12GraphicDevice*>(set->device);
		gm = static_cast<JDx12GraphicResourceManager*>(set->gm);
		cam = helper.cam;

		auto gInterface = helper.GetResourceInterface();
		auto aInterface = helper.GetGpuAcceleratorInterface();

		rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		dsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		if (!rtSet.IsValid() || !dsSet.IsValid())
			return;

		const size_t sceneGuid = helper.scene->GetGuid();

		lightPropSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
		normalSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);

		auto preRsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA);
		preDepthSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::STORE_PREVIOUS_FRAME_DATA);

		preLightPropSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::LIGHTING_PROPERTY);
		preNormalSet = context->ComputeSet(preRsSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);

		colorSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::RAYTRACING_GI);

		resolution = rtSet.info->GetResourceSize();
		currFrameIndex = helper.info.frame.currIndex;

		taShareData = static_cast<JDx12GraphicResourceShareData*>(set->shareData)->GetRestirTemporalAccumulationData(resolution.x, resolution.y);
		drawSceneShareData = static_cast<JDx12GraphicResourceShareData*>(set->shareData)->GetDrawSceneData(helper.cam->GetGuid(), resolution);
		if (taShareData == nullptr || drawSceneShareData == nullptr || !drawSceneShareData->IsValid())
			return;

		colorHistoryIntermediateSet00 = context->ComputeSet(taShareData->restirColorHistoryIntermediate00);
		colorHistoryIntermediateSet01 = context->ComputeSet(taShareData->restirColorHistoryIntermediate01);
		for (uint i = 0; i < SIZE_OF_ARRAY(denoiseMipmapSet); ++i)
			denoiseMipmapSet[i] = context->ComputeSet(taShareData->restirDenoiseMipmap[i]);

		depthDerivative = context->ComputeSet(drawSceneShareData->depthDerivativeMap);
		viewZSet = context->ComputeSet(drawSceneShareData->viewZMap);
		preViewZSet = context->ComputeSet(drawSceneShareData->preViewZMap);
	}
	void JDx12RaytracingDenoiser::DenoiseDataSet::SetUserPrivate(UserPrivateData* data, const JDrawHelper& helper)
	{
		userPrivate = data;
		if (userPrivate->colorHistory[0] == nullptr)
			requestCreateDependencyData = true;
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
		return rtSet.IsValid() && dsSet.IsValid() && taShareData != nullptr && drawSceneShareData != nullptr && drawSceneShareData->IsValid();
	}

	void JDx12RaytracingDenoiser::DenoiserBase::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		Activate();
		BuildRootSignature(static_cast<JDx12GraphicDevice*>(device));
		BuildPso(static_cast<JDx12GraphicDevice*>(device));
	}
	void JDx12RaytracingDenoiser::DenoiserBase::ClearResource()
	{
		DeActivate();
		ClearPso();
		ClearRootSignature();
	}

	JDx12RaytracingDenoiser::RestirDenoiser::~RestirDenoiser()
	{
		ClearResource();
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::BuildRootSignature(JDx12GraphicDevice* device)
	{ 
		JDx12RootSignatureBuilder2<PreBlur::rootSlotCount, 1> preBlurBuilder;
		preBlurBuilder.PushConstantsBuffer(0);
		preBlurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		preBlurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		preBlurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		preBlurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		preBlurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		preBlurBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		preBlurBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		preBlurBuilder.Create(device->GetDevice(), L"PreBlurRootSignature", preBlurRootSignature.GetAddressOf());

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
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		tBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		tBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		tBuilder.Create(device->GetDevice(), L"TaRootSignature", taRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Fix::rootSlotCount, 1> fixBuilder;
		fixBuilder.PushConstantsBuffer(0);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		fixBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		fixBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		fixBuilder.Create(device->GetDevice(), L"HistoryFixRootSignature", historyFixRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Clamping::rootSlotCount, 1> clampingBuilder;
		clampingBuilder.PushConstantsBuffer(0);
		clampingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		clampingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		clampingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		clampingBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		clampingBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		clampingBuilder.Create(device->GetDevice(), L"ClampingRootSignature", historyClampingRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<AntiFireFly::rootSlotCount, 1> antiFireFlyBuilder;
		antiFireFlyBuilder.PushConstantsBuffer(0);
		antiFireFlyBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		antiFireFlyBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		antiFireFlyBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		antiFireFlyBuilder.Create(device->GetDevice(), L"ClampingRootSignature", antiFireFlyRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Atrous::rootSlotCount, 1> aBuilder;
		aBuilder.PushConstantsBuffer(0);
		aBuilder.PushConstants(1, 1);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		aBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		aBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		aBuilder.Create(device->GetDevice(), L"AtrousRootSignature", atorusRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Stabilization::rootSlotCount, 1> stabilizationBuilder;
		stabilizationBuilder.PushConstantsBuffer(0);
		stabilizationBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		stabilizationBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		stabilizationBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		stabilizationBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		stabilizationBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
		stabilizationBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		stabilizationBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		stabilizationBuilder.Create(device->GetDevice(), L"HistoryStabilizationRootSignature", historyStabilizationRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder<Clear::rootSlotCount> clearBuilder;
		clearBuilder.PushConstantsBuffer(0);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5);
		clearBuilder.Create(device->GetDevice(), L"ClearRootSignature", clearRootSignature.GetAddressOf());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::BuildPso(JDx12GraphicDevice* device)
	{ 
		preBlurShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		taShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		historyFixShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		historyClampingShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		antiFireFlyShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		atorusShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		firstHistoryStabilizationShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		secondHistoryStabilizationShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		thirdHistoryStabilizationShader = std::make_unique<JDx12ComputeShaderDataHolder>();
		clearShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputePsoBulder<Common::shdaderCount> psoBuilder("JDx12RaytracingDenoiser");
		 
		psoBuilder.PushHolder(preBlurShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"PreBlur.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(preBlurRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(taShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"TemporalAccumulation.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(taRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(historyFixShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"HistoryFix.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(historyFixRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(historyClampingShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"HistoryClamping.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(historyClampingRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(antiFireFlyShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"AnitiFireFly.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(antiFireFlyRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(atorusShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"Atrous.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(atorusRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(firstHistoryStabilizationShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"HistoryStabilization.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushMacroSet(JMacroSet{ RADIUS_RATE, std::to_wstring(1.0f) });
		psoBuilder.PushRootSignature(historyStabilizationRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(secondHistoryStabilizationShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"HistoryStabilization.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushMacroSet(JMacroSet{ RADIUS_RATE, std::to_wstring(0.5f) });
		psoBuilder.PushRootSignature(historyStabilizationRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(thirdHistoryStabilizationShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"HistoryStabilization.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushMacroSet(JMacroSet{ RADIUS_RATE, std::to_wstring(0.25f) });
		psoBuilder.PushRootSignature(historyStabilizationRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(clearShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::RestirDenoise(L"Clear.hlsl"), L"main"));
		psoBuilder.PushThreadDim(Common::GetThreadDim16());
		psoBuilder.PushRootSignature(clearRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.Create(device->GetDevice());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::ClearRootSignature()
	{ 
		preBlurRootSignature = nullptr;
		taRootSignature = nullptr;
		historyFixRootSignature = nullptr;
		historyClampingRootSignature = nullptr;
		antiFireFlyRootSignature = nullptr;
		//giBlurHotHistoryRootSignature = nullptr;
		//giDownSamplingRootSignature = nullptr;
		//giReconstructRootSignature = nullptr;
		atorusRootSignature = nullptr;
		historyStabilizationRootSignature = nullptr;
		clearRootSignature = nullptr;
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::ClearPso()
	{ 
		preBlurShader = nullptr;
		taShader = nullptr;
		historyFixShader = nullptr;
		historyClampingShader = nullptr;
		antiFireFlyShader = nullptr;
		//giBlurHotHistoryShader = nullptr;
		//giDownSamplingShader = nullptr; 
		//giReconstructShader = nullptr;
		atorusShader = nullptr;
		firstHistoryStabilizationShader = nullptr;
		secondHistoryStabilizationShader = nullptr;
		thirdHistoryStabilizationShader = nullptr;
		clearShader = nullptr;
	}

	void JDx12RaytracingDenoiser::RestirDenoiser::PreBlur(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.preBlurSrc->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preBlurDest->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(preBlurRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(PreBlur::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(PreBlur::srcColorMapIndex, set.preBlurSrc->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(PreBlur::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(PreBlur::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(PreBlur::histroyLengthIndex, set.preBlurHistoryLength->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(PreBlur::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(PreBlur::destColorMapIndex, set.preBlurDest->GetGpuUavHandle());

		set.context->SetPipelineState(preBlurShader.get());
		set.context->Dispatch2D(set.resolution, preBlurShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::TemporalAccumulation(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.taSrc->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preViewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preNormalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.taPreColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.taPreFastColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.taPreHistoryLength->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		set.context->Transition(set.taColorHistory->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.taFastColorHistory->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.taHistoryLength->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->InsertUAVBarrier(set.taColorHistory->holder);
		set.context->InsertUAVBarrier(set.taPreColorHistory->holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(taRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(TA::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(TA::colorMapIndex, set.taSrc->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preViewZMapIndex, set.preViewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preNormalMapIndex, set.preNormalSet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TA::preColorHistoryIndex, set.taPreColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preFastColorHistoryIndex, set.taPreFastColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preHistroyLengthIndex, set.taPreHistoryLength->GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TA::lightPropIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preLightPropIndex, set.preLightPropSet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TA::colorHistoryIndex, set.taColorHistory->GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(TA::fastColorHistoryIndex, set.taFastColorHistory->GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(TA::histroyLengthIndex, set.taHistoryLength->GetGpuUavHandle());

		set.context->SetPipelineState(taShader.get());
		set.context->Dispatch2D(set.resolution, taShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::HistoryFix(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.fixSrcColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.fixSrcFastColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.fixHistoryLength->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.fixDestColorHistory->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.fixDestFastColorHistory->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.depthDerivative.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		//set.context->InsertUAVBarrier(set.colorHistoryset.holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(historyFixRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Fix::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Fix::srcColorHistoryIndex, set.fixSrcColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Fix::srcFastColorHistoryIndex, set.fixSrcFastColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Fix::historyLengthIndex, set.fixHistoryLength->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Fix::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Fix::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Fix::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(Fix::destColorHistoryIndex, set.fixDestColorHistory->GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Fix::destFastColorHistoryIndex, set.fixDestFastColorHistory->GetGpuUavHandle());

		set.context->SetPipelineState(historyFixShader.get());
		set.context->Dispatch2D(set.resolution, historyFixShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::HistoryClamping(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.clampingSrcColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.clampingSrcFastColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.clampingHistoryLength->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.clampingDestColorHistory->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.colorHistoryset.holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(historyClampingRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Clamping::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Clamping::srcColorHistoryIndex, set.clampingSrcColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Clamping::srcFastColorHistoryIndex, set.clampingSrcFastColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Clamping::historyLengthIndex, set.clampingHistoryLength->GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(Clamping::destColorHistoryIndex, set.clampingDestColorHistory->GetGpuUavHandle());

		set.context->SetPipelineState(historyClampingShader.get());
		set.context->Dispatch2D(set.resolution, historyClampingShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::AnitiFireFly(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.antiFireFlySrcColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.antiFireFlyDestColorHistory->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		//set.context->InsertUAVBarrier(set.colorHistoryset.holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(antiFireFlyRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(AntiFireFly::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(AntiFireFly::srcColorHistoryIndex, set.antiFireFlySrcColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(AntiFireFly::destColorHistoryIndex, set.antiFireFlyDestColorHistory->GetGpuUavHandle());

		set.context->SetPipelineState(antiFireFlyShader.get());
		set.context->Dispatch2D(set.resolution, antiFireFlyShader->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::Atrous(DenoiseDataSet& set, const JDrawHelper& helper, const uint stepCount)
	{
		set.context->Transition(set.atrousHistoryLength->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.atrousPing->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.atrousPong->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->InsertUAVBarrier(set.atrousPing->holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(atorusRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Atrous::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Atrous::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Atrous::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Atrous::histroyLengthIndex, set.atrousHistoryLength->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Atrous::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());
		set.context->SetPipelineState(atorusShader.get());

		JDx12GraphicResourceComputeSet* srcSet = set.atrousPing;
		JDx12GraphicResourceComputeSet* destSet = set.atrousPong;

		for (uint i = 0; i < stepCount; ++i)
		{
			uint stepSize = 1 << i;
			set.context->SetComputeRoot32BitConstants(Atrous::atrousCBIndex, 0, stepSize);
			set.context->SetComputeRootDescriptorTable(Atrous::srcColorHistoryIndex, srcSet->GetGpuSrvHandle());
			set.context->SetComputeRootDescriptorTable(Atrous::destColorHistoryIndex, destSet->GetGpuUavHandle());
			set.context->Dispatch2D(set.resolution, atorusShader->dispatchInfo.threadDim.XY());

			JDx12GraphicResourceComputeSet* temp = srcSet;
			srcSet = destSet;
			destSet = temp;
			//std::swap(srcSet, destSet);
		}
		//srcSet = last updaetd destSet 
		set.context->CopyResource(srcSet->holder, set.atrousColorHistory->holder);
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::HistoryStabilization(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.context->Transition(set.stabSrcColorHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.stabDestColorMap->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->InsertUAVBarrier(set.stabSrcColorHistory->holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(historyStabilizationRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Stabilization::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Stabilization::colorHistoryIndex, set.stabSrcColorHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Stabilization::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Stabilization::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Stabilization::histroyLengthIndex, set.stabHistoryLength->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Stabilization::depthDerivativeMapIndex, set.depthDerivative.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Stabilization::colorMapIndex, set.stabDestColorMap->GetGpuUavHandle());

		JDx12ComputeShaderDataHolder* shaderPtr = nullptr;
		if (set.loopCount == 0)
			shaderPtr = firstHistoryStabilizationShader.get();
		else if (set.loopCount == 1)
			shaderPtr = secondHistoryStabilizationShader.get();
		else
			shaderPtr = thirdHistoryStabilizationShader.get();

		set.context->SetPipelineState(shaderPtr);
		set.context->Dispatch2D(set.resolution, shaderPtr->dispatchInfo.threadDim.XY());
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::ClearDenoiseResource(const DenoiseDataSet& set, const JDrawHelper& helper)
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
		set.context->SetComputeRootDescriptorTable(Clear::FastHistoryIndex, set.fastColorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::histroyLengthIndex, set.historyLengthSet.GetGpuUavHandle());

		set.context->SetComputeRootDescriptorTable(Clear::preColorHistoryIndex, set.preColorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::preFastHistoryIndex, set.preFastColorHistorySet.GetGpuUavHandle());
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
	void JDx12RaytracingDenoiser::RestirDenoiser::SettingFirstLoop(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.preBlurSrc = &set.colorSet;
		set.preBlurDest = &set.colorHistoryIntermediateSet00;
		set.preBlurHistoryLength = &set.historyLengthSet;

		set.taSrc = &set.colorHistoryIntermediateSet00;
		set.taColorHistory = &set.colorHistorySet;
		set.taFastColorHistory = &set.fastColorHistorySet;
		set.taHistoryLength = &set.historyLengthSet;
		set.taPreColorHistory = &set.preColorHistorySet;
		set.taPreFastColorHistory = &set.preFastColorHistorySet;
		set.taPreHistoryLength = &set.preHistoryLengthSet;

		set.fixSrcColorHistory = &set.colorHistorySet;
		set.fixSrcFastColorHistory = &set.fastColorHistorySet;
		set.fixDestColorHistory = &set.colorHistoryIntermediateSet00;
		set.fixDestFastColorHistory = &set.colorHistoryIntermediateSet01;
		set.fixHistoryLength = &set.historyLengthSet;

		set.clampingSrcColorHistory = &set.colorHistoryIntermediateSet00;
		set.clampingSrcFastColorHistory = &set.colorHistoryIntermediateSet01;
		set.clampingDestColorHistory = &set.colorHistorySet;
		set.clampingHistoryLength = &set.historyLengthSet;

		set.antiFireFlySrcColorHistory = set.clampingDestColorHistory;
		set.antiFireFlyDestColorHistory = &set.colorHistoryIntermediateSet00;

		//4 loop ... last loop is ping <- pong
		set.atrousPing = &set.colorHistoryIntermediateSet00;
		set.atrousPong = &set.colorHistoryIntermediateSet01;
		set.atrousColorHistory = &set.colorHistorySet;
		set.atrousHistoryLength = &set.historyLengthSet;

		set.stabSrcColorHistory = set.atrousPing;
		set.stabDestColorMap = &set.colorSet;
		set.stabHistoryLength = &set.historyLengthSet;

		set.loopCount = 0;
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::SettingSecondLoop(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.taSrc = &set.colorSet;
		set.taColorHistory = &set.preColorHistorySet;
		set.taFastColorHistory = &set.preFastColorHistorySet;
		set.taHistoryLength = &set.preHistoryLengthSet;
		set.taPreColorHistory = &set.colorHistorySet;
		set.taPreFastColorHistory = &set.fastColorHistorySet;
		set.taPreHistoryLength = &set.historyLengthSet;

		set.fixSrcColorHistory = &set.preColorHistorySet;
		set.fixSrcFastColorHistory = &set.preFastColorHistorySet;
		set.fixDestColorHistory = &set.colorHistoryIntermediateSet00;
		set.fixDestFastColorHistory = &set.colorHistoryIntermediateSet01;
		set.fixHistoryLength = &set.preHistoryLengthSet;

		set.clampingSrcColorHistory = &set.colorHistoryIntermediateSet00;
		set.clampingSrcFastColorHistory = &set.colorHistoryIntermediateSet01;
		set.clampingDestColorHistory = &set.preColorHistorySet;
		set.clampingHistoryLength = &set.preHistoryLengthSet;

		set.antiFireFlySrcColorHistory = set.clampingDestColorHistory;
		set.antiFireFlyDestColorHistory = &set.colorHistoryIntermediateSet00;

		//4 loop ... last loop is ping <- pong
		set.atrousPing = &set.colorHistoryIntermediateSet00;
		set.atrousPong = &set.colorHistoryIntermediateSet01;
		set.atrousColorHistory = &set.preColorHistorySet;
		set.atrousHistoryLength = &set.preHistoryLengthSet;

		set.stabSrcColorHistory = set.atrousPing;
		set.stabDestColorMap = &set.colorSet;
		set.stabHistoryLength = &set.preHistoryLengthSet;

		set.loopCount = 1;
	}
	void JDx12RaytracingDenoiser::RestirDenoiser::SettingThirdLoop(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.taSrc = &set.colorSet;
		set.taColorHistory = &set.colorHistorySet;
		set.taFastColorHistory = &set.fastColorHistorySet;
		set.taHistoryLength = &set.historyLengthSet;
		set.taPreColorHistory = &set.preColorHistorySet;
		set.taPreFastColorHistory = &set.preFastColorHistorySet;
		set.taPreHistoryLength = &set.preHistoryLengthSet;

		set.fixSrcColorHistory = &set.colorHistorySet;
		set.fixSrcFastColorHistory = &set.fastColorHistorySet;
		set.fixDestColorHistory = &set.colorHistoryIntermediateSet00;
		set.fixDestFastColorHistory = &set.colorHistoryIntermediateSet01;
		set.fixHistoryLength = &set.historyLengthSet;

		set.clampingSrcColorHistory = &set.colorHistoryIntermediateSet00;
		set.clampingSrcFastColorHistory = &set.colorHistoryIntermediateSet01;
		set.clampingDestColorHistory = &set.colorHistorySet;
		set.clampingHistoryLength = &set.historyLengthSet;

		set.antiFireFlySrcColorHistory = set.clampingDestColorHistory;
		set.antiFireFlyDestColorHistory = &set.colorHistoryIntermediateSet00;

		//4 loop ... last loop is ping <- pong
		set.atrousPing = &set.colorHistoryIntermediateSet00;
		set.atrousPong = &set.colorHistoryIntermediateSet01;
		set.atrousColorHistory = &set.colorHistorySet;
		set.atrousHistoryLength = &set.historyLengthSet;

		set.stabSrcColorHistory = set.atrousPing;
		set.stabDestColorMap = &set.colorSet;
		set.stabHistoryLength = &set.historyLengthSet;

		set.loopCount = 2;
	}

	JDx12RaytracingDenoiser::JDx12RaytracingDenoiser(PushGraphicEventPtr pushGraphicEvPtr)
		:pushGraphicEvPtr(pushGraphicEvPtr)
	{
		denoiser[0] = &restirDenoiser;
	}
	JDx12RaytracingDenoiser::~JDx12RaytracingDenoiser()
	{
		ClearResource();
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
	bool JDx12RaytracingDenoiser::HasDrawSequencePostProcessing()const noexcept
	{
		return true;
	} 
	void JDx12RaytracingDenoiser::DrawSequencePostProcessing()
	{
		GraphicVolatileStorageInterface::UpdateEnd(userPrivate);
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
		if (!set.IsValid())
			return;

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
			if (restirDenoiser.IsActivated())
			{
				if (set.userPrivate->HasClearRequest())
					restirDenoiser.ClearDenoiseResource(set, helper);

				restirDenoiser.SettingFirstLoop(set, helper); 
				restirDenoiser.PreBlur(set, helper);
				restirDenoiser.TemporalAccumulation(set, helper);
				restirDenoiser.HistoryFix(set, helper);
				restirDenoiser.HistoryClamping(set, helper);
				restirDenoiser.AnitiFireFly(set, helper);
				restirDenoiser.Atrous(set, helper, 4);
				restirDenoiser.HistoryStabilization(set, helper);

				if constexpr (Common::recursiveCount > 1)
				{
					restirDenoiser.SettingSecondLoop(set, helper);
					restirDenoiser.TemporalAccumulation(set, helper);
					restirDenoiser.HistoryFix(set, helper);
					restirDenoiser.HistoryClamping(set, helper);
					restirDenoiser.AnitiFireFly(set, helper);
					restirDenoiser.Atrous(set, helper, 4);
					restirDenoiser.HistoryStabilization(set, helper);
				}
				if constexpr (Common::recursiveCount > 2)
				{
					restirDenoiser.SettingThirdLoop(set, helper);
					restirDenoiser.TemporalAccumulation(set, helper);
					restirDenoiser.HistoryFix(set, helper);
					restirDenoiser.HistoryClamping(set, helper);
					restirDenoiser.AnitiFireFly(set, helper);
					restirDenoiser.Atrous(set, helper, 4);
					restirDenoiser.HistoryStabilization(set, helper);
				}
			}
		}
		End(set, helper);
	}
	void JDx12RaytracingDenoiser::Begin(DenoiseDataSet& set, const JDrawHelper& helper)
	{
		auto data = userPrivate.find(helper.cam->GetGuid());
		if (data == userPrivate.end())
			data = userPrivate.emplace(helper.cam->GetGuid(), std::make_unique<UserPrivateData>(set.device)).first;
		set.SetUserPrivate(data->second.get(), helper);
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
		set.context->SetComputeRootDescriptorTable(HOT::FastHistoryIndex, set.momentHistorySet.GetGpuSrvHandle());
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
	void JDx12RaytracingDenoiser::End(const DenoiseDataSet& set, const JDrawHelper& helper)
	{
		set.userPrivate->End(helper);
	}
	void JDx12RaytracingDenoiser::CreateDependencyData(JGraphicDevice* device, JGraphicResourceManager* gm, UserPrivateData* userPrivate, JVector2<uint> rtSize)
	{
		JGraphicResourceTypeSet typeSet(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON, J_GRAPHIC_TASK_TYPE::UNKNOWN);
		JGraphicResourceCreationDesc desc(typeSet);
		desc.width = rtSize.x;
		desc.height = rtSize.y;
		desc.bindDesc.requestAdditionalBind[(uint)J_GRAPHIC_BIND_TYPE::UAV] = true;
		desc.bindDesc.useEngineDefinedBindType = false;
		desc.textureDesc = std::make_unique<JTextureCreationDesc>();
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE;

		desc.formatHint = std::make_unique<JGraphicFormatHint>();
		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;
		desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON;
		for (uint i = 0; i < userPrivate->historyCount; ++i)
			userPrivate->colorHistory[i] = gm->CreateResource(device, desc);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R16G16B16A16_UNORM;
		desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON;
		for (uint i = 0; i < userPrivate->historyCount; ++i)
			userPrivate->fastColorHistory[i] = gm->CreateResource(device, desc);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32_UINT;
		desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON;
		for (uint i = 0; i < userPrivate->historyCount; ++i)
			userPrivate->historyLength[i] = gm->CreateResource(device, desc);
		 
		userPrivate->device = device;
		userPrivate->gm = gm;
	}
	void JDx12RaytracingDenoiser::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		//추후에 옵션추가해서 reCurrentDenoiser도 선택가능하게 기능 구현할 필요가 있다.
		restirDenoiser.BuildResource(device, gM);

		for (auto& data : userPrivate)
			data.second->SetClearTrigger();
	}
	void JDx12RaytracingDenoiser::ClearResource()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(denoiser); ++i)
			denoiser[i]->ClearResource();
	}
}