#include"JDepthMapDebug.h"
#include"../../Application/JApplicationVariable.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../Utility/JD3DUtility.h"
#include"../../../Lib/DirectX/d3dx12.h" 
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
			cShaderData.reset();
			cRootSignature.Reset();
		}
		void JDepthMapDebug::DrawDepthDebug(ID3D12GraphicsCommandList* commandList, 
			const CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle,
			const JVector2<uint> size)
		{
			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)size.x, (float)size.y, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, size.x, size.y };

			commandList->RSSetViewports(1, &mViewport);
			commandList->RSSetScissorRects(1, &mScissorRect);

			commandList->SetComputeRootSignature(cRootSignature.Get()); 
			commandList->SetComputeRootDescriptorTable(0, srcHandle);
			commandList->SetComputeRootDescriptorTable(1, destHandle);

			commandList->SetPipelineState(cShaderData->Pso.Get());

			commandList->Dispatch(2, 256, 1);
		} 
		void JDepthMapDebug::BuildComputeResource(ID3D12Device* device, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat)
		{
			static constexpr int slotCount = 2;

			CD3DX12_DESCRIPTOR_RANGE depthMapTable;
			depthMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE wTextureTable;
			wTextureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1,0);

			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];
			slotRootParameter[0].InitAsDescriptorTable(1, &depthMapTable);
			slotRootParameter[1].InitAsDescriptorTable(1, &wTextureTable);

			// 생성자 대입자 결과가 다름
			CD3DX12_STATIC_SAMPLER_DESC sample(0, // shaderRegister
				D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_WRAP);

			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, 1, &sample, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
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

			D3D_SHADER_MACRO macro{ NULL, NULL };
			std::wstring computeShaderPath = JApplicationVariable::GetShaderPath() + L"\\DepthMapDebug.hlsl"; 

			cShaderData = std::make_unique<JComputeShaderData>();
			cShaderData->Cs = JD3DUtility::CompileShader(computeShaderPath, &macro, "CS", "cs_5_1");
		 
			D3D12_COMPUTE_PIPELINE_STATE_DESC newShaderPso;
			ZeroMemory(&newShaderPso, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC)); 
			newShaderPso.pRootSignature = cRootSignature.Get();
			newShaderPso.CS =
			{
				reinterpret_cast<BYTE*>(cShaderData->Cs->GetBufferPointer()),
				cShaderData->Cs->GetBufferSize()
			};
			ThrowIfFailedG(device->CreateComputePipelineState(&newShaderPso, IID_PPV_ARGS(cShaderData->Pso.GetAddressOf())));
		}
	}
}