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


#include"JDx12RaytracingAmbientOcclusion.h" 
#include"../../Dx/JDx12RaytracingUtility.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"   
#include"../../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../../Command/Dx/JDx12CommandContext.h"
#include"../../../Utility/Dx/JDx12ObjectCreation.h" 
#include"../../../FrameResource/Dx/JDx12FrameResource.h" 
#include"../../../JGraphicUpdateHelper.h" 
#include"../../../../Core/Platform/JHardwareInfo.h"
#include"../../../../Core/Math/JVectorExtend.h" 
#include"../../../../Object/Component/Camera/JCamera.h"   
#include"../../../../Object/Resource/Scene/JScene.h"    
#include<random>

#ifdef DEVELOP
#include"../../../../Develop/Debug/JDevelopDebug.h"
#endif 

#define USE_DEBUG_SYMBOL L"USE_DEBUG"
namespace JinEngine::Graphic
{
	struct RayPayload
	{
		JVector4F color;
		uint   recursionDepth;
	};
	struct ShadowRayPayload
	{
		float isVisible;
		uint index;
	};
	struct ProceduralPrimitiveAttributes
	{
		JVector3F normal;
	};
	namespace Private
	{
#ifdef _DEBUG
		static constexpr bool allowDebug = false;
		static constexpr uint bufferSize = 32;
#else
		static constexpr bool allowDebug = false;
		static constexpr uint bufferSize = 32;
#endif

		static constexpr uint ssaoCBIndex = 0;
		static constexpr uint sampleCBIndex = ssaoCBIndex + 1;
		static constexpr uint rtCBIndex = sampleCBIndex + 1;
		static constexpr uint asViewindex = rtCBIndex + 1;
		static constexpr uint depthMapIndex = asViewindex + 1;
		static constexpr uint normalMapIndex = depthMapIndex + 1;
		static constexpr uint randomMapIndex = normalMapIndex + 1;
		static constexpr uint aoMapIndex = randomMapIndex + 1;
		static constexpr uint debugIndex = aoMapIndex + 1;
		static constexpr uint glSlotCount = allowDebug ? debugIndex + 1 : aoMapIndex + 1;

		static constexpr uint rtCB32BitCount = sizeof(JMatrix4x4) / 4;
		//static constexpr uint loPassCBIndex = glSlotCount;
		//static constexpr uint loSlotCount = loPassCBIndex + aoMapIndex + 1;

		static constexpr uint threadDimX = 16;
		static constexpr uint threadDimY = 16;

		static constexpr uint sampleCount[(uint)J_SSAO_SAMPLE_TYPE::COUNT]
		{
			 Constants::ssaoMaxSampleCount / 8,
			 Constants::ssaoMaxSampleCount / 4,
			 Constants::ssaoMaxSampleCount / 2,
			 Constants::ssaoMaxSampleCount
		};
		static constexpr uint randomWidth = 4;

		const wchar_t* hitGroupName = L"HitGroup";
		const wchar_t* closestShaderName = L"ClosestHitShader";
		const wchar_t* anyHitShaderName = L"AnyHitShader";
		const wchar_t* raygenShaderName = L"RayGenShader";
		const wchar_t* missShaderName = L"MissShader";
	} 

