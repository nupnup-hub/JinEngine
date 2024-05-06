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


#include"JDx12Adaptee.h"
#include"../JGraphicUpdateHelper.h"
#include"../Accelerator/Dx/JDx12GpuAcceleratorManager.h"
#include"../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../Debug/Dx/JDx12GraphicDebug.h"
#include"../DepthMap/Dx/JDx12DepthTest.h"
#include"../Device/Dx/JDx12GraphicDevice.h"
#include"../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../Culling/Dx/JDx12CullingManager.h"
#include"../Culling/Occlusion/Dx/JDx12HardwareOccCulling.h"
#include"../Culling/Occlusion/Dx/JDx12HZBOccCulling.h"
#include"../Culling/Light/Dx/JDx12LightCulling.h"
#include"../Culling/Frustum/JFrustumCulling.h"
#include"../Scene/Dx/JDx12SceneDraw.h"
#include"../ShadowMap/Dx/JDx12ShadowMap.h" 
#include"../ShadowMap/JCsmManager.h"
#include"../Outline/Dx/JDx12Outline.h"
#include"../Image/JPostProcessPipeline.h"
#include"../Image/Dx/JDx12Blur.h"
#include"../Image/Dx/JDx12DownSampling.h"
#include"../Image/Dx/JDx12Ssao.h"
#include"../Image/Dx/JDx12ToneMapping.h"
#include"../Image/Dx/JDx12Bloom.h"
#include"../Image/Dx/JDx12Antialise.h"
#include"../Image/Dx/JDx12PostProcessHistogram.h"
#include"../Image/Dx/JDx12PostProcessExposure.h"
#include"../Image/Dx/JDx12ConvertColor.h"
#include"../Raytracing/Light/Global/Dx/JDx12RaytracingGI.h"
#include"../Raytracing/Occlusion/Dx/JDx12RaytracingAmbientOcclusion.h" 
#include"../Raytracing/Denoiser/Dx/JDx12RaytracingDenoiser.h" 
#include"../FrameResource/Dx/JDx12FrameResource.h"
#include"../Utility/Dx/JDx12Utility.h" 
#include"../Utility/Dx/JDx12ClearBuffer.h"
#include"../Command/Dx/JDx12CommandContext.h"

#define CAST_DX12_CONT(x) static_cast<JDx12CommandContext*>(x)

namespace JinEngine::Graphic
{
	namespace
	{
		static void SettingDescriptorHeaps(JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList)
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { dx12Gm->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
			cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		}
		static bool IsValid(const JGraphicDrawReferenceSet& drawRefSet)
		{
			return drawRefSet.IsValid();
			/*
			return drawRefSet.IsValid() && JGraphicDeviceUser::IsAllSameDevice<J_GRAPHIC_DEVICE_TYPE::DX12>(drawRefSet.device,
				drawRefSet.graphicResourceM,
				drawRefSet.currFrame,
				drawRefSet.cullingM,
				drawRefSet.depthTest,
				drawRefSet.depthDebug,
				drawRefSet.blur,
				drawRefSet.downSample,
				drawRefSet.ssao,
				drawRefSet.shareData);
			*/
		}

