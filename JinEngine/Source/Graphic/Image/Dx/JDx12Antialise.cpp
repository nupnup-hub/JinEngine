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


#include"JDx12Antialise.h"
#include"../JImageProcessingFilter.h"
#include"../../Command/Dx/JDx12CommandContext.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicTaskDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"  
#include"../../Utility/Dx/JDx12ObjectCreation.h"  
#include"../../Utility/Dx/JDx12Utility.h"  
#include"../../JGraphicUpdateHelper.h"   
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include<random>

#define USE_LUMA_INPUT_BUFFER L"USE_LUMA_INPUT_BUFFER"
#define VERTICAL_ORIENTATION L"VERTICAL_ORIENTATION"

namespace JinEngine::Graphic
{
	namespace Common
	{
		static constexpr uint clearUserDataFrequency = 7680;
	}
	namespace FXAA
	{
		ROOT_INDEX_CREATOR(, firstSrvHandleIndex, secondSrvHandleIndex, thirdSrvHandleIndex,
			firstUavHandleIndex, secondUavHandleIndex, thirdUavHandleIndex, forthUavHandleIndex, cbPassIndex)

		static constexpr uint cb32BitCount = 7;
		static constexpr uint splitCount = 2;

		static JVector3<uint> Pass1ThreadDim()
		{
			return JVector3<uint>(8, 8, 1);
		}
		static JVector3<uint> Pass2ThreadDim()
		{
			return JVector3<uint>(64, 1, 1);
		}
		static JVector3<uint> ResolveThreadDim()
		{
			return JVector3<uint>(64, 1, 1);
		}
	}
	namespace TAA
	{
		ROOT_INDEX_CREATOR(Prepare, passCBIndex, depthMapIndex, preDepthMapIndex, viewZMapIndex, preViewZMapIndex)
		ROOT_INDEX_CREATOR(TA, passCBIndex, colorMapIndex, viewZMapIndex, normalMapIndex, preViewZMapIndex, preNormalMapIndex, preColorHistoryIndex, lightPropIndex, preLightPropIndex, colorHistoryIndex)
		ROOT_INDEX_CREATOR(Sharpening, passCBIndex, historyIndex, destMapIndex)
		ROOT_INDEX_CREATOR(Clear, passCBIndex, colorHistoryIndex, preColorHistoryIndex)

		static constexpr uint sampleNumberMax = 16; 
		static constexpr uint shaderCount = 4;

		static JVector3<uint> ThreadDim()
		{
			return JVector3<uint>(16, 16, 1);
		}
	}

