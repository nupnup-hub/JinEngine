#include"JDx12SceneDraw.h" 
#include"../JSceneDrawConstants.h" 
#include"../../JGraphicUpdateHelper.h"
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../Culling/JCullingInterface.h"
#include"../../Culling/JCullingInfo.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../FrameResource/JObjectConstants.h" 
#include"../../FrameResource/JAnimationConstants.h" 
#include"../../FrameResource/JMaterialConstants.h" 
#include"../../FrameResource/JPassConstants.h" 
#include"../../FrameResource/JCameraConstants.h" 
#include"../../FrameResource/JLightConstants.h"     
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../Utility/JD3DUtility.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include"../../../Object/Component/Animator/JAnimator.h" 
#include"../../../Object/Component/Camera/JCamera.h" 
#include"../../../Object/Component/Transform/JTransform.h" 
#include"../../../Object/Component/RenderItem/JRenderItem.h" 
#include"../../../Object/Resource/Mesh/JMeshGeometry.h" 
#include"../../../Object/Resource/Material/JMaterial.h"
#include"../../../Object/Resource/Shader/JShader.h"
#include"../../../Object/Resource/Scene/JScene.h" 
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Application/JApplicationEngine.h"
 
namespace JinEngine::Graphic
{
	namespace Private
	{
		static const std::unordered_map<J_SHADER_VERTEX_LAYOUT, std::vector<D3D12_INPUT_ELEMENT_DESC>> inputLayout
		{
			{SHADER_VERTEX_LAYOUT_STATIC,
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			}},
			{SHADER_VERTEX_LAYOUT_SKINNED,
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			}},
		};
		static const std::vector<CD3DX12_STATIC_SAMPLER_DESC> Sampler()noexcept
		{
			return std::vector<CD3DX12_STATIC_SAMPLER_DESC>
			{
				//PointWrap
				CD3DX12_STATIC_SAMPLER_DESC(0, // shaderRegister
					D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
					D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
					D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
					D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

					//pointClamp
					CD3DX12_STATIC_SAMPLER_DESC(1, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

						//linearWrap
					CD3DX12_STATIC_SAMPLER_DESC(2, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_WRAP), // addressW

						//linearClamp
					CD3DX12_STATIC_SAMPLER_DESC(3, // shaderRegister
						D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP), // addressW

						//anisotropicWrap
					CD3DX12_STATIC_SAMPLER_DESC(4, // shaderRegister
						D3D12_FILTER_ANISOTROPIC, // filter
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
						0.0f,                             // mipLODBias
						8),				                  // maxAnisotropy

						//anisotropicClamp
					CD3DX12_STATIC_SAMPLER_DESC(5, // shaderRegister
						D3D12_FILTER_ANISOTROPIC, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
						0.0f,                              // mipLODBias
						8),                                // maxAnisotropy

					//shadow cube not cmp
					CD3DX12_STATIC_SAMPLER_DESC(6, // shaderRegister
						D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
						0.0f,                               // mipLODBias
						16,                                 // maxAnisotropy
						D3D12_COMPARISON_FUNC_LESS_EQUAL,
						D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK),

					//shadow pcss find bloker
					CD3DX12_STATIC_SAMPLER_DESC(7, // shaderRegister
						//D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
						D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
						0.0f,                               // mipLODBias
						16,                                 // maxAnisotropy
						D3D12_COMPARISON_FUNC_LESS_EQUAL),

					//shadow pcss filter
					CD3DX12_STATIC_SAMPLER_DESC(8, // shaderRegister
						D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
						//D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
						0.0f,                               // mipLODBias
						16,                                 // maxAnisotropy
						D3D12_COMPARISON_FUNC_LESS_EQUAL),

					//shadow linear point cmp
					CD3DX12_STATIC_SAMPLER_DESC(9, // shaderRegister
						D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
						D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
						0.0f,                               // mipLODBias
						16,                                 // maxAnisotropy
						D3D12_COMPARISON_FUNC_LESS_EQUAL,
						D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
			};
		}
		static D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertD3d12PrimitiveType(const J_SHADER_PRIMITIVE_TYPE primitiveType)noexcept
		{
			switch (primitiveType)
			{
			case JinEngine::J_SHADER_PRIMITIVE_TYPE::DEFAULT:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// defualt value
			case JinEngine::J_SHADER_PRIMITIVE_TYPE::LINE:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			default:
				break;
			}
		}
		static D3D12_COMPARISON_FUNC ConvertD3d12Comparesion(const J_SHADER_DEPTH_COMPARISON_FUNC depthCompareFunc)noexcept
		{
			switch (depthCompareFunc)
			{
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT:
				return D3D12_COMPARISON_FUNC_LESS;	// defualt value
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::NEVER:
				return D3D12_COMPARISON_FUNC_NEVER;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::LESS:
				return D3D12_COMPARISON_FUNC_LESS;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::EQUAL:
				return D3D12_COMPARISON_FUNC_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL:
				return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::GREATER:
				return D3D12_COMPARISON_FUNC_GREATER;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::NOT_EQUAL:
				return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::GREATER_EQUAL:
				return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::ALWAYS:
				return D3D12_COMPARISON_FUNC_ALWAYS;
			default:
				break;
			}
		}
		static D3D12_CULL_MODE ConvertD3d12CullMode(const bool isCullModeNone)noexcept
		{
			if (isCullModeNone)
				return D3D12_CULL_MODE_NONE;
			else
				return D3D12_CULL_MODE_BACK;	//default value
		}
	}
	void JDx12SceneDraw::Initialize(JGraphicDevice* device, JGraphicResourceManager* gm, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gm))
			return;

		BuildRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), info);
	}
	void JDx12SceneDraw::Clear()
	{
		mRootSignature.Reset();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12SceneDraw::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	bool JDx12SceneDraw::HasPreprocessing()const noexcept
	{
		return true;
	}
	bool JDx12SceneDraw::HasPostprocessing()const noexcept
	{
		return true;
	}
	void JDx12SceneDraw::BindResource(const JGraphicBindSet* bindSet)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12BindSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		cmdList->SetGraphicsRootSignature(mRootSignature.Get());
		dx12Frame->enginePassCB->SetGraphicCBBufferView(cmdList, Constants::enginePassCBIndex, 0);
		dx12Frame->dLightBuffer->SetGraphicsRootShaderResourceView(cmdList, Constants::dLitBuffIndex);
		dx12Frame->pLightBuffer->SetGraphicsRootShaderResourceView(cmdList, Constants::pLitBuffIndex);
		dx12Frame->sLightBuffer->SetGraphicsRootShaderResourceView(cmdList, Constants::sLitBuffIndex);
		dx12Frame->csmBuffer->SetGraphicsRootShaderResourceView(cmdList, Constants::csmBuffIndex);
		dx12Frame->materialBuffer->SetGraphicsRootShaderResourceView(cmdList, Constants::matBuffIndex);

		cmdList->SetGraphicsRootDescriptorTable(Constants::texture2DBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D));
		cmdList->SetGraphicsRootDescriptorTable(Constants::textureCubeBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE));
		cmdList->SetGraphicsRootDescriptorTable(Constants::textureShadowMapBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP));
		cmdList->SetGraphicsRootDescriptorTable(Constants::textureShadowMapArrayBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY));
		cmdList->SetGraphicsRootDescriptorTable(Constants::textureShadowMapCubeBuffIndex, dx12Gm->GetFirstGpuSrvDescriptorHandle(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_CUBE));
	}
	void JDx12SceneDraw::BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		//cam always has one rtv and dsv
		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		const int rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
		const int rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, 0);

		const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
		const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
 
		ID3D12Resource* dsResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
		ID3D12Resource* rtResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = dx12Gm->GetCpuRtvDescriptorHandle(rtvHeapIndex);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);

		JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

		cmdList->ClearRenderTargetView(rtv, dx12Gm->GetBackBufferClearColor(), 0, nullptr);
		cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}
	void JDx12SceneDraw::EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
		const int rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);

		ID3D12Resource* dsResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
		ID3D12Resource* rtResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);

		JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	void JDx12SceneDraw::DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12DrawSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12DrawSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		auto cInterface = helper.GetCullInterface();

		const int rtvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
		const int rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, 0);

		const int dsvVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, 0);
		const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);

		const int camFrameIndex = helper.GetCamFrameIndex();
		const int sceneFrameIndex = helper.GetPassFrameIndex();

		const D3D12_VIEWPORT viewPort = dx12Device->GetViewPort();
		const D3D12_RECT rect = dx12Device->GetRect();
		cmdList->RSSetViewports(1, &viewPort);
		cmdList->RSSetScissorRects(1, &rect);

		ID3D12Resource* dsResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, dsvVecIndex);
		ID3D12Resource* rtResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, rtvVecIndex);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = dx12Gm->GetCpuRtvDescriptorHandle(rtvHeapIndex);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);

		JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);	 
		
		ID3D12Resource* hdResource = nullptr;
		if (cInterface.HasCullingData(J_CULLING_TYPE::HD_OCCLUSION))
		{
			hdResource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, cInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION));
			JD3DUtility::ResourceTransition(cmdList, hdResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PREDICATION);
		}
		/*else
		{			
			//Debug
			auto firstcam = helper.scene->FindFirstSelectedCamera(false);
			if (firstcam != nullptr && firstcam->AllowHdOcclusionCulling())
			{
				auto cullInterface = firstcam->CullingUserInterface();
				hdResource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, cullInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION));
				JD3DUtility::ResourceTransition(cmdList, hdResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PREDICATION);
			}
		}*/

		cmdList->ClearRenderTargetView(rtv, dx12Gm->GetBackBufferClearColor(), 0, nullptr);
		cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, true, &dsv);

		dx12Frame->scenePassCB->SetGraphicCBBufferView(cmdList, Constants::scenePassCBIndex, sceneFrameIndex);
		dx12Frame->cameraCB->SetGraphicCBBufferView(cmdList, Constants::camCBIndex, camFrameIndex);

		const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);
		const std::vector<JUserPtr<JGameObject>>& objVec02 = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec03 = helper.GetGameObjectCashVec(J_RENDER_LAYER::SKY, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec04 = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_UI, Core::J_MESHGEOMETRY_TYPE::STATIC);
 
		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec00, helper, JDrawCondition(helper, false, true, helper.allowDrawDebug));
		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebug));
		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebug));
		if (helper.allowOcclusionCulling && helper.cam->AllowHdOcclusionCulling())
			cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
		if (helper.allowDrawDebug)
			DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec02, helper, JDrawCondition());
		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec03, helper, JDrawCondition());
		if (helper.allowDrawDebug)
		{
			const int debugVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, 0);
			const int debugHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
			ID3D12Resource* debugResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, debugVecIndex);
			JGraphicResourceInfo* debugDepthInfo = dx12Gm->GetInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, debugVecIndex);

			CD3DX12_CPU_DESCRIPTOR_HANDLE editorDsv = dx12Gm->GetCpuDsvDescriptorHandle(debugHeapIndex);
			JD3DUtility::ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			cmdList->ClearDepthStencilView(editorDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			cmdList->OMSetRenderTargets(1, &rtv, true, &editorDsv);
			DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec04, helper, JDrawCondition());
			JD3DUtility::ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		}

		JD3DUtility::ResourceTransition(cmdList, rtResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		JD3DUtility::ResourceTransition(cmdList, dsResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
		if (hdResource != nullptr)
			JD3DUtility::ResourceTransition(cmdList, hdResource, D3D12_RESOURCE_STATE_PREDICATION, D3D12_RESOURCE_STATE_COMMON);
	}
	void JDx12SceneDraw::DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet))
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12DrawSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12DrawSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		auto gRInterface = helper.cam->GraphicResourceUserInterface();

		const int rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, 0);
		const int dsvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::SCENE_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);

		const int camFrameIndex = helper.GetCamFrameIndex();
		const int sceneFrameIndex = helper.GetPassFrameIndex();

		const D3D12_VIEWPORT viewPort = dx12Device->GetViewPort();
		const D3D12_RECT rect = dx12Device->GetRect();
		cmdList->RSSetViewports(1, &viewPort);
		cmdList->RSSetScissorRects(1, &rect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = dx12Gm->GetCpuRtvDescriptorHandle(rtvHeapIndex);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);

		cmdList->OMSetRenderTargets(1, &rtv, true, &dsv);

		dx12Frame->scenePassCB->SetGraphicCBBufferView(cmdList, Constants::scenePassCBIndex, sceneFrameIndex);
		dx12Frame->cameraCB->SetGraphicCBBufferView(cmdList, Constants::camCBIndex, camFrameIndex);

		const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);
		const std::vector<JUserPtr<JGameObject>>& objVec02 = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
		const std::vector<JUserPtr<JGameObject>>& objVec03 = helper.GetGameObjectCashVec(J_RENDER_LAYER::SKY, Core::J_MESHGEOMETRY_TYPE::STATIC);

		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec00, helper, JDrawCondition(helper, false, true, helper.allowDrawDebug));
		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, helper.allowDrawDebug));
		if (helper.allowOcclusionCulling && helper.cam->AllowHdOcclusionCulling())
			cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
		if (helper.allowDrawDebug)
			DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec02, helper, JDrawCondition());

		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec03, helper, JDrawCondition());
	}
	void JDx12SceneDraw::DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(drawSet) || !helper.allowDrawDebug)
			return;

		const JDx12GraphicSceneDrawSet* dx12DrawSet = static_cast<const JDx12GraphicSceneDrawSet*>(drawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12DrawSet->device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12DrawSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12DrawSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12DrawSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12DrawSet->cmdList;

		auto gRInterface = helper.cam->GraphicResourceUserInterface();
		const int rtvHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, J_GRAPHIC_BIND_TYPE::RTV, 0);

		const int camFrameIndex = helper.GetCamFrameIndex();
		const int sceneFrameIndex = helper.GetPassFrameIndex();

		const D3D12_VIEWPORT viewPort = dx12Device->GetViewPort();
		const D3D12_RECT rect = dx12Device->GetRect();
		cmdList->RSSetViewports(1, &viewPort);
		cmdList->RSSetScissorRects(1, &rect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = dx12Gm->GetCpuRtvDescriptorHandle(rtvHeapIndex);

		dx12Frame->scenePassCB->SetGraphicCBBufferView(cmdList, Constants::scenePassCBIndex, sceneFrameIndex);
		dx12Frame->cameraCB->SetGraphicCBBufferView(cmdList, Constants::camCBIndex, camFrameIndex);

		const std::vector<JUserPtr<JGameObject>>& objVec = helper.GetGameObjectCashVec(J_RENDER_LAYER::DEBUG_UI, Core::J_MESHGEOMETRY_TYPE::STATIC);

		const int debugVecIndex = gRInterface.GetResourceArrayIndex(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, 0);
		const int debugHeapIndex = gRInterface.GetHeapIndexStart(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, J_GRAPHIC_BIND_TYPE::DSV, 0);
		ID3D12Resource* debugResource = dx12Gm->GetResource(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, debugVecIndex);
		JGraphicResourceInfo* debugDepthInfo = dx12Gm->GetInfo(J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL, debugVecIndex);

		CD3DX12_CPU_DESCRIPTOR_HANDLE editorDsv = dx12Gm->GetCpuDsvDescriptorHandle(debugHeapIndex);
		JD3DUtility::ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		cmdList->ClearDepthStencilView(editorDsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, true, &editorDsv);
		DrawGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec, helper, JDrawCondition());
		JD3DUtility::ResourceTransition(cmdList, debugResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ);
	}
	void JDx12SceneDraw::DrawGameObject(ID3D12GraphicsCommandList* cmdList,
		JDx12FrameResource* dx12Frame,
		JDx12GraphicResourceManager* dx12Gm,
		JDx12CullingManager* dx12Cm,
		const std::vector<JUserPtr<JGameObject>>& gameObject,
		const JDrawHelper& helper,
		const JDrawCondition& condition)
	{
		ID3D12Resource* hdOccResource = nullptr;
		if (condition.allowHDOcclusionCulling && helper.cam->AllowHdOcclusionCulling())
		{
			auto cullInterface = helper.cam->CullingUserInterface();
			hdOccResource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, cullInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION));
		}

		//Debug
		//Scene Observer로 main cam hd occ Debug시 가끔 main cam에 hd occ 가시성결과가 비정확할때가 있는데(hd occ opion on, off시 한번이라도 업데이트 하면 정상작동)
		//Debug 이외에는 문제가 발생하지않으며 그 문제도 한번에 업데이트로 없어지므로 굳이 특정하지않고 사용하도록한다.
		/*if (condition.allowHDOcclusionCulling && condition.allowAllCullingResult)
		{
			auto firstcam = helper.scene->FindFirstSelectedCamera(false);
			if (firstcam != nullptr && firstcam->AllowHdOcclusionCulling())
			{ 
				auto cullInterface = firstcam->CullingUserInterface();
				hdOccResource = dx12Cm->GetResource(J_CULLING_TYPE::HD_OCCLUSION, cullInterface.GetArrayIndex(J_CULLING_TYPE::HD_OCCLUSION));
			}
		}*/
		const bool canUseHd = hdOccResource != nullptr;
		uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
		uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

		auto objectCB = dx12Frame->objectCB->GetResource();
		auto skinCB = dx12Frame->skinnedCB->GetResource();

		const uint gameObjCount = (uint)gameObject.size();
		uint st = 0;
		uint ed = gameObjCount;
		if (helper.CanDispatchWorkIndex())
			helper.DispatchWorkIndex(gameObjCount, st, ed);

		auto cullUser = helper.GetCullInterface(); 
		for (uint i = st; i < ed; ++i)
		{
			JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
			const uint objFrameIndex = helper.GetObjectFrameIndex(renderItem);
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem);

			if (condition.allowCulling && cullUser.IsCulled(boundFrameIndex))
				continue;

			if (condition.allowAllCullingResult && helper.RefelectOtherCamCullig(boundFrameIndex))
				continue;

			JUserPtr<JMeshGeometry> mesh = renderItem->GetMesh();
			const D3D12_VERTEX_BUFFER_VIEW vertexPtr = dx12Gm->VertexBufferView(mesh);
			const D3D12_INDEX_BUFFER_VIEW indexPtr = dx12Gm->IndexBufferView(mesh);

			cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
			cmdList->IASetIndexBuffer(&indexPtr);
			cmdList->IASetPrimitiveTopology(renderItem->GetPrimitiveType());

			JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>().Get();
			const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

			if (condition.allowDebugOutline && gameObject[i]->IsSelected())
				cmdList->OMSetStencilRef(Constants::outlineStencilRef);
			 
			for (uint j = 0; j < submeshCount; ++j)
			{
				const JShader* shader = renderItem->GetValidMaterial(j)->GetShader().Get();
				const bool onSkinned = animator != nullptr && condition.allowAnimation;
				const Core::J_MESHGEOMETRY_TYPE meshType = onSkinned ? Core::J_MESHGEOMETRY_TYPE::SKINNED : Core::J_MESHGEOMETRY_TYPE::STATIC;
				const J_SHADER_VERTEX_LAYOUT shaderLayout = JShaderType::ConvertToVertexLayout(meshType);

				JDx12GraphicShaderDataHolder* dx12ShaderData = static_cast<JDx12GraphicShaderDataHolder*>(shader->GetGraphicData(shaderLayout).Get());
				if (condition.allowDebugOutline && gameObject[i]->IsSelected())
					cmdList->SetPipelineState(dx12ShaderData->extraPso[(uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::STENCIL_WRITE_ALWAYS].Get());
				else
					cmdList->SetPipelineState(dx12ShaderData->pso.Get());

				D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + (objFrameIndex + j) * objectCBByteSize;
				cmdList->SetGraphicsRootConstantBufferView(Constants::objCBIndex, objectCBAddress);
				if (onSkinned && meshType == Core::J_MESHGEOMETRY_TYPE::SKINNED)
				{
					D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + helper.GetAnimationFrameIndex(animator) * skinCBByteSize;
					cmdList->SetGraphicsRootConstantBufferView(Constants::skinCBIndex, skinObjCBAddress);
				}
				if (canUseHd)
					cmdList->SetPredication(hdOccResource, boundFrameIndex * 8, D3D12_PREDICATION_OP_EQUAL_ZERO);
				cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
			}
			if (condition.allowDebugOutline && gameObject[i]->IsSelected())
				cmdList->OMSetStencilRef(Constants::commonStencilRef);
		}
	}
	void JDx12SceneDraw::NotifyChangeGraphicShaderMacro(JGraphicDevice* device, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device))
			return;

		ReBuildRootSignature(static_cast<JDx12GraphicDevice*>(device)->GetDevice(), info);
	}
	JOwnerPtr<JGraphicShaderDataHolderBase> JDx12SceneDraw::CreateGraphicShader(JGraphicDevice* device, JGraphicResourceManager* gResourceM, const JGraphicShaderInitData& initData)
	{
		if (!IsSameDevice(device))
			return nullptr;

		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(device);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(gResourceM);
		auto holder = Core::JPtrUtil::MakeOwnerPtr<JDx12GraphicShaderDataHolder>();

		CompileShader(holder.Get(), initData);
		StuffInputLayout(holder->inputLayout, initData.layoutType);
		StuffPso(holder.Get(),
			dx12Device,
			dx12Gm,
			initData.gFunctionFlag,
			J_GRAPHIC_SHADER_EXTRA_FUNCTION::NONE,
			initData.condition);

		for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT; ++i)
		{
			StuffPso(holder.Get(),
				dx12Device,
				dx12Gm,
				initData.gFunctionFlag,
				(J_GRAPHIC_SHADER_EXTRA_FUNCTION)i,
				initData.condition);
		}

		return std::move(holder);
	}
	void JDx12SceneDraw::CompileShader(JDx12GraphicShaderDataHolder* holder, const JGraphicShaderInitData& initData)
	{
		std::wstring vertexShaderPath = JApplicationEngine::ShaderPath() + L"\\VertexShader.hlsl";
		std::wstring pixelShaderPath = JApplicationEngine::ShaderPath() + L"\\PixelShader.hlsl";

		auto d3dMacro = JDxShaderDataUtil::ToD3d12Macro(initData.macro[initData.layoutType]);
		holder->vs = JD3DUtility::CompileShader(vertexShaderPath, d3dMacro.data(), "VS", "vs_5_1");
		holder->ps = JD3DUtility::CompileShader(pixelShaderPath, d3dMacro.data(), "PS", "ps_5_1");
	}
	void JDx12SceneDraw::StuffInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag)
	{
		outInputLayout = Private::inputLayout.find(vertexLayoutFlag)->second;
	}
	void JDx12SceneDraw::StuffPso(JDx12GraphicShaderDataHolder* holder,
		JDx12GraphicDevice* dx12Device,
		JDx12GraphicResourceManager* dx12Gm,
		const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const J_GRAPHIC_SHADER_EXTRA_FUNCTION extraType,
		const JShaderCondition& condition)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		newShaderPso.InputLayout = { holder->inputLayout.data(), (uint)holder->inputLayout.size() };
		newShaderPso.pRootSignature = mRootSignature.Get();
		newShaderPso.VS =
		{
			reinterpret_cast<BYTE*>(holder->vs->GetBufferPointer()),
			holder->vs->GetBufferSize()
		};
		newShaderPso.PS =
		{
			reinterpret_cast<BYTE*>(holder->ps->GetBufferPointer()),
			holder->ps->GetBufferSize()
		};
		if (holder->hs != nullptr)
		{
			newShaderPso.HS =
			{
				reinterpret_cast<BYTE*>(holder->hs->GetBufferPointer()),
				holder->hs->GetBufferSize()
			};
		}
		if (holder->ds != nullptr)
		{
			newShaderPso.DS =
			{
				reinterpret_cast<BYTE*>(holder->ds->GetBufferPointer()),
				holder->ds->GetBufferSize()
			};
		}
		if (holder->gs != nullptr)
		{
			newShaderPso.GS =
			{
				reinterpret_cast<BYTE*>(holder->gs->GetBufferPointer()),
				holder->gs->GetBufferSize()
			};
		}

		newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		if (extraType == J_GRAPHIC_SHADER_EXTRA_FUNCTION::STENCIL_WRITE_ALWAYS)
		{
			newShaderPso.DepthStencilState.StencilEnable = true;
			newShaderPso.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;
			newShaderPso.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_REPLACE;
		}

		if ((gFunctionFlag & SHADER_FUNCTION_SKY) > 0)
		{
			newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			//newShaderPso.DepthStencilState.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; 
			newShaderPso.DepthStencilState.StencilEnable = false;
		}

		newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		newShaderPso.SampleMask = UINT_MAX;
		newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		newShaderPso.NumRenderTargets = 1;
		newShaderPso.RTVFormats[0] = dx12Gm->GetBackBufferFormat();
		newShaderPso.SampleDesc.Count = dx12Device->GetM4xMsaaState() ? 4 : 1;
		newShaderPso.SampleDesc.Quality = dx12Device->GetM4xMsaaState() ? (dx12Device->GetM4xMsaaQuality() - 1) : 0;

		newShaderPso.DSVFormat = dx12Gm->GetDepthStencilFormat();
		if ((gFunctionFlag & SHADER_FUNCTION_DEBUG) > 0)
		{
			//newShaderPso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			newShaderPso.DepthStencilState.StencilEnable = false;
		}
		//Debug
		//For Occ
		/*
		if (((gFunctionFlag & SHADER_FUNCTION_LIGHT) > 0) && ((gFunctionFlag & SHADER_FUNCTION_SHADOW) > 0))
		{
			CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
			blendDesc.RenderTarget[0] =
			{
				TRUE, FALSE,
				D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};
			newShaderPso.BlendState = blendDesc;
		}
		*/

		if (condition.primitiveCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.PrimitiveTopologyType = Private::ConvertD3d12PrimitiveType(condition.primitiveType);
		if (condition.depthCompareCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.DepthStencilState.DepthFunc = Private::ConvertD3d12Comparesion(condition.depthCompareFunc);
		if (condition.cullModeCondition == J_SHADER_APPLIY_CONDITION::APPLY)
			newShaderPso.RasterizerState.CullMode = Private::ConvertD3d12CullMode(condition.isCullModeNone);

		if (extraType == J_GRAPHIC_SHADER_EXTRA_FUNCTION::NONE)
		{
			ThrowIfFailedG(dx12Device->GetDevice()->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(holder->pso.GetAddressOf())));
			holder->pso->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Pso") - 1, "Pso");
		}
		else if (extraType == J_GRAPHIC_SHADER_EXTRA_FUNCTION::STENCIL_WRITE_ALWAYS)
		{
			ThrowIfFailedG(dx12Device->GetDevice()->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(holder->extraPso[(int)extraType].GetAddressOf())));
			holder->extraPso[(int)extraType]->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Extra Pso") - 1, "Extra Pso");
		}
	}
	void JDx12SceneDraw::BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info)
	{
		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_DESCRIPTOR_RANGE tex2DTable;
		tex2DTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.binding2DTextureCapacity, 2, 0);

		CD3DX12_DESCRIPTOR_RANGE cubeMapTable;
		cubeMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingCubeMapCapacity, 2, 1);

		CD3DX12_DESCRIPTOR_RANGE shadowMapTable;
		shadowMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCapacity, 2, 2);

		CD3DX12_DESCRIPTOR_RANGE shadowMapArryTable;
		shadowMapArryTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureArrayCapacity, 2, 3);

		CD3DX12_DESCRIPTOR_RANGE shadowMapCubeTable;
		shadowMapCubeTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, info.bindingShadowTextureCubeCapacity, 2, 4);

		CD3DX12_ROOT_PARAMETER slotRootParameter[Constants::graphicRootSignatureSlotCount];

		// Create root CBV.
		slotRootParameter[Constants::objCBIndex].InitAsConstantBufferView(Constants::objCBIndex);
		slotRootParameter[Constants::skinCBIndex].InitAsConstantBufferView(Constants::skinCBIndex);
		slotRootParameter[Constants::enginePassCBIndex].InitAsConstantBufferView(Constants::enginePassCBIndex);
		slotRootParameter[Constants::scenePassCBIndex].InitAsConstantBufferView(Constants::scenePassCBIndex);
		slotRootParameter[Constants::camCBIndex].InitAsConstantBufferView(Constants::camCBIndex);
		//slotRootParameter[Constants::litIndexCBIndex].InitAsConstants(2, Constants::litIndexCBIndex);
		//slotRootParameter[5].InitAsConstantBufferView(5);
		//slotRootParameter[Constants::boundObjCBIndex].InitAsConstantBufferView(Constants::boundObjCBIndex);

		//Light Buff
		slotRootParameter[Constants::dLitBuffIndex].InitAsShaderResourceView(0, 0);
		slotRootParameter[Constants::pLitBuffIndex].InitAsShaderResourceView(0, 1);
		slotRootParameter[Constants::sLitBuffIndex].InitAsShaderResourceView(0, 2);
		slotRootParameter[Constants::csmBuffIndex].InitAsShaderResourceView(0, 3);
		//Material Buff
		slotRootParameter[Constants::matBuffIndex].InitAsShaderResourceView(1);
		//Texture Buff
		slotRootParameter[Constants::texture2DBuffIndex].InitAsDescriptorTable(1, &tex2DTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Constants::textureCubeBuffIndex].InitAsDescriptorTable(1, &cubeMapTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Constants::textureShadowMapBuffIndex].InitAsDescriptorTable(1, &shadowMapTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Constants::textureShadowMapArrayBuffIndex].InitAsDescriptorTable(1, &shadowMapArryTable, D3D12_SHADER_VISIBILITY_ALL);
		slotRootParameter[Constants::textureShadowMapCubeBuffIndex].InitAsDescriptorTable(1, &shadowMapCubeTable, D3D12_SHADER_VISIBILITY_ALL);

		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> sam = Private::Sampler();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Constants::graphicRootSignatureSlotCount,
			slotRootParameter,
			(uint)sam.size(), sam.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
			IID_PPV_ARGS(mRootSignature.GetAddressOf())));

		mRootSignature->SetName(L"Main RootSignature");
	}
	void JDx12SceneDraw::ReBuildRootSignature(ID3D12Device* device, const JGraphicInfo& info)
	{
		if (mRootSignature == nullptr)
			return;

		mRootSignature.Reset();
		BuildRootSignature(device, info);
	}
}