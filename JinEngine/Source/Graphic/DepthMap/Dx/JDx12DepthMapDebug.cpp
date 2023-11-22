#include"JDx12DepthMapDebug.h" 
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
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Component/Light/JLight.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<DirectXColors.h>

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
	}
	void JDx12DepthMapDebug::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		DXGI_FORMAT backBufferFormat = static_cast<JDx12GraphicResourceManager*>(gM)->GetBackBufferFormat();
		DXGI_FORMAT depthStencilFormat = static_cast<JDx12GraphicResourceManager*>(gM)->GetDepthStencilFormat();

		BuildComputeResource(d3d12Device, backBufferFormat, depthStencilFormat);
	}
	void JDx12DepthMapDebug::Clear()
	{
		linearDepthMapShaderData.reset();
		for(uint i = 0; i < (uint)J_GRAPHIC_PROJECTION_TYPE::COUNT; ++i)
			nonLinearDepthMapShaderData[i].reset();
		cRootSignature.Reset();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12DepthMapDebug::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	} 
	void JDx12DepthMapDebug::DrawCamDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(debugSet) || !helper.allowDrawDepthMap)
			return;
 
		const JDx12GraphicDepthMapDebugTaskSet* dx12Set = static_cast<const JDx12GraphicDepthMapDebugTaskSet*>(debugSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12Set->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12Set->cmdList;

		//ortho is linear
		const bool isNonLinearDepthMap = !helper.cam->IsOrthoCamera();
		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		const int sceneDsIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
		const int sceneDebugDsIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, 0);
		 
		auto mainDepthInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, sceneDsIndex);
		auto mainDepthDebugInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, sceneDebugDsIndex);

		if (isNonLinearDepthMap)
		{
			DrawNonLinearDepthDebug(cmdList,
				dx12Gm->GetGpuSrvDescriptorHandle(mainDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
				dx12Gm->GetGpuSrvDescriptorHandle(mainDepthDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
				JVector2<uint>(helper.info.width, helper.info.height),
				helper.cam->GetNear(),
				helper.cam->GetFar(),
				!helper.cam->IsOrthoCamera());
		}
		else
		{
			DrawLinearDepthDebug(cmdList,
				dx12Gm->GetGpuSrvDescriptorHandle(mainDepthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
				dx12Gm->GetGpuSrvDescriptorHandle(mainDepthDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
				JVector2<uint>(helper.info.width, helper.info.height),
				helper.cam->GetNear(),
				helper.cam->GetFar());
		}
	}
	void JDx12DepthMapDebug::DrawLitDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(debugSet) || !helper.allowDrawDepthMap)
			return;

		const JDx12GraphicDepthMapDebugTaskSet* dx12Set = static_cast<const JDx12GraphicDepthMapDebugTaskSet*>(debugSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12Set->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12Set->cmdList;

		bool isNonLinearDepthMap = false;
		const J_LIGHT_TYPE litType = helper.lit->GetLightType();
		if (litType == J_LIGHT_TYPE::SPOT || litType == J_LIGHT_TYPE::POINT)
			isNonLinearDepthMap = true;	//spot is perspective
		 
		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType());
		const uint debugCount = gRInterface.GetDataCount(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG);
		for (uint i = 0; i < debugCount; ++i)
		{
			const uint width = gRInterface.GetResourceWidth(grType, i);
			const uint height = gRInterface.GetResourceWidth(grType, i);

			const int litDsIndex = gRInterface.GetResourceArrayIndex(grType, i);
			const int litDebugDsIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, i);

			auto depthInfo = dx12Gm->GetDxInfo(grType, litDsIndex);
			auto depthDebugInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, litDebugDsIndex);
			 
			/*
			* light는 type에 따라서 projection type이 정해진다.
			* directional => orhto ... Linear
			* point, spot => perspective ... NonLinear
			*/
			if (isNonLinearDepthMap)
			{
				DrawNonLinearDepthDebug(cmdList,
					dx12Gm->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
					dx12Gm->GetGpuSrvDescriptorHandle(depthDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
					JVector2<uint>(width, height),
					helper.lit->GetFrustumNear(),
					helper.lit->GetFrustumFar(),
					true);
			}
			else
			{
				DrawLinearDepthDebug(cmdList,
					dx12Gm->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
					dx12Gm->GetGpuSrvDescriptorHandle(depthDebugInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::UAV)),
					JVector2<uint>(width, height),
					helper.lit->GetFrustumNear(),
					helper.lit->GetFrustumFar());
			}
		}
	}
	void JDx12DepthMapDebug::DrawLinearDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet)
	{
		if (!IsSameDevice(debugSet))
			return;

		const JDx12GraphicDepthMapDebugTaskSet* dx12Set = static_cast<const JDx12GraphicDepthMapDebugTaskSet*>(debugSet);	
		if (!dx12Set->useHandle)
			return;

		DrawLinearDepthDebug(dx12Set->cmdList, dx12Set->srcHandle, dx12Set->destHandle, dx12Set->size, dx12Set->nearF, dx12Set->farF);
	}
	void JDx12DepthMapDebug::DrawNonLinearDepthDebug(const JGraphicDepthMapDebugTaskSet* debugSet)
	{
		if (!IsSameDevice(debugSet))
			return;
		 
		const JDx12GraphicDepthMapDebugTaskSet* dx12Set = static_cast<const JDx12GraphicDepthMapDebugTaskSet*>(debugSet);
		if (!dx12Set->useHandle)
			return;

		DrawNonLinearDepthDebug(dx12Set->cmdList, dx12Set->srcHandle, dx12Set->destHandle, dx12Set->size, dx12Set->nearF, dx12Set->farF, dx12Set->isPerspective);
	}
	void JDx12DepthMapDebug::DrawLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
		const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
		const JVector2<uint> size,
		const float nearF,
		const float farF)
	{
		D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)size.x, (float)size.y, 0.0f, 1.0f };
		D3D12_RECT mScissorRect = { 0, 0, size.x, size.y };

		//commandList->RSSetViewports(1, &mViewport);
		//commandList->RSSetScissorRects(1, &mScissorRect);

		commandList->SetComputeRootSignature(cRootSignature.Get());
		commandList->SetComputeRootDescriptorTable(0, srcHandle);
		commandList->SetComputeRootDescriptorTable(1, destHandle);

		commandList->SetComputeRoot32BitConstants(2, 2, &size, 0); 
		commandList->SetComputeRoot32BitConstants(2, 1, &nearF, 2);
		commandList->SetComputeRoot32BitConstants(2, 1, &farF, 3);
		commandList->SetPipelineState(linearDepthMapShaderData->pso.Get());

		commandList->Dispatch(1, 512, 1);
	}
	void JDx12DepthMapDebug::DrawNonLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
		const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
		const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
		const JVector2<uint> size,
		const float nearF,
		const float farF, 
		const bool isPerspective)
	{
		D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)size.x, (float)size.y, 0.0f, 1.0f };
		D3D12_RECT mScissorRect = { 0, 0, size.x, size.y };

		//commandList->RSSetViewports(1, &mViewport);
		//commandList->RSSetScissorRects(1, &mScissorRect);

		commandList->SetComputeRootSignature(cRootSignature.Get());
		commandList->SetComputeRootDescriptorTable(srcTextureHandleIndex, srcHandle);
		commandList->SetComputeRootDescriptorTable(destTextureHandleIndex, destHandle);

		commandList->SetComputeRoot32BitConstants(settingCbIndex, 1, &size.x, 0);
		commandList->SetComputeRoot32BitConstants(settingCbIndex, 1, &size.y, 1);
		commandList->SetComputeRoot32BitConstants(settingCbIndex, 1, &nearF, 2);
		commandList->SetComputeRoot32BitConstants(settingCbIndex, 1, &farF, 3);
		if (isPerspective)
			commandList->SetPipelineState(nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE]->pso.Get());
		else
			commandList->SetPipelineState(nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC]->pso.Get());

		commandList->Dispatch(1, 512, 1);
	}
	void JDx12DepthMapDebug::BuildComputeResource(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat)
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

		cRootSignature->SetName(L"DepthDebug RootSignature");
		 
		std::wstring computeShaderPath = JApplicationEngine::ShaderPath() + L"\\DepthMapDebug.hlsl";
		linearDepthMapShaderData = std::make_unique<JDx12ComputeShaderDataHolder>();
		linearDepthMapShaderData->cs = JDxShaderDataUtil::CompileShader(computeShaderPath, L"LinearMap", L"cs_6_0");

		std::vector<JMacroSet> macroSet; 
		macroSet.push_back({ L"PERSPECTIVE_DEPTH_MAP", L"1"});
		//macroSet.push_back({ NULL, NULL });

		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE] = std::make_unique<JDx12ComputeShaderDataHolder>();
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::PERSPECTIVE]->cs = JDxShaderDataUtil::CompileShader(computeShaderPath, macroSet, L"NonLinearMap", L"cs_6_0");
		
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC] = std::make_unique<JDx12ComputeShaderDataHolder>();
		nonLinearDepthMapShaderData[(uint)J_GRAPHIC_PROJECTION_TYPE::ORTHOLOGIC]->cs = JDxShaderDataUtil::CompileShader(computeShaderPath,  L"NonLinearMap", L"cs_6_0");

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
}
