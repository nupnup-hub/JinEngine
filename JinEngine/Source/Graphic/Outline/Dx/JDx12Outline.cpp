#include"JDx12Outline.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../Device/Dx/JDx12GraphicDevice.h" 
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"  
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h"
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"
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
		JDx12Outline::~JDx12Outline()
		{
			ClearResource();
		}
		void JDx12Outline::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
		{
			if (!IsSameDevice(device) || !IsSameDevice(gM))
				return;

			BuildResource(device);
		}
		void JDx12Outline::Clear()
		{
			ClearResource(); 
		} 
		J_GRAPHIC_DEVICE_TYPE JDx12Outline::GetDeviceType()const noexcept
		{
			return J_GRAPHIC_DEVICE_TYPE::DX12;
		} 
		bool JDx12Outline::HasDependency(const JGraphicInfo::TYPE type)const noexcept
		{
			return false;
		}
		bool JDx12Outline::HasDependency(const JGraphicOption::TYPE type)const noexcept
		{
			return type == JGraphicOption::TYPE::RENDERING;
		}
		void JDx12Outline::NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)
		{ 
		}
		void JDx12Outline::NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)
		{
			if (set.preOption.rendering.renderTargetFormat != set.newOption.rendering.renderTargetFormat)
			{
				auto dx12Set = static_cast<const JDx12GraphicOptionChangedSet&>(set);
				Clear();
				Initialize(dx12Set.device, dx12Set.gm);
			}
		}
		void JDx12Outline::UpdatePassBuf(const uint width, const uint height, const uint stencilRefOffset)
		{
			JOutlineConstants constants;
			constants.offset = stencilRefOffset;
			constants.width = width;
			constants.height = height;

			//Quad Mesh값에 따라 설정
			//instead use ndc vertex
			/*
			const XMVECTOR s = XMVectorSet(2, 2, 1, 1);
			const XMVECTOR t = XMVectorSet(-1, 1, 0, 1);
			const XMVECTOR q = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

			constants.world.StoreXM(XMMatrixTranspose(XMMatrixAffineTransformation(s, zero, q, t)));
			*/
			outlineCB->CopyData(0, constants);
		}
		void JDx12Outline::DrawCamOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper)
		{
			if (!IsSameDevice(drawSet) || !helper.allowDrawDebugObject)
				return;

			const JDx12GraphicOutlineDrawSet* dx12Set = static_cast<const JDx12GraphicOutlineDrawSet*>(drawSet);
			JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12Set->context);
 
			auto gRInterface = helper.cam->GraphicResourceUserInterface();
			auto rtSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
			auto dsSet = context->ComputeSet(gRInterface, J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_TASK_TYPE::SCENE_DRAW);
  
			context->Transition(rtSet.holder, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			context->SetViewportAndRect(rtSet.info->GetResourceSize());
			context->SetRenderTargetView(rtSet);
  
			//stencil srv
			dsSet.viewOffset += 1;
			DrawOutline(context, rtSet.GetGpuSrvHandle(), dsSet.GetGpuSrvHandle());
			context->Transition(rtSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		}
		void JDx12Outline::DrawOutline(const JGraphicOutlineDrawSet* drawSet, const JDrawHelper& helper)
		{
			if (!IsSameDevice(drawSet) || !helper.allowDrawDebugObject)
				return;

			const JDx12GraphicOutlineDrawSet* dx12Set = static_cast<const JDx12GraphicOutlineDrawSet*>(drawSet);
			JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12Set->context);

			if (!dx12Set->useHandle)
				return;

			DrawOutline(context, dx12Set->depthMapHandle, dx12Set->stencilMapHandle);
		}
		void JDx12Outline::DrawOutline(JDx12CommandContext* context,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle,
			const CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle)
		{
			context->SetGraphicsRootSignature(mRootSignature.Get());
			context->SetPipelineState(gShaderData.get());
			context->SetGraphicsRootDescriptorTable(0, depthMapHandle);
			context->SetGraphicsRootDescriptorTable(1, stencilMapHandle);
			context->SetGraphicsRootConstantBufferView(2, outlineCB->GetResource()->GetGPUVirtualAddress());
			context->DrawFullScreenTriangle();
		}
		void JDx12Outline::BuildResource(JGraphicDevice* device)
		{ 
			ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice(); 
			BuildRootSignature(d3d12Device);
			BuildPso(d3d12Device);
			BuildUploadBuffer(device);
			UpdatePassBuf(GetGraphicInfo().width, GetGraphicInfo().height, Constants::commonStencilRef);
		}
		void JDx12Outline::BuildRootSignature(ID3D12Device* device)
		{
			static constexpr int slotCount = 3;
			JDx12RootSignatureBuilder2<slotCount, 1> builder;
			builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);		//depthMap
			builder.PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);		//stencilMap
			builder.PushConstantsBuffer(0);									//passCB
			std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam 
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
			for (const auto& data : sam)
				builder.PushSampler(data);
			builder.Create(device, L"Outline RootSignature", mRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_NONE);
		}
		void JDx12Outline::BuildPso(ID3D12Device* device)
		{
			const DXGI_FORMAT rtvFormat = Constants::GetRenderTargetFormat(GetGraphicOption().rendering.renderTargetFormat);
			const DXGI_FORMAT dsvHformat = Constants::GetDepthStencilFormat();

			//const DXGI_FORMAT& rtvFormat, const DXGI_FORMAT& dsvFormat
			std::wstring vertexPath = JApplicationEngine::ShaderPath() + L"\\FullScreenTriangleVs.hlsl";
			std::wstring pixelPath = JApplicationEngine::ShaderPath() + L"\\Outline.hlsl";

			gShaderData = std::make_unique<JDx12GraphicShaderDataHolder>();
			gShaderData->vs = JDxShaderDataUtil::CompileShader(vertexPath, L"FullScreenTriangleVS", L"vs_6_0");
			gShaderData->ps = JDxShaderDataUtil::CompileShader(pixelPath, L"PS", L"ps_6_0");
			/*gShaderData->inputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};*/

			D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
			ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			//newShaderPso.InputLayout = { gShaderData->inputLayout.data(), (uint)gShaderData->inputLayout.size() };
			newShaderPso.InputLayout.NumElements = 0;
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

			ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(gShaderData->GetPsoAddress())));
		}
		void JDx12Outline::BuildUploadBuffer(JGraphicDevice* device)
		{
			outlineCB = std::make_unique<JDx12GraphicBufferT<JOutlineConstants>>(L"OutLine", J_GRAPHIC_BUFFER_TYPE::UPLOAD_CONSTANT);
			outlineCB->Build(device, 1);
		}
		void JDx12Outline::ClearResource()
		{
			outlineCB.reset();
			mRootSignature.Reset();
			gShaderData.reset();
		}
	}
}