	JDx12Antialise::TAAUserPrivateData::TAAUserPrivateData(JGraphicDevice* device)
		:frameBuffer(JDx12GraphicBufferT<TAAPassConstants>(L"TAAPassConstants", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT))
	{
		SetWaitFrame(Constants::gNumFrameResources);
		SetClearTrigger();
		frameBuffer.Build(device, Constants::gNumFrameResources);
	}
	JDx12Antialise::TAAUserPrivateData::~TAAUserPrivateData()
	{
		for (uint i = 0; i < historyCount; ++i)
			gm->DestroyGraphicTextureResource(device, colorHistory[i].Release());
		gm->DestroyGraphicTextureResource(device, viewZ.Release());
		gm->DestroyGraphicTextureResource(device, preViewZ.Release());
		frameBuffer.Clear();
	}
	void JDx12Antialise::TAAUserPrivateData::Begin(const JDrawHelper& helper)
	{
		const JUserPtr<JCamera>& cam = helper.cam;
		const JVector2F camRtSize = cam->GetRenderTargetSize();
		//const JVector2<uint> quaterRtSize = camRtSize / 4.0f;
		const JUserPtr<JScene>& scene = helper.scene;
		const size_t sceneGuid = scene->GetGuid();

		static TAAPassConstants constants;

		constants.camInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetInvView()));
		constants.camPreInvView.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreInvView()));
		constants.camPreViewProj.StoreXM(DirectX::XMMatrixTranspose(cam->GetPreViewProj().LoadXM()));
		constants.rtSize = camRtSize;
		constants.invRtSize = 1.0f / camRtSize;
		cam->GetUvToView(constants.uvToViewA, constants.uvToViewB);
		cam->GetPreUvToView(constants.preUvToViewA, constants.preUvToViewB);

		constants.camNearFar = JVector2F(cam->GetNear(), cam->GetFar());
		constants.camNearMulFar = constants.camNearFar.x * constants.camNearFar.y;
		++constants.sampleNumber;
		if (constants.sampleNumber >= TAA::sampleNumberMax)
			constants.sampleNumber = 0;

		frameBuffer.CopyData(helper.info.frame.currIndex, constants);
	}
	void JDx12Antialise::TAAUserPrivateData::End(const JDrawHelper& helper)
	{
		AddUpdateCount();
		SetAliveTrigger();
		++historyIndex;
		if (historyIndex >= historyCount)
			historyIndex = 0;
		preHistoryIndex = (historyCount - 1) - historyIndex;
	}

	JDx12Antialise::TAADataSet::TAADataSet(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		const JDx12PostProcessComputeSet* set = static_cast<const JDx12PostProcessComputeSet*>(computeSet);
		context = static_cast<JDx12CommandContext*>(set->context);
		device = static_cast<JDx12GraphicDevice*>(set->device);
		gm = static_cast<JDx12GraphicResourceManager*>(set->gm);
		cam = helper.cam;

		imageShare = static_cast<ImageProcessingShareData*>(set->imageShareData);
		if (imageShare == nullptr)
			return;

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

		colorSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		//colorSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		destSet = context->ComputeSet(imageShare->GetUpdateWaitIntermediate());

		resolution = rtSet.info->GetResourceSize();
		currFrameIndex = helper.info.frame.currIndex;

		imageShare->AddUpdatedIndexCount();
	}
	void JDx12Antialise::TAADataSet::SetUserPrivate(TAAUserPrivateData* data, const JDrawHelper& helper)
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
			preColorHistorySet = context->ComputeSet(userPrivate->colorHistory[userPrivate->preHistoryIndex]);

			viewZSet = context->ComputeSet(userPrivate->viewZ);
			preViewZSet = context->ComputeSet(userPrivate->preViewZ);
		}
	}

	void JDx12Antialise::AABase::BuildResource(JGraphicDevice* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		Activate();
		BuildSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), info, option);
		BuildPso(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), info, option);
	}
	void JDx12Antialise::AABase::ClearResource()
	{
		DeActivate();
		ClearPso();
		ClearSignature();
	}
	void JDx12Antialise::FxaaResource::BuildSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		BuildRootSignature(device);
		BuildCommandSignature(device);
	}
	void JDx12Antialise::FxaaResource::BuildRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder2<FXAA::rootSlotCount, 1> builder;
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);
		builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3);
		builder.PushConstants(FXAA::cb32BitCount, 0);
		builder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		builder.Create(device, L"FxaaMappingRootSignature", rootSignature.GetAddressOf()); 
	}
	void JDx12Antialise::FxaaResource::BuildCommandSignature(ID3D12Device* device)
	{ 
		uint byteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
		D3D12_INDIRECT_ARGUMENT_DESC indirectDesc;
		indirectDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc;
		commandSignatureDesc.ByteStride = byteStride;
		commandSignatureDesc.NumArgumentDescs = 1;
		commandSignatureDesc.pArgumentDescs = &indirectDesc;
		commandSignatureDesc.NodeMask = 1;

		ThrowIfFailedG(device->CreateCommandSignature(&commandSignatureDesc, nullptr,
			IID_PPV_ARGS(&commandSignature)));

		commandSignature->SetName(L"FxaaCommandSignature");
	}
	void JDx12Antialise::FxaaResource::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		pass1InputLinearColor = std::make_unique<JDx12ComputeShaderDataHolder>();
		pass1InputLuma = std::make_unique<JDx12ComputeShaderDataHolder>();
		pass2Vertical = std::make_unique< JDx12ComputeShaderDataHolder>();
		pass2Horizontal = std::make_unique<JDx12ComputeShaderDataHolder>();
		resolveWork = std::make_unique<JDx12ComputeShaderDataHolder>();

		constexpr uint shaderCount = 5;
		JDx12ComputeShaderDataHolder* holderSet[shaderCount]
		{
			pass1InputLinearColor.get(),
			pass1InputLuma.get(),
			pass2Vertical.get(),
			pass2Horizontal.get(),
			resolveWork.get()
		};
		JCompileInfo compileInfoSet[shaderCount]
		{
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass1.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass1.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass2.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaPass2.hlsl"), L"main"),
			JCompileInfo(ShaderRelativePath::Postprocessing(L"FxaaResolveWork.hlsl"), L"main")
		};
		std::vector<JMacroSet> macroSet[shaderCount]
		{
			std::vector<JMacroSet>{},
			std::vector<JMacroSet>{JMacroSet{USE_LUMA_INPUT_BUFFER, std::to_wstring(1) }},
			std::vector<JMacroSet>{JMacroSet{VERTICAL_ORIENTATION, std::to_wstring(1) }},
			std::vector<JMacroSet>{},
			std::vector<JMacroSet>{}
		};
		JVector3<uint> threadDim[shaderCount]
		{
			FXAA::Pass1ThreadDim(),
			FXAA::Pass1ThreadDim(),
			FXAA::Pass2ThreadDim(),
			FXAA::Pass2ThreadDim(),
			FXAA::ResolveThreadDim()
		};

		for (uint i = 0; i < shaderCount; ++i)
		{
			JDx12ComputeShaderDataHolder* holder = holderSet[i];
			JComputeShaderInitData initData;
			initData.dispatchInfo.threadDim = threadDim[i];
			initData.macro = macroSet[i];
			initData.PushThreadDimensionMacro();

			holder->cs = JDxShaderDataUtil::CompileShader(compileInfoSet[i].filePath, initData.macro, compileInfoSet[i].functionName, L"cs_6_0");
			holder->dispatchInfo = initData.dispatchInfo;

			JDx12ComputePso::Create(device, holder, rootSignature.Get(), D3D12_PIPELINE_STATE_FLAG_NONE);
		}
	}
	void JDx12Antialise::FxaaResource::ClearSignature()
	{
		rootSignature = nullptr;
		commandSignature = nullptr;
	}
	void JDx12Antialise::FxaaResource::ClearPso()
	{
		pass1InputLinearColor = nullptr;
		pass1InputLuma = nullptr;

		pass2Vertical = nullptr;
		pass2Horizontal = nullptr;
		resolveWork = nullptr;
	}
	void JDx12Antialise::FxaaResource::Execute(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		JDx12PostProcessComputeSet* set = static_cast<JDx12PostProcessComputeSet*>(computeSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(set->context);

		auto imageShare = static_cast<ImageProcessingShareData*>(computeSet->imageShareData);
		if (imageShare == nullptr)
			return;

		const bool isInputLuma = helper.option.postProcess.useToneMapping;

		JDx12GraphicResourceComputeSet targetSet = context->ComputeSet(imageShare->GetUpdatedIntermediate());
		JDx12GraphicResourceComputeSet workCounterSet = context->ComputeSet(imageShare->fxaaWorkCounter);
		JDx12GraphicResourceComputeSet indirectSet = context->ComputeSet(imageShare->fxaaIndirectParameters);
		JDx12GraphicResourceComputeSet workerQueueSet = context->ComputeSet(imageShare->fxaaWorkerQueue);
		JDx12GraphicResourceComputeSet colorQueueSet = context->ComputeSet(imageShare->fxaaColorQueue);
		JDx12GraphicResourceComputeSet lumaSet = context->ComputeSet(imageShare->lumaUnorm);

		const JVector2F targetSize = targetSet.info->GetResourceSize();
		const JVector2F invTargetSize = 1.0f / targetSize;
		//const uint lastQueueIndex = (workerQueueSet.info->GetWidth() / 4) - 1;
		const uint lastQueueIndex = workerQueueSet.info->GetElementCount() - 1;

		context->SetComputeRootSignature(rootSignature.Get());
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 0, invTargetSize);
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 2, helper.option.postProcess.fxaaContrastThreshold);
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 3, helper.option.postProcess.fxaaSubPixelRemoval);
		context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 4, lastQueueIndex);

		// Apply algorithm to each quarter of the screen separately to reduce maximum size of work buffers.
		const uint blockWidth = targetSize.x / 2;
		const uint blockHeight = targetSize.y / 2;
		const JVector2<uint> blockSize(blockWidth, blockHeight);

		for (uint x = 0; x < 2; x++)
		{
			for (uint y = 0; y < 2; y++)
			{
				JVector2<uint> startPixel(x * blockWidth, y * blockHeight);
				context->SetComputeRoot32BitConstants(FXAA::cbPassIndex, 5, startPixel);

				// Begin by analysing the luminance buffer and setting aside high-contrast pixels in
				// work queues to be processed later.  There are horizontal edge and vertical edge work
				// queues so that the shader logic is simpler for each type of edge.
				// Counter values do not need to be reset because they are read and cleared at once.

				context->Transition(targetSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				context->Transition(workerQueueSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				context->Transition(colorQueueSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				JVector3<uint> groupDim = JVector3<uint>::Zero();
				if (isInputLuma)
				{
					context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
					context->SetPipelineState(pass1InputLuma.get());
					context->SetComputeRootDescriptorTable(FXAA::firstSrvHandleIndex, targetSet.GetGpuSrvHandle());
					context->SetComputeRootDescriptorTable(FXAA::secondSrvHandleIndex, lumaSet.GetGpuSrvHandle());
					context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, workCounterSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::secondUavHandleIndex, workerQueueSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::thirdUavHandleIndex, colorQueueSet.GetGpuUavHandle());
					context->Dispatch2D(blockSize, pass1InputLuma->dispatchInfo.threadDim.XY());
				}
				else
				{
					context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
					context->SetPipelineState(pass1InputLinearColor.get());
					context->SetComputeRootDescriptorTable(FXAA::firstSrvHandleIndex, targetSet.GetGpuSrvHandle());
					context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, workCounterSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::secondUavHandleIndex, workerQueueSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::thirdUavHandleIndex, colorQueueSet.GetGpuUavHandle());
					context->SetComputeRootDescriptorTable(FXAA::forthUavHandleIndex, lumaSet.GetGpuUavHandle());
					context->Dispatch2D(blockSize, pass1InputLinearColor->dispatchInfo.threadDim.XY());
					context->Transition(lumaSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				}

				context->SetPipelineState(resolveWork.get());
				context->Transition(indirectSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				context->InsertUAVBarrier(workCounterSet.holder, true);

				context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, indirectSet.GetGpuUavHandle());
				context->SetComputeRootDescriptorTable(FXAA::secondUavHandleIndex, workerQueueSet.GetGpuUavHandle());
				context->SetComputeRootDescriptorTable(FXAA::thirdUavHandleIndex, workCounterSet.GetGpuUavHandle());
				context->Dispatch(1, 1, 1);

				context->InsertUAVBarrier(workCounterSet.holder);
				context->Transition(indirectSet.holder, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
				context->Transition(workerQueueSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				context->Transition(colorQueueSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				context->Transition(targetSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

				context->SetComputeRootDescriptorTable(FXAA::firstSrvHandleIndex, lumaSet.GetGpuSrvHandle());
				context->SetComputeRootDescriptorTable(FXAA::secondSrvHandleIndex, workerQueueSet.GetGpuSrvHandle());
				context->SetComputeRootDescriptorTable(FXAA::thirdSrvHandleIndex, colorQueueSet.GetGpuSrvHandle());
				context->SetComputeRootDescriptorTable(FXAA::firstUavHandleIndex, targetSet.GetGpuUavHandle());

				context->SetPipelineState(pass2Horizontal.get());
				context->DispatchIndirect(commandSignature.Get(), indirectSet, 0);
				context->SetPipelineState(pass2Vertical.get());
				context->DispatchIndirect(commandSignature.Get(), indirectSet, 12);

				context->InsertUAVBarrier(targetSet.holder);
			}
		}
	}

	void JDx12Antialise::TaaResource::BuildSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		using namespace TAA;
		JDx12RootSignatureBuilder<Prepare::rootSlotCount> pBuilder;
		pBuilder.PushConstantsBuffer(0);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		pBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		pBuilder.Create(device, L"PrepareRootSignature", prepareRootSignature.GetAddressOf());

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
		tBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		tBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		tBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		tBuilder.Create(device, L"TaRootSignature", taRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder2<Sharpening::rootSlotCount, 2> sBuilder;
		sBuilder.PushConstantsBuffer(0);
		sBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); 
		sBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		sBuilder.PushSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		sBuilder.PushSampler(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		sBuilder.Create(device, L"Sharpening", sharpeningRootSignature.GetAddressOf());

		JDx12RootSignatureBuilder<Clear::rootSlotCount> clearBuilder;
		clearBuilder.PushConstantsBuffer(0);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		clearBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
		clearBuilder.Create(device, L"ClearRootSignature", clearRootSignature.GetAddressOf());
	}
	void JDx12Antialise::TaaResource::BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		using namespace TAA;
		prepare = std::make_unique<JDx12ComputeShaderDataHolder>();
		ta = std::make_unique<JDx12ComputeShaderDataHolder>();
		sharpening = std::make_unique<JDx12ComputeShaderDataHolder>();
		clearShader = std::make_unique<JDx12ComputeShaderDataHolder>();

		JDx12ComputePsoBulder<shaderCount> psoBuilder("JDx12AntialiseTAA");

		psoBuilder.PushHolder(prepare.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::TAA(L"Prepare.hlsl"), L"main"));
		psoBuilder.PushThreadDim(ThreadDim());
		psoBuilder.PushRootSignature(prepareRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(ta.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::TAA(L"TemporalAccumulation.hlsl"), L"main"));
		psoBuilder.PushThreadDim(ThreadDim());
		psoBuilder.PushRootSignature(taRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(sharpening.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::TAA(L"Sharpening.hlsl"), L"main"));
		psoBuilder.PushThreadDim(ThreadDim());
		psoBuilder.PushRootSignature(sharpeningRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.PushHolder(clearShader.get());
		psoBuilder.PushCompileInfo(JCompileInfo(ShaderRelativePath::TAA(L"Clear.hlsl"), L"main"));
		psoBuilder.PushThreadDim(ThreadDim());
		psoBuilder.PushRootSignature(clearRootSignature.Get());
		psoBuilder.Next();

		psoBuilder.Create(device);
	}
	void JDx12Antialise::TaaResource::ClearSignature()
	{
		prepareRootSignature = nullptr;
		taRootSignature = nullptr;
		sharpeningRootSignature = nullptr;
		clearRootSignature = nullptr;
	}
	void JDx12Antialise::TaaResource::ClearPso()
	{
		prepare = nullptr;
		ta = nullptr;
		sharpening = nullptr;
		clearShader = nullptr;
	}
	void JDx12Antialise::TaaResource::Prepare(TAADataSet& set, const JDrawHelper& helper)
	{
		using namespace TAA;
		set.context->Transition(set.dsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preDepthSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.preViewZSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(prepareRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Prepare::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Prepare::depthMapIndex, set.dsSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Prepare::preDepthMapIndex, set.preDepthSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(Prepare::viewZMapIndex, set.viewZSet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Prepare::preViewZMapIndex, set.preViewZSet.GetGpuUavHandle());

		set.context->SetPipelineState(prepare.get());
		set.context->Dispatch2D(set.resolution, prepare->dispatchInfo.threadDim.XY());
	}
	void JDx12Antialise::TaaResource::TemporalAccumulation(TAADataSet& set, const JDrawHelper& helper)
	{
		using namespace TAA;
		set.context->Transition(set.taSrc->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.viewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preViewZSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.preNormalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.taPreHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		set.context->Transition(set.taCurHistory->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS); 
		set.context->InsertUAVBarrier(set.taPreHistory->holder);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(taRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(TA::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(TA::colorMapIndex, set.taSrc->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::viewZMapIndex, set.viewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::normalMapIndex, set.normalSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preViewZMapIndex, set.preViewZSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preNormalMapIndex, set.preNormalSet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TA::preColorHistoryIndex, set.taPreHistory->GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::lightPropIndex, set.lightPropSet.GetGpuSrvHandle());
		set.context->SetComputeRootDescriptorTable(TA::preLightPropIndex, set.preLightPropSet.GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(TA::colorHistoryIndex, set.taCurHistory->GetGpuUavHandle());

		set.context->SetPipelineState(ta.get());
		set.context->Dispatch2D(set.resolution, ta->dispatchInfo.threadDim.XY());
	}
	void JDx12Antialise::TaaResource::Sharpening(TAADataSet& set, const JDrawHelper& helper)
	{
		using namespace TAA; 
		set.context->Transition(set.sharpHistory->holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		set.context->Transition(set.sharpDest->holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(sharpeningRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Sharpening::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Sharpening::historyIndex, set.sharpHistory->GetGpuSrvHandle());

		set.context->SetComputeRootDescriptorTable(Sharpening::destMapIndex, set.sharpDest->GetGpuUavHandle());
		 
		set.context->SetPipelineState(sharpening.get());
		set.context->Dispatch2D(set.resolution, sharpening->dispatchInfo.threadDim.XY());
	}
	void JDx12Antialise::TaaResource::ClearTAAResource(TAADataSet& set, const JDrawHelper& helper)
	{
		using namespace TAA;
		set.context->Transition(set.colorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->Transition(set.preColorHistorySet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		set.context->FlushResourceBarriers();

		set.context->SetComputeRootSignature(clearRootSignature.Get());
		set.context->SetComputeRootConstantBufferView(Clear::passCBIndex, &set.userPrivate->frameBuffer, set.currFrameIndex);
		set.context->SetComputeRootDescriptorTable(Clear::colorHistoryIndex, set.colorHistorySet.GetGpuUavHandle());
		set.context->SetComputeRootDescriptorTable(Clear::preColorHistoryIndex, set.preColorHistorySet.GetGpuUavHandle());
		set.context->SetPipelineState(clearShader.get());
		set.context->Dispatch2D(set.resolution, clearShader->dispatchInfo.threadDim.XY());

		set.context->InsertUAVBarrier(set.colorHistorySet.holder);
		set.context->InsertUAVBarrier(set.preColorHistorySet.holder);
		set.context->FlushResourceBarriers();
		set.userPrivate->OffClearTrigger();
	}
	void JDx12Antialise::TaaResource::SettingFirstLoop(TAADataSet& set, const JDrawHelper& helper)
	{
		set.taSrc = &set.colorSet;
		set.taPreHistory = &set.preColorHistorySet;
		set.taCurHistory = &set.colorHistorySet;
		 
		set.sharpHistory = &set.colorHistorySet;
		set.sharpDest = &set.destSet;
	}

	JDx12Antialise::JDx12Antialise(PushGraphicEventPtr pushGraphicEvPtr)
		:pushGraphicEvPtr(pushGraphicEvPtr)
	{
		aaBase[0] = &fxaa;
		aaBase[1] = &taa;
	}
	JDx12Antialise::~JDx12Antialise()
	{
		ClearResource();
	}
	void JDx12Antialise::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device);
		for (auto& data : taaUserPrivate)
			data.second->SetClearTrigger();
	}
	void JDx12Antialise::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Antialise::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12Antialise::HasDependency(const JGraphicInfo::TYPE type)const noexcept
	{
		return false;
	}
	bool JDx12Antialise::HasDependency(const JGraphicOption::TYPE type)const noexcept
	{
		return type == JGraphicOption::TYPE::DEBUGGING;
	}
	void JDx12Antialise::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
	{
	}
	void JDx12Antialise::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
	{
		auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
		if (set.newOption.debugging.requestRecompileTAAShader)
		{
			Clear();
			BuildResource(dx12Set.device);
		}
	}
	void JDx12Antialise::ApplyFXAA(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet) || !helper.option.postProcess.useFxaa)
			return;

		fxaa.Execute(computeSet, helper);
	}
	void JDx12Antialise::ApplyTAA(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet) || !helper.option.postProcess.useTaa)
			return;

		TAADataSet set(computeSet, helper);
		BeginTAA(set, helper);
		if (set.requestCreateDependencyData)
		{
			using CreateDependencyF = Core::JMFunctorType<JDx12Antialise, void, JGraphicDevice*, JGraphicResourceManager*, TAAUserPrivateData*, JVector2<uint>>;
			pushGraphicEvPtr(Core::UniqueBind(std::make_unique<CreateDependencyF::Functor>(&JDx12Antialise::CreateDependencyData, this),
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
			if (taa.IsActivated())
			{
				if (set.userPrivate->HasClearRequest())
					taa.ClearTAAResource(set, helper);

				taa.SettingFirstLoop(set, helper);
				taa.Prepare(set, helper);
				taa.TemporalAccumulation(set, helper);
				taa.Sharpening(set, helper);
			}
		}
		EndTAA(set, helper);
	}
	void JDx12Antialise::BeginTAA(TAADataSet& set, const JDrawHelper& helper)
	{
		auto data = taaUserPrivate.find(helper.cam->GetGuid());
		if (data == taaUserPrivate.end())
			data = taaUserPrivate.emplace(helper.cam->GetGuid(), std::make_unique<TAAUserPrivateData>(set.device)).first;
		set.SetUserPrivate(data->second.get(), helper);
	}
	void JDx12Antialise::EndTAA(TAADataSet& set, const JDrawHelper& helper)
	{
		set.userPrivate->End(helper);
		++computeCount;
		if (computeCount >= Common::clearUserDataFrequency)
		{
			Core::JVolatileStorageInterface::UpdateEnd(taaUserPrivate);
			computeCount = 0;
		}
	}
	void JDx12Antialise::RecompileShader(const JGraphicShaderCompileSet& set)
	{
		ClearResource();
		BuildResource(set.device);
	}
	void JDx12Antialise::CreateDependencyData(JGraphicDevice* device, JGraphicResourceManager* gm, TAAUserPrivateData* userPrivate, JVector2<uint> rtSize)
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

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32_FLOAT;
		desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON;
		userPrivate->viewZ = gm->CreateResource(device, desc);

		desc.formatHint->format = J_GRAPHIC_RESOURCE_FORMAT::R32_FLOAT;
		desc.type.resouce = J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON;
		userPrivate->preViewZ = gm->CreateResource(device, desc);

		userPrivate->device = device;
		userPrivate->gm = gm;
	}
	void JDx12Antialise::BuildResource(JGraphicDevice* device)
	{ 
		for (uint i = 0; i < SIZE_OF_ARRAY(aaBase); ++i)
			aaBase[i]->BuildResource(device, GetGraphicInfo(), GetGraphicOption());
	}
	void JDx12Antialise::ClearResource()
	{
		for (uint i = 0; i < SIZE_OF_ARRAY(aaBase); ++i)
			aaBase[i]->ClearResource();
	}
}