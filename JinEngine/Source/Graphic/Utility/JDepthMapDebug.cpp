#include"JDepthMapDebug.h"
#include"../../Application/JApplicationEngine.h" 
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../Utility/JD3DUtility.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include<DirectXColors.h>
using namespace DirectX;
using namespace Microsoft::WRL;

namespace JinEngine
{
	namespace Graphic
	{
		void JDepthMapDebug::Initialize(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat)
		{
			BuildComputeResource(device, backBufferFormat, depthStencilFormat);
		}
		void JDepthMapDebug::Clear()
		{
			linearDepthMapShaderData.reset();
			nonLinearDepthMapShaderData.reset();
			cRootSignature.Reset();
		}
		void JDepthMapDebug::DrawLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
			const JVector2<uint> size,
			const float camNear,
			const float camFar)
		{
			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)size.x, (float)size.y, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, size.x, size.y };

			commandList->RSSetViewports(1, &mViewport);
			commandList->RSSetScissorRects(1, &mScissorRect);

			commandList->SetComputeRootSignature(cRootSignature.Get()); 
			commandList->SetComputeRootDescriptorTable(0, srcHandle);
			commandList->SetComputeRootDescriptorTable(1, destHandle);

			commandList->SetComputeRoot32BitConstants(2, 1, &size.x, 0);
			commandList->SetComputeRoot32BitConstants(2, 1, &size.y, 1);
			commandList->SetComputeRoot32BitConstants(2, 1, &camNear, 2);
			commandList->SetComputeRoot32BitConstants(2, 1, &camFar, 3);
			commandList->SetPipelineState(linearDepthMapShaderData->pso.Get());

			commandList->Dispatch(1, 512, 1);
		} 	 
		void JDepthMapDebug::DrawNonLinearDepthDebug(ID3D12GraphicsCommandList* commandList,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
			const JVector2<uint> size,
			const float camNear,
			const float camFar)
		{
			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)size.x, (float)size.y, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, size.x, size.y };

			commandList->RSSetViewports(1, &mViewport);
			commandList->RSSetScissorRects(1, &mScissorRect);

			commandList->SetComputeRootSignature(cRootSignature.Get());
			commandList->SetComputeRootDescriptorTable(0, srcHandle);
			commandList->SetComputeRootDescriptorTable(1, destHandle);

			commandList->SetComputeRoot32BitConstants(2, 1, &size.x, 0);
			commandList->SetComputeRoot32BitConstants(2, 1, &size.y, 1);
			commandList->SetComputeRoot32BitConstants(2, 1, &camNear, 2);
			commandList->SetComputeRoot32BitConstants(2, 1, &camFar, 3);
			commandList->SetPipelineState(nonLinearDepthMapShaderData->pso.Get());

			commandList->Dispatch(1, 512, 1);
		}
		void JDepthMapDebug::BuildComputeResource(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat)
		{
			static constexpr int slotCount = 3;

			CD3DX12_DESCRIPTOR_RANGE depthMapTable;
			depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE wTextureTable;
			wTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];
			slotRootParameter[0].InitAsDescriptorTable(1, &depthMapTable);
			slotRootParameter[1].InitAsDescriptorTable(1, &wTextureTable);
			slotRootParameter[2].InitAsConstants(4, 0);

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

			cRootSignature->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("DepthDebug RootSignature") - 1, "DepthDebug RootSignature");

			D3D_SHADER_MACRO macro{ NULL, NULL };
			std::wstring computeShaderPath = JApplicationEngine::ShaderPath() + L"\\DepthMapDebug.hlsl";

			linearDepthMapShaderData = std::make_unique<JComputeShaderData>();
			linearDepthMapShaderData->cs = JD3DUtility::CompileShader(computeShaderPath, &macro, "LinearMap", "cs_5_1");

			nonLinearDepthMapShaderData = std::make_unique<JComputeShaderData>();
			nonLinearDepthMapShaderData->cs = JD3DUtility::CompileShader(computeShaderPath, &macro, "NonLinearMap", "cs_5_1");

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
				reinterpret_cast<BYTE*>(nonLinearDepthMapShaderData->cs->GetBufferPointer()),
				nonLinearDepthMapShaderData->cs->GetBufferSize()
			};
			ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(nonLinearDepthMapShaderData->pso.GetAddressOf())));
		}
	}
}