	JDx12RaytracingAmbientOcclusion::~JDx12RaytracingAmbientOcclusion()
	{
		ClearResource();
	}
	void JDx12RaytracingAmbientOcclusion::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{ 
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		BuildResource(device, gM);
	}
	void JDx12RaytracingAmbientOcclusion::Clear()
	{ 
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12RaytracingAmbientOcclusion::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	void JDx12RaytracingAmbientOcclusion::StreamOutDebugInfo()
	{
		if constexpr (!Private::allowDebug)
			return;

		static int trigger = 0;
		if (!Develop::JDevelopDebug::IsActivate())
		{
			trigger = 1;
			return;
		}
		else if (trigger <= 0)
			return;
		--trigger;

		int count = 0;
		JRaytracingDebugConstants<debuggingSampleCount>* constants = debugHandle->Map(count);
		 
		Develop::JDevelopDebug::CreatePublicLogHandler("RaytracingDebug");
		Develop::JDevelopDebug::PushDefaultLogHandler("RaytracingDebug");
		Develop::JDevelopDebug::PushLog("bufferSize: " + std::to_string(Private::bufferSize));
		for (uint i = 0; i < Private::bufferSize; ++i)
		{
			JRaytracingDebugConstants<debuggingSampleCount>& data = constants[i];
			float ao = 0;
			Develop::JDevelopDebug::PushLog("Index: " + std::to_string(i));
			Develop::JDevelopDebug::PushLog("Pixel: " + data.index.ToString());
			for (uint j = 0; j < debuggingSampleCount; ++j)
			{
				Develop::JDevelopDebug::PushLog("Sample: " + std::to_string(j));
				Develop::JDevelopDebug::PushLog("RayOrigin: " + data.rayOriginAndMinT[j].XYZ().ToString());
				Develop::JDevelopDebug::PushLog("RayDir: " + data.rayDirAndMaxT[j].XYZ().ToString());
				Develop::JDevelopDebug::PushLog("MinT: " + std::to_string(data.rayOriginAndMinT[j].w));
				Develop::JDevelopDebug::PushLog("MaxT: " + std::to_string(data.rayDirAndMaxT[j].w));

				Develop::JDevelopDebug::PushLog("Intersect: " + data.intersectPosition[j].XYZ().ToString());
				Develop::JDevelopDebug::PushLog("IsHit: " + std::to_string(data.intersectPosition[j].w));
				Develop::JDevelopDebug::PushLog("\n");
				ao += data.intersectPosition[j].w;
			}
			Develop::JDevelopDebug::PushLog("Ao: " + std::to_string(1.0f - (ao / (float)debuggingSampleCount)));
			Develop::JDevelopDebug::PushLog("\n");
		}
		Develop::JDevelopDebug::Write();
		Develop::JDevelopDebug::PopDefaultLogHandler("RaytracingDebug");
	}
	void JDx12RaytracingAmbientOcclusion::ComputeAmbientOcclusion(const JGraphicRtAoComputeSet* computeSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(computeSet) || helper.cam == nullptr || !helper.allowSsao)
			return;

		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(computeSet->context); 
		
		auto gInterface = helper.cam->GraphicResourceUserInterface();
		auto aInterface = helper.scene->GpuAcceleratorUserInterface();
		auto ssaoDesc = helper.cam->GetSsaoDesc();

		auto rtSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		auto dsSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
		auto normalSet = context->ComputeSet(rtSet.info, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
		auto randomSet = context->ComputeSet(randomVecInfo);
		auto aoSet = context->ComputeSet(gInterface, J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);
		auto accelSet = context->ComputeSet(aInterface);

		const DirectX::XMMATRIX viewM = helper.cam->GetView();
		JMatrix4x4 camInvView;
		camInvView.StoreXM(XMMatrixTranspose(XMMatrixInverse(nullptr, viewM)));

		if constexpr (Private::allowDebug)
			context->Transition(debugHandle->GetUABufferInterface()->GetHolder(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context->Transition(dsSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(normalSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context->Transition(randomSet.holder, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE); 
		context->Transition(aoSet.holder, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		context->SetComputeRootSignature(globalRootSignature.Get());
		context->SetComputeRootConstantBufferView(Private::ssaoCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SSAO_PASS, helper.GetCamFrameIndex(CameraFrameLayer::ssao));
		context->SetComputeRootConstantBufferView(Private::sampleCBIndex, sampleCB.get(), 3);
		context->SetComputeRoot32BitConstants(Private::rtCBIndex, 2, camInvView);
		context->SetTlasView(Private::asViewindex, accelSet);
		context->SetComputeRootDescriptorTable(Private::depthMapIndex, dsSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Private::normalMapIndex, normalSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Private::randomMapIndex, randomSet.GetGpuSrvHandle());
		context->SetComputeRootDescriptorTable(Private::aoMapIndex, aoSet.GetGpuUavHandle());
		if constexpr (Private::allowDebug)
			context->SetComputeRootUnorderedAccessView(Private::debugIndex, debugHandle->GetUABufferInterface());
		DispatchRays(context, rayGenShaderTable->GetResource(), aoSet.info->GetWidth(), aoSet.info->GetHeight());
		context->InsertUAVBarrier(aoSet.holder);
		if constexpr(Private::allowDebug)
			context->CopyResource(debugHandle.get());
	} 
	void JDx12RaytracingAmbientOcclusion::DispatchRays(JDx12CommandContext* context, ID3D12Resource* rayGenShaderTable, uint width, uint height)
	{		 
		D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
		dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable->GetResource()->GetGPUVirtualAddress();
		dispatchDesc.HitGroupTable.SizeInBytes = hitGroupShaderTable->GetResource()->GetDesc().Width;
		dispatchDesc.HitGroupTable.StrideInBytes = hitGroupShaderTable->GetElementByteSize();
		dispatchDesc.MissShaderTable.StartAddress = missShaderTable->GetResource()->GetGPUVirtualAddress();
		dispatchDesc.MissShaderTable.SizeInBytes = missShaderTable->GetResource()->GetDesc().Width;
		dispatchDesc.MissShaderTable.StrideInBytes = missShaderTable->GetElementByteSize();
		dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable->GetGPUVirtualAddress();
		dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable->GetDesc().Width;
		dispatchDesc.Width = width;
		dispatchDesc.Height = height;
		dispatchDesc.Depth = 1;
		context->SetPipelineState(stateObject.Get()); 
		context->DispatchRays(&dispatchDesc);
	}
	void JDx12RaytracingAmbientOcclusion::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gM);

		BuildRootSignature(dx12Device);
		BuildShaderResource(dx12Device);
		BuildBuffer(device, dx12Gm);
	}
	void JDx12RaytracingAmbientOcclusion::BuildRootSignature(JDx12GraphicDevice* device)
	{
		BuildRtAoRootSignature(device->GetRaytracingDevice());
	}
	void JDx12RaytracingAmbientOcclusion::BuildShaderResource(JDx12GraphicDevice* device)
	{
		const JGraphicInfo& gInfo = GetGraphicInfo();
		const JGraphicOption& gOption = GetGraphicOption();
		BuildRtAoStateObject(device->GetRaytracingDevice(), gInfo);
		BuildShaderTables(device->GetRaytracingDevice(), gInfo, gOption);
	}
	void JDx12RaytracingAmbientOcclusion::BuildRtAoRootSignature(ID3D12Device5* device)
	{
		JDx12RootSignatureBuilder2<Private::glSlotCount, 2> globalBuilder;
		globalBuilder.PushConstantsBuffer(0);
		globalBuilder.PushConstantsBuffer(1);
		globalBuilder.PushConstants(Private::rtCB32BitCount, 2);
		globalBuilder.PushShaderResource(0);
		globalBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		globalBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		globalBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		globalBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		if constexpr(Private::allowDebug)
			globalBuilder.PushUnorderedAccess(1);
		std::vector< CD3DX12_STATIC_SAMPLER_DESC> sam
		{
			CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

			CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP) // addressW 
		};
		for (const auto& data : sam)
			globalBuilder.PushSampler(data);
		globalBuilder.Create(device, L"glAoRootSignature", globalRootSignature.GetAddressOf());
		
		/*
		JDx12RootSignatureBuilder2<Private::loSlotCount, 2> localBuilder;
		localBuilder.PushConstantsBuffer(0);
		localBuilder.PushConstantsBuffer(1);
		localBuilder.PushShaderResource(0);
		localBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		localBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		localBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
		localBuilder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		for (const auto& data : sam)
			localBuilder.PushSampler(data);
		localBuilder.Create(device, L"loAoRootSignature", localRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		*/
	}
	// Create a raytracing pipeline state object (RTPSO).
	// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
	// with all configuration options resolved, such as local root signatures and other state.
	void JDx12RaytracingAmbientOcclusion::BuildRtAoStateObject(ID3D12Device5* device, const JGraphicInfo& info)
	{
		JStateObjectBuildData buildData; 
		// DXIL library
		BuildDxilLibrarySubobject(buildData);

		// Hit groups
		BuildHitGroupSubobjects(buildData);

		//Shader config
		//Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
		BuildShaderConfig(buildData);

		// Local root signature and shader association
		//This is a root signature that enables a shader to have unique arguments that come from shader tables.
		//BuildLocalRootSignatureSubobjects(buildData);
	 
		// Global root signature
		// This is a root signature that is shared across all RTAO shaders invoked during a DispatchRays() call.
		BuildGlobalRootSignatureSubobjects(buildData);

        // Pipeline config
        // Defines the maximum TraceRay() recursion depth.
		BuildPipelineConfig(buildData);

		if constexpr(Private::allowDebug)
			JDx12RtUtil::PrintStateObjectDesc(buildData.raytracingPipeline);
		// Create the state object.
		ThrowIfFailedG(device->CreateStateObject(buildData.raytracingPipeline, IID_PPV_ARGS(&stateObject)));
	}
	void JDx12RaytracingAmbientOcclusion::BuildDxilLibrarySubobject(JStateObjectBuildData& buildData)
	{
		D3D12_DXIL_LIBRARY_DESC desc;

		JComputeShaderInitData initData; 
		if (Private::allowDebug)
			initData.macro.push_back(JMacroSet{ USE_DEBUG_SYMBOL, L"1" });
		buildData.shader = JDxShaderDataUtil::CompileShader(JCompileInfo(L"RaytracingTest.hlsl", L"RayGenShader"), initData.macro,  L"lib_6_3");
		 
		CD3DX12_SHADER_BYTECODE libdxil;
		libdxil.pShaderBytecode = buildData.shader->GetBufferPointer();
		libdxil.BytecodeLength = buildData.shader->GetBufferSize();

		auto lib = buildData.raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		lib->SetDXILLibrary(&libdxil);
		lib->DefineExport(Private::raygenShaderName);
		lib->DefineExport(Private::closestShaderName);
		lib->DefineExport(Private::missShaderName);
		lib->DefineExport(Private::anyHitShaderName);
		// Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
	}
	void JDx12RaytracingAmbientOcclusion::BuildHitGroupSubobjects(JStateObjectBuildData& buildData)
	{ 
		auto hitGroup = buildData.raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetClosestHitShaderImport(Private::closestShaderName);
		hitGroup->SetAnyHitShaderImport(Private::anyHitShaderName);
		hitGroup->SetHitGroupExport(Private::hitGroupName);
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES); 
	}
	void JDx12RaytracingAmbientOcclusion::BuildShaderConfig(JStateObjectBuildData& buildData)
	{
		auto shaderConfig = buildData.raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		UINT payloadSize = sizeof(ShadowRayPayload);
		UINT attributeSize = sizeof(struct ProceduralPrimitiveAttributes); 
		shaderConfig->Config(payloadSize, attributeSize);
	}
	void JDx12RaytracingAmbientOcclusion::BuildLocalRootSignatureSubobjects(JStateObjectBuildData& buildData)
	{
		auto loRootSub = buildData.raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		loRootSub->SetRootSignature(localRootSignature.Get());
		// Shader association
		auto rootSignatureAssociation = buildData.raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*loRootSub);
		rootSignatureAssociation->AddExport(Private::hitGroupName);
	}
	void JDx12RaytracingAmbientOcclusion::BuildGlobalRootSignatureSubobjects(JStateObjectBuildData& buildData)
	{
		auto glRootSub = buildData.raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		glRootSub->SetRootSignature(globalRootSignature.Get());
	}
	void JDx12RaytracingAmbientOcclusion::BuildPipelineConfig(JStateObjectBuildData& buildData)
	{
		auto pipelineConfig = buildData.raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		// PERFOMANCE TIP: Set max recursion depth as low as needed
		// as drivers may apply optimization strategies for low recursion depths.
		UINT maxRecursionDepth = 1;
		pipelineConfig->Config(maxRecursionDepth);
	}
	// Build shader tables.
	// This encapsulates all shader records - shaders and the arguments for their local root signatures.
	// For AO, the shaders are simple with only one shader type per shader table.
	void JDx12RaytracingAmbientOcclusion::BuildShaderTables(ID3D12Device5* device, const JGraphicInfo& info, const JGraphicOption& option)
	{
		void* rayGenShaderIDs;
		void* missShaderID;
		void* hitGroupShaderID;

		// A shader name look-up table for shader table debug print out.
		std::unordered_map<void*, std::wstring> shaderIdToStringMap;

		auto GetShaderIDsLam = [&](auto* stateObjectProperties)
		{
			rayGenShaderIDs = stateObjectProperties->GetShaderIdentifier(Private::raygenShaderName);
			shaderIdToStringMap[rayGenShaderIDs] = Private::raygenShaderName;

			missShaderID = stateObjectProperties->GetShaderIdentifier(Private::missShaderName);
			shaderIdToStringMap[missShaderID] = Private::missShaderName;

			hitGroupShaderID = stateObjectProperties->GetShaderIdentifier(Private::hitGroupName);
			shaderIdToStringMap[hitGroupShaderID] = Private::hitGroupName;
		};

		// Get shader identifiers.
		uint shaderIDSize;
		Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
		ThrowIfFailedG(stateObject.As(&stateObjectProperties));

		GetShaderIDsLam(stateObjectProperties.Get());
		shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

		// RayGen shader tables.
		{
			UINT numShaderRecords = 1;
			UINT shaderRecordSize = shaderIDSize; // No root arguments

			std::unique_ptr<JShaderTable> sRayGenShaderTable = std::make_unique<JShaderTable>(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
			sRayGenShaderTable->PushBack(JShaderRecord(rayGenShaderIDs, shaderIDSize, nullptr, 0));
			sRayGenShaderTable->DebugPrint(shaderIdToStringMap);
			rayGenShaderTable = std::move(sRayGenShaderTable);
		}

		// Miss shader table.
		{
			UINT numShaderRecords = 1;
			UINT shaderRecordSize = shaderIDSize; // No root arguments

			std::unique_ptr<JShaderTable> sMissShaderTable = std::make_unique<JShaderTable>(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
			sMissShaderTable->PushBack(JShaderRecord(missShaderID, shaderIDSize, nullptr, 0));
			sMissShaderTable->DebugPrint(shaderIdToStringMap);
			missShaderTable = std::move(sMissShaderTable);
		}


		// maxInstanceContributionToHitGroupIndex - since BLAS instances in this sample specify non-zero InstanceContributionToHitGroupIndex for Pathtracer raytracing pass, 
		//  the AO raytracing needs to add as many shader records to all hit group shader tables so that DXR shader addressing lands on a valid shader record for all BLASes.
		UINT maxInstanceContributionToHitGroupIndex = 0;

		// Hit group shader table.
		{
			// Duplicate the shader records because the TLAS has BLAS instances with non-zero InstanceContributionToHitGroupIndex.
			// For the last offset we need only one more shader record, because RTAO TraceRay always indexes the first shader record
			// of each BLAS instance shader record range due to RTAOTraceRayParameters::HitGroup::GeometryStride of 0.
			UINT numShaderRecords = maxInstanceContributionToHitGroupIndex + 1;
			UINT shaderRecordSize = shaderIDSize; // No root arguments

			std::unique_ptr<JShaderTable> sHitGroupShaderTable = std::make_unique<JShaderTable>(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
			sHitGroupShaderTable->PushBack(JShaderRecord(hitGroupShaderID, shaderIDSize, nullptr, 0));
			sHitGroupShaderTable->DebugPrint(shaderIdToStringMap);
			hitGroupShaderTable = std::move(sHitGroupShaderTable);
		}
	}
	void JDx12RaytracingAmbientOcclusion::BuildBuffer(JGraphicDevice* device, JDx12GraphicResourceManager* gm)
	{
		std::random_device rd;	//use hardware
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> disSNorm(-1.0f, 1.0f);
		std::uniform_real_distribution<float> disUNorm(0.0f, 1.0f);

		if (sampleCB == nullptr)
			sampleCB = std::make_unique<JDx12GraphicBufferT<SsaoSampleConstants>>(L"Sample", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);

		sampleCB->Clear();
		sampleCB->Build(device, (uint)J_SSAO_SAMPLE_TYPE::COUNT);

		for (uint i = 0; i < (uint)J_SSAO_SAMPLE_TYPE::COUNT; ++i)
		{
			SsaoSampleConstants constants;
			for (uint j = 0; j < Private::sampleCount[i]; ++j)
			{
				float scale = j / (float)Private::sampleCount[i];
				scale = JMathHelper::Lerp(0.1f, 1.0f, scale * scale);
				JVector4F sample(disSNorm(gen), disSNorm(gen), disUNorm(gen), 1.0f);
				sample = sample.Normalize();
				sample *= disUNorm(gen);
				sample *= scale;
				constants.sample[j] = sample;
			}
			sampleCB->CopyData(i, constants);
		}

		//ssao
		uint randomCount = Private::randomWidth * Private::randomWidth;
		std::vector<JVector4F> ssaoRandomVec(randomCount);
		for (uint i = 0; i < randomCount; ++i)
		{
			JVector4F noise(disSNorm(gen), disSNorm(gen), 0.0f, 1.0f);
			ssaoRandomVec[i] = noise.Normalize();
		}

		JGraphicResourceCreationDesc desc;
		desc.width = Private::randomWidth;
		desc.height = Private::randomWidth;
		desc.textureDesc = std::make_unique<JTextureCreationDesc>();
		desc.textureDesc->mipMapDesc.type = J_GRAPHIC_MIP_MAP_TYPE::NONE; 
		desc.uploadBufferDesc = std::make_unique<JUploadBufferCreationDesc>(ssaoRandomVec.data(), ssaoRandomVec.size() * sizeof(JVector4F));

		randomVecInfo = gm->CreateResource(device, desc, J_GRAPHIC_RESOURCE_TYPE::TEXTURE_COMMON);

		if constexpr (Private::allowDebug)
		{
			if (debugHandle == nullptr)
				debugHandle = std::make_unique<HlslDebug>(L"RaytracingDebug", Private::debugIndex);
			debugHandle->Build(device, Private::bufferSize);
		}
	}
	void JDx12RaytracingAmbientOcclusion::ClearResource()
	{
		ClearRootSignature();
		ClearStateObject();
		ClearShaderTable();
		ClearBuffer();
	}
	void JDx12RaytracingAmbientOcclusion::ClearRootSignature()
	{ 
		globalRootSignature = nullptr;
		localRootSignature = nullptr; 
	}
	void JDx12RaytracingAmbientOcclusion::ClearStateObject()
	{
		stateObject = nullptr;
	}
	void JDx12RaytracingAmbientOcclusion::ClearShaderTable()
	{
		rayGenShaderTable = nullptr;
		hitGroupShaderTable = nullptr;
		missShaderTable = nullptr;
	}
	void JDx12RaytracingAmbientOcclusion::ClearBuffer()
	{
		sampleCB = nullptr;
		randomVecInfo = nullptr;
		if constexpr (Private::allowDebug)
			debugHandle = nullptr;
	}
}