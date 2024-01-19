#include"JDx12DepthTest.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JD3DUtility.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../FrameResource/JAnimationConstants.h"
#include"../../FrameResource/JObjectConstants.h"
#include"../../FrameResource/JBoundingObjectConstants.h"
#include"../../FrameResource/JDepthTestConstants.h"
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
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

	void JDx12DepthTest::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		DXGI_FORMAT dsvFormat = static_cast<JDx12GraphicResourceManager*>(gM)->GetDepthStencilFormat();

		BuildRootSignature(d3d12Device);
		BuildPso(d3d12Device, dsvFormat);
	}
	void JDx12DepthTest::Clear()
	{
		gShaderData.Clear();
		mRootSignature.Reset();
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
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12DetphTestSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DetphTestSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12DetphTestSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12DetphTestSet->cmdList;

		if (!BindGraphicResource(cmdList, dx12Frame, helper))
			return;

		JUserPtr<JMeshGeometry> mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE);
		const D3D12_VERTEX_BUFFER_VIEW vertexPtr = dx12Gm->VertexBufferView(mesh);
		const D3D12_INDEX_BUFFER_VIEW indexPtr = dx12Gm->IndexBufferView(mesh);

		cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
		cmdList->IASetIndexBuffer(&indexPtr);
		cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		uint boundingObjectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JBoundingObjectConstants));
		auto boundingObjectCB = dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT)->GetResource();

		const uint gameObjCount = (uint)gameObject.size();
		uint st = 0;
		uint ed = gameObjCount;
		if (helper.CanDispatchWorkIndex())
			helper.DispatchWorkIndex(gameObjCount, st, ed);

		auto cullUser = helper.GetCullInterface();
		cmdList->SetPipelineState(gShaderData[TEST_TYPE::BOUNDING_TEST]->pso.Get());
		for (uint i = st; i < ed; ++i)
		{
			JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem);

			if (condition.onlyDrawOccluder && !renderItem->IsOccluder())
				continue;

			if (condition.allowCulling && !renderItem->IsIgnoreCullingResult() && cullUser.IsCulled(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM, boundFrameIndex))
				continue;

			D3D12_GPU_VIRTUAL_ADDRESS boundingObjectCBAddress = boundingObjectCB->GetGPUVirtualAddress() + boundFrameIndex * boundingObjectCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView(objCBIndex, boundingObjectCBAddress);
			cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
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
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12DetphTestSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DetphTestSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12DetphTestSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12DetphTestSet->cmdList;

		if (!BindGraphicResource(cmdList, dx12Frame, helper))
			return;

		auto cInterface = helper.GetCullInterface();
		const int occVecIndex = cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM);
		ID3D12QueryHeap* occQueryHeap = dx12Cm->GetQueryHeap(occVecIndex);

		JUserPtr<JMeshGeometry> mesh = _JResourceManager::Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE);
		const D3D12_VERTEX_BUFFER_VIEW vertexPtr = dx12Gm->VertexBufferView(mesh);
		const D3D12_INDEX_BUFFER_VIEW indexPtr = dx12Gm->IndexBufferView(mesh);

		cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
		cmdList->IASetIndexBuffer(&indexPtr);
		cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		uint boundingObjectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JBoundingObjectConstants));
		auto boundingObjectCB = dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::BOUNDING_OBJECT)->GetResource();

		const uint gameObjCount = (uint)gameObject.size();
		uint st = 0;
		uint ed = gameObjCount;
		if (helper.CanDispatchWorkIndex())
			helper.DispatchWorkIndex(gameObjCount, st, ed);

		auto cullUser = helper.GetCullInterface();
		cmdList->SetPipelineState(gShaderData[TEST_TYPE::QUERY_TEST]->pso.Get());
		for (uint i = st; i < ed; ++i)
		{
			JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem);

			if (condition.onlyDrawOccluder && !renderItem->IsOccluder())
				continue;

			if (condition.allowCulling && !renderItem->IsIgnoreCullingResult() && cullUser.IsCulled(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM, boundFrameIndex))
				continue;

			D3D12_GPU_VIRTUAL_ADDRESS boundingObjectCBAddress = boundingObjectCB->GetGPUVirtualAddress() + boundFrameIndex * boundingObjectCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView(objCBIndex, boundingObjectCBAddress);

			cmdList->BeginQuery(occQueryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, boundFrameIndex);
			cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(0), 1, mesh->GetSubmeshStartIndexLocation(0), mesh->GetSubmeshBaseVertexLocation(0), 0);
			cmdList->EndQuery(occQueryHeap, D3D12_QUERY_TYPE_BINARY_OCCLUSION, boundFrameIndex);
		}
	}
	bool JDx12DepthTest::BindGraphicResource(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return false;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12BindSet->currFrame);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		return BindGraphicResource(cmdList, dx12Frame, helper);
	}
	bool JDx12DepthTest::BindGraphicResource(ID3D12GraphicsCommandList* cmdList, JDx12FrameResource* dx12Frame, const JDrawHelper& helper)
	{
		cmdList->SetGraphicsRootSignature(mRootSignature.Get());
		int frameIndex = -1;

		using CamFrameInterface = JCameraPrivate::FrameIndexInterface;
		using LitFrameInterface = JLightPrivate::FrameIndexInterface;
		if (helper.GetDrawType() == JDrawHelper::DRAW_TYPE::OCC)
		{
			if (helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA)
				frameIndex = helper.GetCamDepthTestPassFrameIndex();
			else if (helper.cullingCompType == J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT)
				frameIndex = helper.GetLitDepthTestPassFrameIndex();
		}

		if (frameIndex == -1)
			return false;
		 
		dx12Frame->GetDx12Buffer(J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS)->SetGraphicCBBufferView(cmdList, passCBIndex, frameIndex);
		return true;
	}
	void JDx12DepthTest::BuildRootSignature(ID3D12Device* device)
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];
		slotRootParameter[objCBIndex].InitAsConstantBufferView(objCBIndex);
		//slotRootParameter[objCBIndex].InitAsConstantBufferView(objCBIndex);
		//slotRootParameter[aniCBIndex].InitAsConstantBufferView(aniCBIndex);
		slotRootParameter[passCBIndex].InitAsConstantBufferView(passCBIndex);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(slotCount, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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

		mRootSignature->SetName(L"DepthTest RootSignature");
	}
	void JDx12DepthTest::BuildPso(ID3D12Device* device, const DXGI_FORMAT depthStencilFormat)
	{
		std::wstring gShaderPath = JApplicationEngine::ShaderPath() + L"\\DepthTest.hlsl";
		for (uint i = 0; i < (uint)TEST_TYPE::COUNT; ++i)
		{
			TEST_TYPE type = (TEST_TYPE)i;
			gShaderData[type] = std::make_unique<JDx12GraphicShaderDataHolder>();
			auto gShaderDataPtr = gShaderData[type].get();

			std::vector<JMacroSet> macro = Private::GetShaderMacro(type);
			gShaderDataPtr->inputLayout = Private::GetInputLayout(Core::J_MESHGEOMETRY_TYPE::STATIC);
			gShaderDataPtr->vs = JDxShaderDataUtil::CompileShader(gShaderPath, macro, L"VS", L"vs_6_0");

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
			ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(gShaderDataPtr->pso.GetAddressOf())));
		}
	}
}