		static void BeginContext(JDx12CommandContext* context,
			const JGraphicDrawReferenceSet& drawRefSet,
			ID3D12GraphicsCommandList* cmd,
			const bool useRaycastDevice = false)
		{
			context->Begin();
			context->Initialize(cmd,
				static_cast<JDx12GraphicDevice*>(drawRefSet.device),
				static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM),
				static_cast<JDx12CullingManager*>(drawRefSet.cullingM),
				static_cast<JDx12GpuAcceleratorManager*>(drawRefSet.acceleratorM),
				static_cast<JDx12FrameResource*>(drawRefSet.currFrame),
				&drawRefSet.info,
				&drawRefSet.option,
				useRaycastDevice);
		}
		static void EndContext(JDx12CommandContext* context)
		{
			context->End();
		}
	}
	namespace Context
	{
		std::string ToString(const J_MAIN_THREAD_ORDER order)
		{
			switch (order)
			{
			case JinEngine::Graphic::J_MAIN_THREAD_ORDER::BEGIN:
				return "Begin Step";
			case JinEngine::Graphic::J_MAIN_THREAD_ORDER::MID:
				return "Mid Step";
			case JinEngine::Graphic::J_MAIN_THREAD_ORDER::END:
				return "End Step";
			default:
				return "Unkwon";
			}
		}
		std::string ToString(const J_THREAD_TASK_TYPE threadTask, const uint threadIndex)
		{
			switch (threadTask)
			{
			case JinEngine::Graphic::J_THREAD_TASK_TYPE::OCC:
				return "Occlusion Step0" + std::to_string(threadIndex);
			case JinEngine::Graphic::J_THREAD_TASK_TYPE::SHADOW_MAP:
				return "Shadow Map Step0" + std::to_string(threadIndex);
			case JinEngine::Graphic::J_THREAD_TASK_TYPE::SCENE:
				return "Scene Step0" + std::to_string(threadIndex);
			default:
				return "Unkwon0" + std::to_string(threadIndex);
			}
		}
		class JDx12CommandContextCashData
		{
		public:
			JDx12CommandContext* publicContext = nullptr;
			JDx12CommandContext* stepContext[(uint)J_MAIN_THREAD_ORDER::COUNT];
			JDx12CommandContext* threadContext[(uint)J_THREAD_TASK_TYPE::COUNT * Graphic::Constants::gMaxFrameThread];
		public:
			void Initialize(JCommandContextManager* manager)
			{
				auto rawPtr = manager->PushCommandContext(std::make_unique<JDx12CommandContext>("Public", true));
				publicContext = static_cast<JDx12CommandContext*>(rawPtr);
				publicContext->SetWriteLogTrigger(false);

				for (uint i = 0; i < (uint)J_MAIN_THREAD_ORDER::COUNT; ++i)
				{
					const J_MAIN_THREAD_ORDER type = (J_MAIN_THREAD_ORDER)i;
					rawPtr = manager->PushCommandContext(std::make_unique<JDx12CommandContext>(ToString(type)));
					stepContext[i] = static_cast<JDx12CommandContext*>(rawPtr);
				}
				for (uint i = 0; i < (uint)J_THREAD_TASK_TYPE::COUNT; ++i)
				{
					for (uint j = 0; j < Constants::gMaxFrameThread; ++j)
					{
						const uint index = i * Graphic::Constants::gMaxFrameThread + j;
						const J_THREAD_TASK_TYPE type = (J_THREAD_TASK_TYPE)i;
						rawPtr = manager->PushCommandContext(std::make_unique<JDx12CommandContext>(ToString(type, j)));
						threadContext[index] = static_cast<JDx12CommandContext*>(rawPtr);
					}
				}
			}
		public:
			JDx12CommandContext* operator()()
			{
				return publicContext;
			}
			JDx12CommandContext* operator()(const J_MAIN_THREAD_ORDER order)
			{
				return stepContext[(uint)order];
			}
			JDx12CommandContext* operator()(const J_THREAD_TASK_TYPE threadTask, const uint threadIndex)
			{
				return threadContext[(uint)threadTask * Graphic::Constants::gMaxFrameThread + threadIndex];
			}
		public:
			void Clear()
			{
				publicContext = nullptr;
				for (uint i = 0; i < SIZE_OF_ARRAY(stepContext); ++i)
					stepContext[i] = nullptr;
				for (uint i = 0; i < SIZE_OF_ARRAY(threadContext); ++i)
					threadContext[i] = nullptr;
			}
		};
		static JDx12CommandContextCashData& CashData()
		{
			static JDx12CommandContextCashData cash;
			return cash;
		}
		static JDx12CommandContext* Get()
		{
			auto ptr = CashData()();
			return ptr->CanUse() ? ptr : nullptr;
		}
		static JDx12CommandContext* Get(const J_MAIN_THREAD_ORDER order)
		{
			auto ptr = CashData()(order);
			return ptr->CanUse() ? ptr : nullptr;
		}
		static JDx12CommandContext* Get(const J_THREAD_TASK_TYPE threadTask, const uint threadIndex)
		{
			auto ptr = CashData()(threadTask, threadIndex);
			return ptr->CanUse() ? ptr : nullptr;
		}
		static void Claer()
		{
			CashData().Clear();
		}
	}
	void JDx12Adaptee::Initialize(JCommandContextManager* manager)
	{
		Context::CashData().Initialize(manager);
	}
	void JDx12Adaptee::Clear()
	{
		Context::CashData().Clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12Adaptee::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	std::unique_ptr<JGraphicDevice> JDx12Adaptee::CreateDevice(const JGraphicSubClassShareData& shareData)
	{
		return std::make_unique<JDx12GraphicDevice>();
	}
	void JDx12Adaptee::CreateResourceManageSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JResourceManageSubclassSet& set)
	{
		set.graphic = std::make_unique<JDx12GraphicResourceManager>();
		set.culling = std::make_unique<JDx12CullingManager>();
		set.accelerator = std::make_unique<JDx12GpuAcceleratorManager>(shareData.frameIndexAccess);
		set.csm = std::make_unique<JCsmManager>();
		set.shareData = std::make_unique<JDx12GraphicResourceShareData>();
		set.context = std::make_unique<JCommandContextManager>();
		for (uint i = 0; i < Constants::gNumFrameResources; ++i)
			set.frame[i] = std::make_unique<JDx12FrameResource>();
	}
	void JDx12Adaptee::CreateDrawSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JDrawingSubclassSet& set)
	{
		set.scene = std::make_unique<JDx12SceneDraw>();
		set.shadowMap = std::make_unique<JDx12ShadowMap>();
		set.depthTest = std::make_unique<JDx12DepthTest>();
	}
	void JDx12Adaptee::CreateCullingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JCullingSubclassSet& set)
	{
		set.frustum = std::make_unique<JFrustumCulling>();
		set.hzb = std::make_unique<JDx12HZBOccCulling>();
		set.hd = std::make_unique<JDx12HardwareOccCulling>();
		set.lit = std::make_unique<JDx12LightCulling>();
	}
	void JDx12Adaptee::CreateImageProcessingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JImageProcessingSubclassSet& set)
	{
		set.debug = std::make_unique<JDx12GraphicDebug>();
		set.outline = std::make_unique<JDx12Outline>();
		set.blur = std::make_unique<JDx12Blur>();
		set.downSampling = std::make_unique<JDx12DownSampling>();
		set.ssao = std::make_unique<JDx12Ssao>();
		set.tm = std::make_unique<JDx12ToneMapping>();
		set.bloom = std::make_unique<JDx12Bloom>();
		set.aa = std::make_unique<JDx12Antialise>();
		set.histogram = std::make_unique<JDx12PostProcessHistogram>();
		set.exposure = std::make_unique<JDx12PostProcessExposure>();
		set.convertColor = std::make_unique<JDx12ConvertColor>();
		set.ppEffectSet = std::make_unique<JPostProcessEffectSet>(set.tm.get(), set.bloom.get(), set.aa.get(), set.histogram.get(), set.exposure.get(), set.convertColor.get());
		set.ppPipeline = std::make_unique<JPostProcessPipeline>();
	}
	void JDx12Adaptee::CreateRaytracingSubclass(const JGraphicSubClassShareData& shareData, _Inout_ JRaytracingSubclassSet& set)
	{
		set.gi = std::make_unique<JDx12RaytracingGI>();
		set.ao = std::make_unique<JDx12RaytracingAmbientOcclusion>(); 
		set.denoiser = std::make_unique<JDx12RaytracingDenoiser>(shareData.pushGraphicEventPtr);
	}
	std::unique_ptr<JGraphicInfoChangedSet> JDx12Adaptee::CreateInfoChangedSet(const JGraphicInfo& preInfo, const JGraphicDrawReferenceSet& drawRefSet)
	{
		return std::make_unique<JDx12GraphicInfoChangedSet>(preInfo, drawRefSet.info, drawRefSet.device, drawRefSet.graphicResourceM, drawRefSet.cullingM);
	}
	std::unique_ptr<JGraphicOptionChangedSet> JDx12Adaptee::CreateOptionChangedSet(const JGraphicOption& preOption, const JGraphicDrawReferenceSet& drawRefSet)
	{
		return std::make_unique<JDx12GraphicOptionChangedSet>(preOption, drawRefSet.option, drawRefSet.device, drawRefSet.graphicResourceM);
	}
	void JDx12Adaptee::BeginUpdateStart(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.device) || !IsSameDevice(drawRefSet.cullingM))
			return;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);

		dx12Cm->CopyCullingResultGpuToCpu(dx12Device, drawRefSet.info.frame.currIndex);
	}
	bool JDx12Adaptee::BeginDrawSceneSingleThread(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicDrawSceneSTSet& dataSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		JDx12GraphicResourceShareData* dx12Share = static_cast<JDx12GraphicResourceShareData*>(drawRefSet.shareData);

		dx12Frame->ResetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		dx12Frame->ResetCmd(J_MAIN_THREAD_ORDER::END);

		//begin => draw scnee
		//end => draw gui
		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		ID3D12DescriptorHeap* descriptorHeaps[] = { dx12Gm->GetDescriptorHeap(J_GRAPHIC_BIND_TYPE::SRV) };
		cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		cmdList->OMSetStencilRef(Constants::commonStencilRef);

		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::BEGIN);
		BeginContext(context, drawRefSet, cmdList, true);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(context);
		dataSet.sceneDraw = std::make_unique<JDx12GraphicSceneDrawSet>(context);
		dataSet.shadowMapDraw = std::make_unique<JDx12GraphicShadowMapDrawSet>(context, drawRefSet.blur);
		dataSet.occDraw = std::make_unique<JDx12GraphicOccDrawSet>(context, drawRefSet.depthTest);
		dataSet.hzbCompute = std::make_unique<JDx12GraphicHzbOccComputeSet>(context);
		dataSet.hdExtract = std::make_unique<JDx12GraphicHdOccExtractSet>(context);
		dataSet.debugCompute = std::make_unique<JDx12GraphicDebugRsComputeSet>(context);
		dataSet.outline = std::make_unique<JDx12GraphicOutlineDrawSet>(context);
		dataSet.ssao = std::make_unique<JDx12GraphicSsaoComputeSet>(context, dx12Share);
		dataSet.postPrcess = std::make_unique<JDx12PostProcessComputeSet>(drawRefSet.postSet, context, dx12Gm, dx12Share);
		dataSet.litCulling = std::make_unique<JDx12GraphicLightCullingTaskSet>(context);
		dataSet.litCullingDebug = std::make_unique<JDx12GraphicLightCullingDebugDrawSet>(context);
		dataSet.rtao = std::make_unique<JDx12GraphicRtAoComputeSet>(context);
		dataSet.rtgi = std::make_unique<JDx12GraphicRtGiComputeSet>(context, dx12Device); 
		dataSet.rtDenoiser = std::make_unique<JDx12GraphicRtDenoiseComputeSet>(context, dx12Device, dx12Gm, dx12Share);
		return true;
	}
	bool JDx12Adaptee::EndDrawSceneSingeThread(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::BEGIN);
		context->FlushResourceBarriers();

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		ThrowIfFailedG(cmdList->Close());
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		EndContext(context);
		return true;
	}
	bool JDx12Adaptee::SettingBeginFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBeginFrameSet& dataSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		dx12Frame->ResetCmd(drawRefSet.info.frame.threadCount);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::BEGIN);
		BeginContext(context, drawRefSet, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(context);
		dataSet.litCulling = std::make_unique<JDx12GraphicLightCullingTaskSet>(context);
		return true;
	}
	bool JDx12Adaptee::ExecuteBeginFrame(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::BEGIN);
		context->FlushResourceBarriers();

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::BEGIN);
		ThrowIfFailedG(cmdList->Close());
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		EndContext(context);
		return true;
	}
	bool JDx12Adaptee::SettingMidFrame(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMidFrameSet& dataSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		JDx12GraphicResourceShareData* dx12Share = static_cast<JDx12GraphicResourceShareData*>(drawRefSet.shareData);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::MID);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::MID);
		BeginContext(context, drawRefSet, cmdList, true);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(context);
		dataSet.sceneDraw = std::make_unique<JDx12GraphicSceneDrawSet>(context);
		dataSet.hzbCompute = std::make_unique<JDx12GraphicHzbOccComputeSet>(context);
		dataSet.hdExtract = std::make_unique<JDx12GraphicHdOccExtractSet>(context);
		dataSet.debugCompute = std::make_unique<JDx12GraphicDebugRsComputeSet>(context);
		dataSet.outline = std::make_unique<JDx12GraphicOutlineDrawSet>(context);
		dataSet.ssao = std::make_unique<JDx12GraphicSsaoComputeSet>(context, dx12Share);
		dataSet.postPrcess = std::make_unique<JDx12PostProcessComputeSet>(drawRefSet.postSet, context, dx12Gm, dx12Share);
		dataSet.litCullingDebug = std::make_unique<JDx12GraphicLightCullingDebugDrawSet>(context);
		dataSet.rtao = std::make_unique<JDx12GraphicRtAoComputeSet>(context);
		dataSet.rtgi = std::make_unique<JDx12GraphicRtGiComputeSet>(context, dx12Device); 
		dataSet.rtDenoiser = std::make_unique<JDx12GraphicRtDenoiseComputeSet>(context, dx12Device, dx12Gm, dx12Share);
		return true;
	}
	bool JDx12Adaptee::ExecuteMidFrame(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::MID);
		context->FlushResourceBarriers();

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::MID);
		ThrowIfFailedG(cmdList->Close());
		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		EndContext(context);
		return true;
	}
	bool JDx12Adaptee::SettingEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		//J_MAIN_THREAD_ORDER::END reset 안됬을시 처리필요  
		if (!cond.isSceneDrawn)
			dx12Frame->ResetCmd(J_MAIN_THREAD_ORDER::END);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::END);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::END);
		BeginContext(context, drawRefSet, cmdList);

		auto swapChainSet = JDx12GraphicResourceComputeSet(dx12Gm, dx12Gm->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, dx12Device->GetBackBufferIndex()));
		auto mainDepthSet = JDx12GraphicResourceComputeSet(dx12Gm, dx12Gm->GetDefaultSceneDsInfo());

		context->Transition(swapChainSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		//context->Transition(mainDepthSet.holder, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChainSet.GetCpuRtvHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE dsv = mainDepthSet.GetCpuDsvHandle();

		cmdList->ClearRenderTargetView(rtv, Constants::GetBackBufferClearColor(), 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, true, nullptr);
		return true;
	}
	bool JDx12Adaptee::ExecuteEndFrame(const JGraphicDrawReferenceSet& drawRefSet, const JGraphicEndConditonSet cond)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(drawRefSet.cullingM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_MAIN_THREAD_ORDER::END);
		JDx12CommandContext* context = Context::Get(J_MAIN_THREAD_ORDER::END);

		auto swapChainSet = JDx12GraphicResourceComputeSet(dx12Gm, dx12Gm->GetInfo(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN, dx12Device->GetBackBufferIndex()));
		auto mainDepthSet = JDx12GraphicResourceComputeSet(dx12Gm, dx12Gm->GetDefaultSceneDsInfo());

		//context->Transition(mainDepthSet.holder, D3D12_RESOURCE_STATE_DEPTH_READ);
		context->Transition(swapChainSet.holder, D3D12_RESOURCE_STATE_PRESENT, true);
		ThrowIfFailedG(cmdList->Close());

		ID3D12CommandList* cmdsLists[] = { cmdList };
		dx12Device->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		dx12Device->Present();

		const int nextBackBuffer = dx12Device->GetBackBufferIndex() + 1;
		//현재 태스크에 fence값
		//작업이 없을때는 fence 0
		//작업이 생겼을 경우 fence값을 증가시키므로
		//1부터 유효한 fence 값이다
		const int nextFence = dx12Device->GetFenceValue() + 1;
		const int backBufferIndex = nextBackBuffer % dx12Gm->GetResourceCapacity(J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN);

		dx12Device->SetBackBufferIndex(backBufferIndex);
		dx12Device->SetFenceValue(nextFence);
		dx12Frame->SetFenceValue(nextFence);
		dx12Device->Signal();
		dx12Cm->UpdateFrameResourceIndex(drawRefSet.nextFrameIndex);

		EndContext(Context::Get(J_MAIN_THREAD_ORDER::END));
		return true;
	}
	bool JDx12Adaptee::SettingDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadOccTaskSet& dataSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_THREAD_TASK_TYPE::OCC, threadIndex);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		JDx12CommandContext* context = Context::Get(J_THREAD_TASK_TYPE::OCC, threadIndex);
		BeginContext(context, drawRefSet, cmdList);

		dataSet.occDraw = std::make_unique<JDx12GraphicOccDrawSet>(context, drawRefSet.depthTest);
		return true;
	}
	bool JDx12Adaptee::NotifyCompleteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!drawRefSet.IsValid() || !IsSameDevice(drawRefSet.currFrame))
			return false;

		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ThrowIfFailedG(dx12Frame->GetCmd(J_THREAD_TASK_TYPE::OCC, threadIndex)->Close());
		SetEvent(dx12Frame->GetHandle(J_THREAD_TASK_TYPE::OCC, threadIndex));

		JDx12CommandContext* context = Context::Get(J_THREAD_TASK_TYPE::OCC, threadIndex);
		context->FlushResourceBarriers();
		EndContext(context);
		return true;
	}
	bool JDx12Adaptee::SettingDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadShadowMapTaskSet& dataSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		JDx12CommandContext* context = Context::Get(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex);
		BeginContext(context, drawRefSet, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(context);
		dataSet.shadowMapDraw = std::make_unique<JDx12GraphicShadowMapDrawSet>(context, drawRefSet.blur);
		return true;
	}
	bool JDx12Adaptee::NotifyCompleteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ThrowIfFailedG(dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex)->Close());
		SetEvent(dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex));

		JDx12CommandContext* context = Context::Get(J_THREAD_TASK_TYPE::SHADOW_MAP, threadIndex);
		context->FlushResourceBarriers();
		EndContext(context);
		return true;
	}
	bool JDx12Adaptee::SettingDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex, _Inout_ JGraphicThreadSceneTaskSet& dataSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);

		ID3D12GraphicsCommandList* cmdList = dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SCENE, threadIndex);
		SettingDescriptorHeaps(dx12Gm, cmdList);

		JDx12CommandContext* context = Context::Get(J_THREAD_TASK_TYPE::SCENE, threadIndex);
		BeginContext(context, drawRefSet, cmdList);

		dataSet.bind = std::make_unique<JDx12GraphicBindSet>(context);
		dataSet.occDraw = std::make_unique<JDx12GraphicOccDrawSet>(context, drawRefSet.depthTest);
		dataSet.sceneDraw = std::make_unique<JDx12GraphicSceneDrawSet>(context);
		return true;
	}
	bool JDx12Adaptee::NotifyCompleteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet, const uint threadIndex)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ThrowIfFailedG(dx12Frame->GetCmd(J_THREAD_TASK_TYPE::SCENE, threadIndex)->Close());
		SetEvent(dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SCENE, threadIndex));

		JDx12CommandContext* context = Context::Get(J_THREAD_TASK_TYPE::SCENE, threadIndex);
		context->FlushResourceBarriers();
		EndContext(context);
		return true;
	}
	bool JDx12Adaptee::ExecuteDrawOccTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12CommandQueue* commandQueue = dx12Device->GetCommandQueue();

		WaitForMultipleObjects(drawRefSet.info.frame.threadCount, dx12Frame->GetHandle(J_THREAD_TASK_TYPE::OCC), true, INFINITE);
		commandQueue->ExecuteCommandLists(drawRefSet.info.frame.threadCount, dx12Frame->GetBatchCmd(J_THREAD_TASK_TYPE::OCC));
		return true;
	}
	bool JDx12Adaptee::ExecuteDrawShadowMapTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12CommandQueue* commandQueue = dx12Device->GetCommandQueue();

		WaitForMultipleObjects(drawRefSet.info.frame.threadCount, dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SHADOW_MAP), true, INFINITE);
		commandQueue->ExecuteCommandLists(drawRefSet.info.frame.threadCount, dx12Frame->GetBatchCmd(J_THREAD_TASK_TYPE::SHADOW_MAP));
		return true;
	}
	bool JDx12Adaptee::ExecuteDrawSceneTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(drawRefSet.currFrame);
		ID3D12CommandQueue* commandQueue = dx12Device->GetCommandQueue();

		WaitForMultipleObjects(drawRefSet.info.frame.threadCount, dx12Frame->GetHandle(J_THREAD_TASK_TYPE::SCENE), true, INFINITE);
		commandQueue->ExecuteCommandLists(drawRefSet.info.frame.threadCount, dx12Frame->GetBatchCmd(J_THREAD_TASK_TYPE::SCENE));
		return true;
	}
	bool JDx12Adaptee::BeginBlurTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicBlurTaskSettingSet& set)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12Device->GetPublicCmdList();

		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
		if (set.useDataHandle)
		{
			srcHandle = dx12Gm->GetMPBGpuDescriptorHandle(*set.fromDH, J_GRAPHIC_BIND_TYPE::SRV);		//0.. n-1
			destHandle = dx12Gm->GetMPBGpuDescriptorHandle(*set.toDH, J_GRAPHIC_BIND_TYPE::UAV);			//1.. n
		}
		else
		{
			srcHandle.ptr = (UINT64)set.fromRH;
			destHandle.ptr = (UINT64)set.toRH;
		}
		if (srcHandle.ptr == NULL || destHandle.ptr == NULL)
			return false;

		JDx12CommandContext* context = Context::Get();
		BeginContext(context, drawRefSet, cmdList);

		auto dataSet = std::make_unique<JDx12GraphicBlurComputeSet>(context,
			std::move(set.desc),
			srcHandle,
			destHandle);

		if (set.useDataHandle)
		{
			dataSet->srcInfo = context->ComputeSet(dx12Gm->GetMPBInfo(*set.fromDH)).info;
			dataSet->destInfo = context->ComputeSet(dx12Gm->GetMPBInfo(*set.toDH)).info;
		}
		set.dataSet = std::move(dataSet);
		return true;
	}
	void JDx12Adaptee::EndBlurTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		JDx12CommandContext* context = Context::Get();
		context->FlushResourceBarriers();
		EndContext(context);
	}
	bool JDx12Adaptee::BeginMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicMipmapGenerationSettingSet& set)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12Device->GetPublicCmdList();

		CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle = dx12Gm->GetMPBGpuDescriptorHandle(set.mipMapHandle[0], J_GRAPHIC_BIND_TYPE::SRV);		//0.. n-1
		CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle = dx12Gm->GetMPBGpuDescriptorHandle(set.mipMapHandle[1], J_GRAPHIC_BIND_TYPE::UAV);	//1.. n
		if (srcHandle.ptr == NULL || destHandle.ptr == NULL)
			return false;

		JDx12CommandContext* context = Context::Get();
		BeginContext(context, drawRefSet, cmdList);

		SettingDescriptorHeaps(dx12Gm, cmdList);
		auto dataSet = std::make_unique<JDx12GraphicDownSampleComputeSet>(context, drawRefSet.blur, std::move(set.desc), set.mipMapHandle, srcHandle, destHandle);

		dataSet->info = context->ComputeSet(dx12Gm->GetMPBInfo(set.mipMapHandle[0])).info;
		set.dataSet = std::move(dataSet);
		return true;
	}
	void JDx12Adaptee::EndMipmapGenerationTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		JDx12CommandContext* context = Context::Get();
		context->FlushResourceBarriers();
		EndContext(context);
	}
	bool JDx12Adaptee::BeginConvertColorTask(const JGraphicDrawReferenceSet& drawRefSet, _Inout_ JGraphicConvetColorSettingSet& set)
	{
		if (!IsValid(drawRefSet))
			return false;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(drawRefSet.device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(drawRefSet.graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12Device->GetPublicCmdList();
		 
		if (set.from == nullptr || set.to == nullptr)
			return false;
		 
		JDx12CommandContext* context = Context::Get();
		BeginContext(context, drawRefSet, cmdList);

		SettingDescriptorHeaps(dx12Gm, cmdList); 
		set.dataSet = std::make_unique<JDx12GraphicConvertColorComputeSet>(context, set.desc, set.from, set.to);
		return true;
	}
	void JDx12Adaptee::EndConvertColorTask(const JGraphicDrawReferenceSet& drawRefSet)
	{
		JDx12CommandContext* context = Context::Get();
		context->FlushResourceBarriers();
		EndContext(context);
	}
}