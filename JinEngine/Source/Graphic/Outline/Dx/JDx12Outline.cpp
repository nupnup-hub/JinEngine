#include"JDx12Outline.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../Device/Dx/JDx12GraphicDevice.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h" 
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../Utility/Dx/JD3DUtility.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../Object/Resource/Mesh/JMeshGeometryPrivate.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include"../../../Application/JApplicationEngine.h"  
#include<DirectXMath.h>

using namespace DirectX;
namespace JinEngine
{
	namespace Graphic
	{
		void JDx12Outline::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
		{
			if (!IsSameDevice(device) || !IsSameDevice(gM))
				return;

			ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
			DXGI_FORMAT backBufferFormat = static_cast<JDx12GraphicResourceManager*>(gM)->GetBackBufferFormat();
			DXGI_FORMAT depthStencilFormat = static_cast<JDx12GraphicResourceManager*>(gM)->GetDepthStencilFormat();

			BuildRootSignature(d3d12Device);
			BuildPso(d3d12Device, backBufferFormat, depthStencilFormat);
			BuildUploadBuffer(device);
			UpdatePassBuf(info.width, info.height, Constants::commonStencilRef);
		}
		void JDx12Outline::Clear()
		{
			outlineCB.reset();
			mRootSignature.Reset();
			gShaderData.reset();
		} 
		J_GRAPHIC_DEVICE_TYPE JDx12Outline::GetDeviceType()const noexcept
		{
			return J_GRAPHIC_DEVICE_TYPE::DX12;
		}
		void JDx12Outline::UpdatePassBuf(const uint width, const uint height, const uint stencilRefOffset)
		{
			JOutlineConstants constants;
			constants.offset = stencilRefOffset;
			constants.width = width;
			constants.height = height;

			//Quad Mesh값에 따라 설정
			const XMVECTOR s = XMVectorSet(2, 2, 1, 1);
			const XMVECTOR t = XMVectorSet(-1, 1, 0, 1);
			const XMVECTOR q = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

			constants.world.StoreXM(XMMatrixTranspose(XMMatrixAffineTransformation(s, zero, q, t)));
			outlineCB->CopyData(0, constants);
		}
		void JDx12Outline::DrawCamOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper)
		{
			if (!IsSameDevice(drawSet) || !helper.allowDrawDebug)
				return;

			const JDx12GraphicOutlineDrawSet* dx12Set = static_cast<const JDx12GraphicOutlineDrawSet*>(drawSet);
			JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12Set->device);
			JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12Set->graphicResourceM);
			ID3D12GraphicsCommandList* cmdList = dx12Set->cmdList;

			auto gRInterface = helper.cam->GraphicResourceUserInterface();
			const int rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
			const int rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, 0);

			const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
			const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
	
			JDx12GraphicResourceInfo* depthInfo = dx12Gm->GetDxInfo(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
			ID3D12Resource* rtResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);
			D3D12_RESOURCE_DESC desc = rtResource->GetDesc();

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = dx12Gm->GetCpuRtvDescriptorHandle(rtvHeapIndex);
			JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

			D3D12_VIEWPORT viewPort;
			D3D12_RECT rect;
			dx12Device->CalViewportAndRect(JVector2F(desc.Width, desc.Height), viewPort, rect);
			cmdList->RSSetViewports(1, &viewPort);
			cmdList->RSSetScissorRects(1, &rect);
			cmdList->OMSetRenderTargets(1, &rtv, true, nullptr);

			DrawOutline(cmdList,
				dx12Gm,
				dx12Gm->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV)),
				dx12Gm->GetGpuSrvDescriptorHandle(depthInfo->GetHeapIndexStart(J_GRAPHIC_BIND_TYPE::SRV) + 1));
			JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		void JDx12Outline::DrawOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper)
		{
			if (!IsSameDevice(drawSet) || !helper.allowDrawDebug)
				return;

			const JDx12GraphicOutlineDrawSet* dx12Set = static_cast<const JDx12GraphicOutlineDrawSet*>(drawSet);
			JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12Set->graphicResourceM);
			ID3D12GraphicsCommandList* cmdList = dx12Set->cmdList;
			if (!dx12Set->useHandle)
				return;

			DrawOutline(cmdList, dx12Gm, dx12Set->depthMapHandle, dx12Set->stencilMapHandle);
		}
		void JDx12Outline::DrawOutline(ID3D12GraphicsCommandList* cmdList,
			JDx12GraphicResourceManager* dx12Gm,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle)
		{
			cmdList->SetGraphicsRootSignature(mRootSignature.Get());
			cmdList->SetGraphicsRootDescriptorTable(0, depthMapHandle);
			cmdList->SetGraphicsRootDescriptorTable(1, stencilMapHandle);
			cmdList->SetGraphicsRootConstantBufferView(2, outlineCB->GetResource()->GetGPUVirtualAddress());
			cmdList->SetPipelineState(gShaderData->pso.Get());

			JUserPtr<JMeshGeometry> mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::QUAD);

			const D3D12_VERTEX_BUFFER_VIEW vertexPtr = dx12Gm->VertexBufferView(mesh);
			const D3D12_INDEX_BUFFER_VIEW indexPtr = dx12Gm->IndexBufferView(mesh);
 
			cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
			cmdList->IASetIndexBuffer(&indexPtr);
			cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
		}
		void JDx12Outline::BuildRootSignature(ID3D12Device* device)
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

			// create a root signature with a single slot which points to a descriptor length consisting of a single constant buffer
			Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
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

			mRootSignature->SetName(L"Outline RootSignature");
		}
		void JDx12Outline::BuildPso(ID3D12Device* device, const DXGI_FORMAT& rtvFormat, const DXGI_FORMAT& dsvFormat)
		{ 
			std::wstring gShaderPath = JApplicationEngine::ShaderPath() + L"\\Outline.hlsl";

			gShaderData = std::make_unique<JDx12GraphicShaderDataHolder>();
			gShaderData->vs = JDxShaderDataUtil::CompileShader(gShaderPath, L"VS", L"vs_6_0");
			gShaderData->ps = JDxShaderDataUtil::CompileShader(gShaderPath, L"PS", L"ps_6_0");
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
			//newShaderPso.DSVFormat = dsvFormat;
			newShaderPso.DSVFormat = DXGI_FORMAT_UNKNOWN;

			ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(gShaderData->pso.GetAddressOf())));
		}
		void JDx12Outline::BuildUploadBuffer(JGraphicDevice* device)
		{
			outlineCB = std::make_unique<JDx12GraphicBuffer<JOutlineConstants>>(L"OutLine", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
			outlineCB->Build(device, 1);
		}
	}
}