#include"JDx12GraphicDebug.h" 
#include"../../JGraphicInfo.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../Utility/Dx/JD3DUtility.h" 
#include"../../../Application/JApplicationEngine.h" 
#include"../../../Core/Exception/JExceptionMacro.h"  
#include"../../../Core/Platform/JHardwareInfo.h"  
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Component/Light/JLight.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<DirectXColors.h>

#define THREAD_DIM_X_SYMBOL L"DIMX"
#define THREAD_DIM_Y_SYMBOL L"DIMY"
#define THREAD_DIM_Z_SYMBOL L"DIMZ"
#define USE_PERSPECTIVE L"USE_PERSPECTIVE"

using namespace DirectX;
using namespace Microsoft::WRL;

namespace JinEngine::Graphic
{
	namespace
	{
		static constexpr uint srcTextureHandleIndex = 0;
		static constexpr uint destTextureHandleIndex = srcTextureHandleIndex + 1;
		static constexpr uint settingCbIndex = destTextureHandleIndex + 1;
		static constexpr uint slotCount = settingCbIndex + 1;

		static void StuffComputeShaderMacro(_Inout_ JComputeShaderInitData& initHelper, const bool isPerspective)
		{
			initHelper.macro.push_back({ THREAD_DIM_X_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.x) });
			initHelper.macro.push_back({ THREAD_DIM_Y_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.y) });
			initHelper.macro.push_back({ THREAD_DIM_Z_SYMBOL, std::to_wstring(initHelper.dispatchInfo.threadDim.z) });
			if (isPerspective)
				initHelper.macro.push_back({ USE_PERSPECTIVE, std::to_wstring(1) });
		}
		static void StuffDispatchInfo(_Inout_ JComputeShaderInitData& initHelper)
		{
			using GpuInfo = Core::JHardwareInfo::GpuInfo;
			std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();

			//수정필요 
			//thread per group factor가 하드코딩됨
			//이후 amd graphic info 추가와 동시에 수정할 예정
			const float threadCount = 512;
			uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
			uint groupDimX = (uint)std::ceil(threadCount / float(gpuInfo[0].maxThreadsDim.x));
			uint groupDimY = threadCount < gpuInfo[0].maxThreadsDim.y ? threadCount : gpuInfo[0].maxThreadsDim.y;

			//textuer size is always 2 squared
			uint threadDimX = threadCount < gpuInfo[0].maxThreadsDim.x ? threadCount : gpuInfo[0].maxThreadsDim.x;
			uint threadDimY = (uint)std::ceil(threadCount / float(gpuInfo[0].maxThreadsDim.y));

			initHelper.dispatchInfo.threadDim = JVector3<uint>(threadDimX, threadDimY, 1);
			initHelper.dispatchInfo.groupDim = JVector3<uint>(groupDimX, groupDimY, 1);
			initHelper.dispatchInfo.taskOriCount = groupDimX * groupDimY * threadDimX * threadDimY;
		}
		static JShaderType::CompileInfo ComputeDepthShaderCompileInfo(const bool isLinear)
		{
			if (isLinear)
				return JShaderType::CompileInfo(L"DebugDepthMap.hlsl", L"LinearMap");
			else
				return JShaderType::CompileInfo(L"DebugDepthMap.hlsl", L"NonLinearMap");
		}
		static JShaderType::CompileInfo ComputeNormalShaderCompileInfo()
		{
			return JShaderType::CompileInfo(L"DebugNormalMap.hlsl", L"VisualizeNormalMap");
		}
		static JShaderType::CompileInfo ComputeAoShaderCompileInfo()
		{
			return JShaderType::CompileInfo(L"DebugAoMap.hlsl", L"VisualizeAoMap");
		}
	}
	JDx12GraphicDebug::DebugDataSet::DebugDataSet(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)
	{
		const JDx12GraphicDebugRsComputeSet* dx12Set = static_cast<const JDx12GraphicDebugRsComputeSet*>(debugSet);
		dx12Device = static_cast<JDx12GraphicDevice*>(dx12Set->device);
		dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12Set->graphicResourceM);
		cmdList = dx12Set->cmdList;
		 
		if (helper.cam != nullptr)
		{ 
			auto gRInterface = helper.cam->GraphicResourceUserInterface();		 
			const uint depthDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
			const uint depthDebugDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
			
			int depthHandleIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::SRV, depthDataIndex);
			int depthDebugHandleIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_BIND_TYPE::UAV, depthDebugDataIndex);

			depthSrcHandle = dx12Gm->GetGpuSrvDescriptorHandle(depthHandleIndex);
			depthDestHandle = dx12Gm->GetGpuSrvDescriptorHandle(depthDebugHandleIndex);
			allowDepth = true;

			const uint rsDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);	  
			const uint normalDebugDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::NORMAL_MAP_VISUALIZE);
			
			const int rsHandleIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::SRV, rsDataIndex); 
			const int normalHandleIndex = gRInterface.GetOptionHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::SRV, J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP, rsDataIndex);
			const int normalDebugHandleIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_BIND_TYPE::UAV, normalDebugDataIndex);		
			if (normalHandleIndex != invalidIndex)
			{
				normalSrcHandle = dx12Gm->GetGpuSrvDescriptorHandle(normalHandleIndex);
				normalDestHandle = dx12Gm->GetGpuSrvDescriptorHandle(normalDebugHandleIndex);
				allowNormal = true;
			}
			const uint aoDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_TASK_TYPE::APPLY_SSAO);
			const uint aoDebugDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::SSAO_VISUALIZE);

			const int aoHandleIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SSAO_MAP, J_GRAPHIC_BIND_TYPE::SRV, aoDataIndex);
			const int aoDebugHandleIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_BIND_TYPE::UAV, aoDebugDataIndex);

			if (aoHandleIndex != invalidIndex)
			{
				aoSrcHandle = dx12Gm->GetGpuSrvDescriptorHandle(aoHandleIndex);
				aoDestHandle = dx12Gm->GetGpuSrvDescriptorHandle(aoDebugHandleIndex);
				allowAo = true;
			}

			size = gRInterface.GetResourceSize(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rsDataIndex);
			nearF = helper.cam->GetNear();
			farF = helper.cam->GetFar();
			isNonLinearDepthMap = !helper.cam->IsOrthoCamera();
		}
		else if (helper.lit != nullptr)
		{
			bool isNonLinearDepthMap = false;
			/*
			* light는 type에 따라서 projection type이 정해진다.
			* directional => orhto ... Linear
			* point, spot => perspective ... NonLinear
			*/ 
			const J_LIGHT_TYPE litType = helper.lit->GetLightType();
			if (litType == J_LIGHT_TYPE::SPOT || litType == J_LIGHT_TYPE::POINT)
				isNonLinearDepthMap = true;	//spot is perspective
	 
			nearF = helper.lit->GetFrustumNear();
			farF = helper.lit->GetFrustumFar(), 
			allowDepth = true;
		} 
		allowOccDepth = helper.allowDrawOccDepthMap;
	} 
	void JDx12GraphicDebug::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		BuildRootSignature(d3d12Device);
		BuildDepthPso(d3d12Device);
		BuildNormalPso(d3d12Device);
		BuildAoPso(d3d12Device);
	}
	void JDx12GraphicDebug::Clear()
	{
		linearDepthMapShaderData = nullptr;
		for (uint i = 0; i < (uint)J_GRAPHIC_PROJECTION_TYPE::COUNT; ++i)
			nonLinearDepthMapShaderData[i] = nullptr;
		normalMapShaderData = nullptr;
		aoMapShaderData = nullptr;
		cRootSignature = nullptr;
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicDebug::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12GraphicDebug::ComputeCamDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(debugSet) || !helper.allowDrawDebugMap)
			return;
		 
		DebugDataSet set(debugSet, helper);
		if (set.allowDepth)
		{
			if (set.isNonLinearDepthMap)
				ComputeDebug(set, DEBUG_TYPE::DEPTH, nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE].get());
			else
				ComputeDebug(set, DEBUG_TYPE::DEPTH, linearDepthMapShaderData.get());
		}
		if(set.allowOccDepth)
			ComputeOccDebug(set, helper);
		if (set.allowNormal)
			ComputeDebug(set, DEBUG_TYPE::NORMAL, normalMapShaderData.get());
		if (set.allowAo)
			ComputeDebug(set, DEBUG_TYPE::AO, aoMapShaderData.get());
	}
	void JDx12GraphicDebug::ComputeLitDebug(const JGraphicDebugRsComputeSet* debugSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(debugSet) || !helper.allowDrawDebugMap)
			return;

		DebugDataSet set(debugSet, helper);

		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType());

		const uint shadowDataIndex = gRInterface.GetResourceDataIndex(grType, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		const uint debugDataIndex = gRInterface.GetResourceDataIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, J_GRAPHIC_TASK_TYPE::DEPTH_MAP_VISUALIZE);
		const uint debugCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP);

		if (set.allowDepth)
		{
			for (uint i = 0; i < debugCount; ++i)
			{
				const uint shadowDataOffset = shadowDataIndex + i;
				const uint debugDataOffset = debugDataIndex + i;

				const int litDsIndex = gRInterface.GetResourceArrayIndex(grType, shadowDataOffset);
				const int litDebugDsIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, debugDataOffset);

				auto depthInfo = set.dx12Gm->GetDxInfo(grType, litDsIndex);
				auto depthDebugInfo = set.dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_MAP, litDebugDsIndex);

				set.depthSrcHandle = set.dx12Gm->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV));
				set.depthDestHandle = set.dx12Gm->GetGpuSrvDescriptorHandle(depthDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV));
				set.size = gRInterface.GetResourceSize(grType, shadowDataOffset);
				if (set.isNonLinearDepthMap)
					ComputeDebug(set, DEBUG_TYPE::DEPTH, nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE].get());
				else
					ComputeDebug(set, DEBUG_TYPE::DEPTH, linearDepthMapShaderData.get());
			}
		}
		if (set.allowOccDepth)
			ComputeOccDebug(set, helper);
	}
	void JDx12GraphicDebug::ComputeOccDebug(DebugDataSet set, const JDrawHelper& helper)
	{ 
		JVector2<uint> occlusionSize = JVector2<uint>(helper.info.occlusionWidth, helper.info.occlusionHeight);

		J_GRAPHIC_RESOURCE_TYPE srcType;
		J_GRAPHIC_RESOURCE_TYPE destType;
		if (helper.option.isOcclusionQueryActivated)
		{
			srcType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP;
			//Debug and mipmap viwe count is same 
			destType = J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG; 
			set.isNonLinearDepthMap = false;
		}
		else
			return;

		auto gRInterface = helper.GetOccGResourceInterface();
		const uint dataCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
		for (uint i = 0; i < dataCount; ++i)
		{
			auto srcInfo = set.dx12Gm->GetDxInfo(srcType, gRInterface.GetResourceArrayIndex(srcType, i));
			auto destInfo = set.dx12Gm->GetDxInfo(destType, gRInterface.GetResourceArrayIndex(destType, i));
			if (srcInfo == nullptr || destInfo == nullptr)
				return;

			const uint viewCount = destInfo->GetViewCount(J_GRAPHIC_BIND_TYPE::SRV);
			for (uint j = 0; j < viewCount; ++j)
			{
				set.size = occlusionSize;
				set.depthSrcHandle = set.dx12Gm->GetGpuSrvDescriptorHandle(srcInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + j);
				set.depthDestHandle = set.dx12Gm->GetGpuSrvDescriptorHandle(destInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV) + j);
				 
				if (set.isNonLinearDepthMap)
					ComputeDebug(set, DEBUG_TYPE::DEPTH, nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE].get());
				else
					ComputeDebug(set, DEBUG_TYPE::DEPTH, linearDepthMapShaderData.get());
				//mipmap
				if (helper.option.isOcclusionQueryActivated)
					occlusionSize /= 2.0f;
			}
		}
	}
	void JDx12GraphicDebug::ComputeDebug(const DebugDataSet& set, const DEBUG_TYPE type, JDx12ComputeShaderDataHolder* holder)
	{
		set.cmdList->SetComputeRootSignature(cRootSignature.Get());
		if (type == DEBUG_TYPE::DEPTH)
		{
			set.cmdList->SetComputeRootDescriptorTable(srcTextureHandleIndex, set.depthSrcHandle);
			set.cmdList->SetComputeRootDescriptorTable(destTextureHandleIndex, set.depthDestHandle);
		}
		else if (type == DEBUG_TYPE::NORMAL)
		{
			set.cmdList->SetComputeRootDescriptorTable(srcTextureHandleIndex, set.normalSrcHandle);
			set.cmdList->SetComputeRootDescriptorTable(destTextureHandleIndex, set.normalDestHandle);
		}
		else if (type == DEBUG_TYPE::AO)
		{
			set.cmdList->SetComputeRootDescriptorTable(srcTextureHandleIndex, set.aoSrcHandle);
			set.cmdList->SetComputeRootDescriptorTable(destTextureHandleIndex, set.aoDestHandle);
		}
	 
		set.cmdList->SetComputeRoot32BitConstants(settingCbIndex, 1, &set.size.x, 0);
		set.cmdList->SetComputeRoot32BitConstants(settingCbIndex, 1, &set.size.y, 1);
		set.cmdList->SetComputeRoot32BitConstants(settingCbIndex, 1, &set.nearF, 2);
		set.cmdList->SetComputeRoot32BitConstants(settingCbIndex, 1, &set.farF, 3);
		set.cmdList->SetPipelineState(holder->pso.Get());
		set.cmdList->Dispatch(1, 512, 1);
	}
	void JDx12GraphicDebug::BuildRootSignature(ID3D12Device* device)
	{
		CD3DX12_DESCRIPTOR_RANGE depthMapTable;
		depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE wTextureTable;
		wTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

		CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];
		slotRootParameter[srcTextureHandleIndex].InitAsDescriptorTable(1, &depthMapTable);
		slotRootParameter[destTextureHandleIndex].InitAsDescriptorTable(1, &wTextureTable);
		slotRootParameter[settingCbIndex].InitAsConstants(4, 0);

		// 생성자 대입자 결과가 다름
		std::vector< CD3DX12_STATIC_SAMPLER_DESC> samDesc
		{
			CD3DX12_STATIC_SAMPLER_DESC(0,
			D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
			0.0f,                               // mipLODBias
			16.0f,                                 // maxAnisotropy
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE)
		};

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, (uint)samDesc.size(), samDesc.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailedHr(hr);
		ThrowIfFailedHr(device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(cRootSignature.GetAddressOf())));
		cRootSignature->SetName(L"Debug RootSignature");
	}
	void JDx12GraphicDebug::BuildDepthPso(ID3D12Device* device)
	{
		linearDepthMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE] = std::make_unique<JDx12ComputeShaderDataHolder>();
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC] = std::make_unique<JDx12ComputeShaderDataHolder>();

		JComputeShaderInitData pInitHelper;
		StuffDispatchInfo(pInitHelper);
		StuffComputeShaderMacro(pInitHelper, true);

		JComputeShaderInitData oInitHelper;
		StuffDispatchInfo(oInitHelper);
		StuffComputeShaderMacro(oInitHelper, false);

		auto linearCompileInfo = ComputeDepthShaderCompileInfo(true);
		auto nonLinearCompileInfo = ComputeDepthShaderCompileInfo(false);

		linearDepthMapShaderData->cs = JDxShaderDataUtil::CompileShader(linearCompileInfo.filePath, pInitHelper.macro, linearCompileInfo.functionName, L"cs_6_0");
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE]->cs = JDxShaderDataUtil::CompileShader(nonLinearCompileInfo.filePath, pInitHelper.macro, nonLinearCompileInfo.functionName, L"cs_6_0");
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC]->cs = JDxShaderDataUtil::CompileShader(nonLinearCompileInfo.filePath, oInitHelper.macro, nonLinearCompileInfo.functionName, L"cs_6_0");

		D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		newShaderPso.pRootSignature = cRootSignature.Get();
		newShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(linearDepthMapShaderData->cs->GetBufferPointer()),
			linearDepthMapShaderData->cs->GetBufferSize()
		};
		ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(linearDepthMapShaderData->pso.GetAddressOf())));

		newShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE]->cs->GetBufferPointer()),
			nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE]->cs->GetBufferSize()
		};
		ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE]->pso.GetAddressOf())));

		newShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC]->cs->GetBufferPointer()),
			nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC]->cs->GetBufferSize()
		};
		ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC]->pso.GetAddressOf())));
	}
	void JDx12GraphicDebug::BuildNormalPso(ID3D12Device* device)
	{
		normalMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();

		JComputeShaderInitData initHelper;
		StuffDispatchInfo(initHelper);
		StuffComputeShaderMacro(initHelper, false);

		auto compileInfo = ComputeNormalShaderCompileInfo();
		normalMapShaderData->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initHelper.macro, compileInfo.functionName, L"cs_6_0");

		D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		newShaderPso.pRootSignature = cRootSignature.Get();
		newShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(normalMapShaderData->cs->GetBufferPointer()),
			normalMapShaderData->cs->GetBufferSize()
		};
		ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(normalMapShaderData->pso.GetAddressOf())));
	}
	void JDx12GraphicDebug::BuildAoPso(ID3D12Device* device)
	{
		aoMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();

		JComputeShaderInitData initHelper;
		StuffDispatchInfo(initHelper);
		StuffComputeShaderMacro(initHelper, false);

		auto compileInfo = ComputeAoShaderCompileInfo();
		aoMapShaderData->cs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, initHelper.macro, compileInfo.functionName, L"cs_6_0");

		D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
		newShaderPso.pRootSignature = cRootSignature.Get();
		newShaderPso.CS =
		{
			reinterpret_cast<BYTE*>(aoMapShaderData->cs->GetBufferPointer()),
			aoMapShaderData->cs->GetBufferSize()
		};
		ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(aoMapShaderData->pso.GetAddressOf())));
	}
}
