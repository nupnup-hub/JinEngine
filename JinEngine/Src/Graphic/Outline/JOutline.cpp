#include"JOutline.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Application/JApplicationVariable.h"  
#include<DirectXMath.h>
using namespace DirectX;
namespace JinEngine
{
	namespace Graphic
	{
		void JOutline::Initialize(ID3D12Device* device, const DXGI_FORMAT& rtvFormat, const DXGI_FORMAT& dsvFormat, const uint width, const uint height, const uint cbOffset)
		{
			BuildRootSignature(device);
			BuildPso(device, rtvFormat, dsvFormat);
			BuildUploadBuffer(device);
			UpdatePassBuf(width, height, cbOffset);
		}
		void JOutline::Clear()
		{
			mRootSignature.Reset();
			gShaderData.reset();
		}
		void JOutline::UpdatePassBuf(const uint width, const uint height, const uint stencilRefOffset)
		{
			JOutlineConstants constants;
			constants.offset = stencilRefOffset;
			constants.width = width;
			constants.height = height;

			const XMVECTOR s = XMVectorSet(2, 2, 1, 1);
			const XMVECTOR t = XMVectorSet(-1, 1, 0, 1);
			const XMVECTOR q = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

			XMStoreFloat4x4(&constants.world, XMMatrixTranspose(XMMatrixAffineTransformation(s, zero, q, t)));
			outlineCB->CopyData(0, constants);
		}
		void JOutline::DrawOutline(ID3D12GraphicsCommandList* commandList, const CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle, const CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle)
		{
			commandList->SetGraphicsRootSignature(mRootSignature.Get());
			commandList->SetGraphicsRootDescriptorTable(0, depthMapHandle);
			commandList->SetGraphicsRootDescriptorTable(1, stencilMapHandle);
			commandList->SetGraphicsRootConstantBufferView(2, outlineCB->Resource()->GetGPUVirtualAddress());
			commandList->SetPipelineState(gShaderData->pso.Get());

			JMeshGeometry* mesh = JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD);

			const D3D12_VERTEX_BUFFER_VIEW vertexPtr = mesh->VertexBufferView();
			const D3D12_INDEX_BUFFER_VIEW indexPtr = mesh->IndexBufferView();

			commandList->IASetVertexBuffers(0, 1, &vertexPtr);
			commandList->IASetIndexBuffer(&indexPtr);
			commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
		}
		void JOutline::BuildRootSignature(ID3D12Device* device)
		{
			static constexpr int slotCount = 3;

			CD3DX12_DESCRIPTOR_RANGE depthMap;
			depthMap.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_DESCRIPTOR_RANGE stencilMap;
			stencilMap.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

			CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];
			slotRootParameter[0].InitAsDescriptorTable(1, &depthMap);
			slotRootParameter[1].InitAsDescriptorTable(1, &stencilMap);
			slotRootParameter[2].InitAsConstantBufferView(0);

			std::vector<CD3DX12_STATIC_SAMPLER_DESC> samDesc
			{
				CD3DX12_STATIC_SAMPLER_DESC(0,
				D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
				0.0f,                               // mipLODBias
				1,                                 // maxAnisotropy
				D3D12_COMPARISON_FUNC_LESS_EQUAL,
				D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK),
			};

			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, (uint)samDesc.size(), samDesc.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
				IID_PPV_ARGS(mRootSignature.GetAddressOf())));
		}
		void JOutline::BuildPso(ID3D12Device* device, const DXGI_FORMAT& rtvFormat, const DXGI_FORMAT& dsvFormat)
		{
			D3D_SHADER_MACRO macro{ NULL, NULL };
			std::wstring gShaderPath = JApplicationVariable::GetShaderPath() + L"\\Outline.hlsl";

			gShaderData = std::make_unique<JGraphicShaderData>();
			gShaderData->vs = JD3DUtility::CompileShader(gShaderPath, &macro, "VS", "vs_5_1");
			gShaderData->ps = JD3DUtility::CompileShader(gShaderPath, &macro, "PS", "ps_5_1");
			gShaderData->inputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
			ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			newShaderPso.InputLayout = { gShaderData->inputLayout.data(), (uint)gShaderData->inputLayout.size() };
			newShaderPso.pRootSignature = mRootSignature.Get();
			newShaderPso.VS =
			{
				reinterpret_cast<BYTE*>(gShaderData->vs->GetBufferPointer()),
				gShaderData->vs->GetBufferSize()
			};
			newShaderPso.PS =
			{
				reinterpret_cast<BYTE*>(gShaderData->ps->GetBufferPointer()),
				gShaderData->ps->GetBufferSize()
			};
			newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			newShaderPso.DepthStencilState.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			//newShaderPso.DepthStencilState.DepthEnable = false;

			D3D12_RENDER_TARGET_BLEND_DESC outlineBlendDesc;
			outlineBlendDesc.BlendEnable = true;
			outlineBlendDesc.LogicOpEnable = false;
			outlineBlendDesc.SrcBlend = D3D12_BLEND_ONE;
			outlineBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			outlineBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
			outlineBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			outlineBlendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			outlineBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			outlineBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
			outlineBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			 
			newShaderPso.BlendState.RenderTarget[0] = outlineBlendDesc;
			newShaderPso.SampleMask = UINT_MAX;
			newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			newShaderPso.NumRenderTargets = 1;
			newShaderPso.RTVFormats[0] = rtvFormat;
			newShaderPso.SampleDesc.Count = 1;
			newShaderPso.SampleDesc.Quality = 0;
			newShaderPso.DSVFormat = dsvFormat;

			ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(gShaderData->pso.GetAddressOf())));
		}
		void JOutline::BuildUploadBuffer(ID3D12Device* device)
		{
			outlineCB = std::make_unique<JUploadBuffer<JOutlineConstants>>(J_UPLOAD_BUFFER_TYPE::CONSTANT);
			outlineCB->Build(device, 1);
		}
	}
}