#include"JDx12DepthTest.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JDx12Utility.h"
#include"../../Utility/Dx/JDx12ObjectCreation.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../FrameResource/JAnimationConstants.h"
#include"../../FrameResource/JObjectConstants.h" 
#include"../../FrameResource/JOcclusionConstants.h"
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h" 
#include"../../JGraphicInfo.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../../Object/Component/Transform/JTransform.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../../../Object/Component/Camera/JCamera.h"
#include"../../../Object/Component/Camera/JCameraPrivate.h"
#include"../../../Object/Component/Light/JLight.h"
#include"../../../Object/Component/Light/JLightPrivate.h"
#include"../../../Object/Component/Light/JDirectionalLight.h"
#include"../../../Object/Component/Light/JDirectionalLightPrivate.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../Object/Resource/Mesh/JMeshGeometryPrivate.h"  
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Core/Identity/JIdentifier.h"
#include"../../../Application/JApplicationEngine.h"
 
namespace JinEngine::Graphic
{
	namespace
	{
		static constexpr uint objCBIndex = 0;
		static constexpr uint passCBIndex = objCBIndex + 1;
		//static constexpr uint aniCBIndex = objCBIndex + 1;
		//static constexpr uint passCBIndex = aniCBIndex + 1;
		static constexpr uint slotCount = passCBIndex + 1;
	}
	namespace Private
	{
		static std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout(const Core::J_MESHGEOMETRY_TYPE meshType)noexcept
		{
			switch (meshType)
			{
			case JinEngine::Core::J_MESHGEOMETRY_TYPE::STATIC:
			{
				return
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};
			}
			case JinEngine::Core::J_MESHGEOMETRY_TYPE::SKINNED:
			{
				return
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } ,
					{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};
			}
			default:
				break;
			}
			return std::vector<D3D12_INPUT_ELEMENT_DESC>();
		}
		static std::vector<JMacroSet> GetShaderMacro(const JDepthTest::TEST_TYPE testType)
		{
			std::vector<JMacroSet> macro;
			switch (testType)
			{
			case JinEngine::Graphic::JDepthTest::TEST_TYPE::QUERY_TEST:
			{
				macro.push_back({ L"OCCLUSION_QUERY", L"1" });
				break;
			}
			case JinEngine::Graphic::JDepthTest::TEST_TYPE::QUERY_TEST_PASS:
			{
				macro.push_back({ L"OCCLUSION_QUERY", L"1" });
				break;
			}
			default:
				break;
			}
			//macro.push_back({ NULL, NULL });
			return macro;
		}
	}

	JDx12DepthTest::~JDx12DepthTest()
	{
		ClearResource();
	}
	void JDx12DepthTest::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;
		 
		BuildResource(device, gM, GetGraphicInfo());
	}
	void JDx12DepthTest::Clear()
	{
		ClearResource(); 
	}
	J_GRAPHIC_DEVICE_TYPE JDx12DepthTest::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12DepthTest::DrawSceneBoundingBox(const JGraphicDepthMapDrawSet* drawSet,
		const std::vector<JUserPtr<JGameObject>>& gameObject,
		const JDrawHelper& helper,
		const JDrawCondition& condition)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicDepthMapDrawSet* dx12DetphTestSet = static_cast<const JDx12GraphicDepthMapDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DetphTestSet->context);
		if (!BindGraphicResource(context, helper))
			return;
   
		uint st, ed= 0; 
		helper.DispatchWorkIndex(gameObject.size(), st, ed);

		auto cullUser = helper.GetCullInterface();
		JUserPtr<JMeshGeometry> mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE);
		
		context->SetMeshGeometryData(mesh);
		context->SetPipelineState(gShaderData[TEST_TYPE::BOUNDING_TEST].get());
		for (uint i = st; i < ed; ++i)
		{
			JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem);

			if (condition.onlyDrawOccluder && !renderItem->IsOccluder())
				continue;

			if (condition.allowCulling && !renderItem->IsIgnoreCullingResult() && cullUser.IsCulled(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM, boundFrameIndex))
				continue;
			 
			context->SetGraphicsRootConstantBufferView(objCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, boundFrameIndex);		 
			context->DrawIndexedInstanced(mesh);
		}
	}
	void JDx12DepthTest::DrawHdOcclusionQueryObject(const JGraphicDepthMapDrawSet* drawSet,
		const std::vector<JUserPtr<JGameObject>>& gameObject,
		const JDrawHelper& helper,
		const JDrawCondition& condition)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicDepthMapDrawSet* dx12DetphTestSet = static_cast<const JDx12GraphicDepthMapDrawSet*>(drawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12DetphTestSet->context);
		if (!BindGraphicResource(context, helper))
			return;

		auto cInterface = helper.GetCullInterface();
		auto cSet = context->ComputeSet(cInterface, J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
		ID3D12QueryHeap* occQueryHeap = context->GetQueryHeap(cSet.info->GetArrayIndex());

		uint st, ed = 0;
		helper.DispatchWorkIndex(gameObject.size(), st, ed);

		JUserPtr<JMeshGeometry> mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE);
		auto cullUser = helper.GetCullInterface();
		
		context->SetPipelineState(gShaderData[TEST_TYPE::QUERY_TEST].get());
		context->SetMeshGeometryData(mesh);
		for (uint i = st; i < ed; ++i)
		{
			JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem);

			if (condition.onlyDrawOccluder && !renderItem->IsOccluder())
				continue;

			if (condition.allowCulling && !renderItem->IsIgnoreCullingResult() && cullUser.IsCulled(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM, boundFrameIndex))
				continue;

			const bool canQuery = condition.IsValidDrawingIndex(boundFrameIndex);
			context->SetGraphicsRootConstantBufferView(objCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT, boundFrameIndex);
			
			if(canQuery)
				context->BeginQuery(occQueryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, boundFrameIndex);
			context->DrawIndexedInstanced(mesh);
			if(canQuery)
				context->EndQuery(occQueryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, boundFrameIndex);
		}
	}
	bool JDx12DepthTest::BindGraphicResource(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return false;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12BindSet->context);

		return BindGraphicResource(context, helper);
	}
	bool JDx12DepthTest::BindGraphicResource(JDx12CommandContext* context, const JDrawHelper& helper)
	{
		context->SetGraphicsRootSignature(mRootSignature.Get()); 
		int frameIndex = invalidIndex;

		using CamFrameInterface = JCameraPrivate::FrameIndexInterface;
		using LitFrameInterface = JLightPrivate::FrameIndexInterface;
		if (helper.GetDrawType() == JDrawHelper::DRAW_TYPE::OCC)
		{
			if (helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
				frameIndex = helper.GetCamFrameIndex(CameraFrameLayer::depthTest);
			else if (helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
				frameIndex = helper.GetLitFrameIndex(LightFrameLayer::depthTest);
		}

		if (frameIndex == invalidIndex)
			return false;
		 
		context->SetGraphicsRootConstantBufferView(passCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, frameIndex);
		return true;
	}
	void JDx12DepthTest::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		DXGI_FORMAT dsvFormat = Constants::GetDepthStencilFormat();

		BuildRootSignature(d3d12Device);
		BuildPso(d3d12Device, dsvFormat);
	}
	void JDx12DepthTest::BuildRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<slotCount> builder;
		builder.PushConstantsBuffer(objCBIndex);		 
		builder.PushConstantsBuffer(passCBIndex);		 
		builder.Create(device, L"DepthTest RootSignature", mRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}
	void JDx12DepthTest::BuildPso(ID3D12Device* device, const DXGI_FORMAT depthStencilFormat)
	{
		JCompileInfo compileInfo(L"DepthTest.hlsl", L"VS"); 
		for (uint i = 0; i < (uint)TEST_TYPE::COUNT; ++i)
		{
			TEST_TYPE type = (TEST_TYPE)i;
			gShaderData[type] = std::make_unique<JDx12GraphicShaderDataHolder>();
			auto gShaderDataPtr = gShaderData[type].get();

			std::vector<JMacroSet> macro = Private::GetShaderMacro(type);
			gShaderDataPtr->inputLayout = Private::GetInputLayout(Core::J_MESHGEOMETRY_TYPE::STATIC);
			gShaderDataPtr->vs = JDxShaderDataUtil::CompileShader(compileInfo.filePath, macro, compileInfo.functionName, L"vs_6_0");

			D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
			ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			newShaderPso.InputLayout = { gShaderDataPtr->inputLayout.data(), (uint)gShaderDataPtr->inputLayout.size() };
			newShaderPso.pRootSignature = mRootSignature.Get();
			newShaderPso.VS =
			{
				reinterpret_cast<BYTE*>(gShaderDataPtr->vs->GetBufferPointer()),
				gShaderDataPtr->vs->GetBufferSize()
			};

			newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			newShaderPso.SampleMask = UINT_MAX;
			newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			newShaderPso.DepthStencilState.StencilEnable = false;
			newShaderPso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
			newShaderPso.NumRenderTargets = 0;
			newShaderPso.SampleDesc.Count = 1;
			newShaderPso.SampleDesc.Quality = 0;

			newShaderPso.RasterizerState.AntialiasedLineEnable = false;
			newShaderPso.RasterizerState.MultisampleEnable = false;
			newShaderPso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

			if (type == TEST_TYPE::QUERY_TEST)
			{
				newShaderPso.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
				newShaderPso.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
				newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
				newShaderPso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
				//same as main ds format
				newShaderPso.DSVFormat = depthStencilFormat;
				//newShaderPso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			}
			else if (type == TEST_TYPE::QUERY_TEST_PASS)
			{
				newShaderPso.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
				newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
				newShaderPso.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
				newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
				newShaderPso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				//same as main ds format
				newShaderPso.DSVFormat = depthStencilFormat;
			}
			else if (type == TEST_TYPE::BOUNDING_TEST)
			{
				//newShaderPso.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
				newShaderPso.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
				newShaderPso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
				newShaderPso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			}
			ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(gShaderDataPtr->GetPsoAddress())));
		}
	}
	void JDx12DepthTest::ClearResource()
	{
		gShaderData.Clear();
		mRootSignature.Reset();
	}
}