#include"JShadowMap.h"
#include"../JGraphicInfo.h"
#include"../JGraphicOption.h"
#include"../JGraphicUpdateHelper.h"
#include "../Culling/Occlusion/JHZBOccCulling.h"
#include "../Culling/JCullingInterface.h"
#include"../Upload/FrameResource/JFrameResource.h"
#include"../Upload/FrameResource/JObjectConstants.h" 
#include"../Upload/FrameResource/JAnimationConstants.h"
#include"../Upload/FrameResource/JMaterialConstants.h"
#include"../Upload/FrameResource/JPassConstants.h"  
#include"../Upload/FrameResource/JShadowMapConstants.h" 

#include"../GraphicResource/JGraphicResourceInterface.h"
#include"../GraphicResource/JGraphicResourceUserAccess.h"
#include"../GraphicResource/JGraphicResourceManager.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Component/Animator/JAnimator.h"
#include"../../Object/Component/Animator/JAnimatorPrivate.h"
#include"../../Object/Component/Light/JLight.h"
#include"../../Object/Component/Light/JLightPrivate.h"
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Component/RenderItem/JRenderItemPrivate.h"
#include"../../Object/Resource/Scene/JScenePrivate.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Object/Resource/Shader/JShader.h"
#include"../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../Object/Resource/Mesh/JMeshGeometryPrivate.h"
#include"../../Object/Resource/JResourceManager.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
			using AniFrameIndexInterface = JAnimatorPrivate::FrameIndexInterface;
			using RItemFrameIndexInterface = JRenderItemPrivate::FrameIndexInterface;
			using LitFrameIndexInterface = JLightPrivate::FrameIndexInterface;
			using MeshBufferViewInterface = JMeshGeometryPrivate::BufferViewInterface;

			static void ResourceTransition(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
			{
				CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, stateBefore, stateAfter);
				cmdList->ResourceBarrier(1, &rsBarrier);
			}
		}
		void JShadowMap::Initialize(ID3D12Device* device, const JGraphicInfo& gInfo)
		{ }
		void JShadowMap::Clear()
		{}
		void JShadowMap::DrawSceneShadowMap(ID3D12GraphicsCommandList* cmdList,
			JFrameResource* currFrame,
			JHZBOccCulling* occCulling,
			JGraphicResourceManager* graphicResource,
			const JGraphicOption& option,
			const JDrawHelper helper)
		{
			auto gRInterface = helper.lit->GraphicResourceUserInterface();
			const J_GRAPHIC_RESOURCE_TYPE rType = J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP;
			const uint shadowWidth = gRInterface.GetResourceWidth(rType);
			const uint shadowHeight = gRInterface.GetResourceHeight(rType);
			const uint rVecIndex = gRInterface.GetResourceArrayIndex(rType);
			const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(rType, J_GRAPHIC_BIND_TYPE::DSV);

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)shadowWidth, (float)shadowHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, shadowWidth, shadowHeight };

			cmdList->RSSetViewports(1, &mViewport);
			cmdList->RSSetScissorRects(1, &mScissorRect);

			ID3D12Resource* shdowMapResource = graphicResource->GetResource(J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, rVecIndex);
			ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			D3D12_CPU_DESCRIPTOR_HANDLE dsv = graphicResource->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);

			currFrame->passCB->SetGraphicCBBufferView(cmdList, 2, helper.passOffset);
			currFrame->shadowMapCalCB->SetGraphicCBBufferView(cmdList, 5, LitFrameIndexInterface::GetShadowMapFrameIndex(helper.lit.Get()));

			using GCash = JScenePrivate::CashInterface;
			const std::vector<JUserPtr<JGameObject>>& objVec00 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = GCash::GetGameObjectCashVec(helper.scene, J_RENDER_LAYER::OPAQUE_OBJECT, J_MESHGEOMETRY_TYPE::SKINNED);

			DrawShadowMapGameObject(cmdList, currFrame, occCulling, graphicResource, objVec00, helper, JDrawCondition(option, helper, false, true, false));
			DrawShadowMapGameObject(cmdList, currFrame, occCulling, graphicResource, objVec01, helper, JDrawCondition(option, helper, helper.scene->IsActivatedSceneTime(), true, false));

			ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
		}
		void JShadowMap::DrawShadowMapGameObject(ID3D12GraphicsCommandList* cmdList,
			JFrameResource* currFrame,
			JHZBOccCulling* occCulling,
			JGraphicResourceManager* graphicResource,
			const std::vector<JUserPtr<JGameObject>>& gameObject,
			const JDrawHelper helper,
			const JDrawCondition& condition)
		{
			JShader* shadowShader = _JResourceManager::Instance().GetDefaultShader(J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SHADOW_MAP_SHADER).Get();

			uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
			uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

			auto objectCB = currFrame->objectCB->Resource();
			auto skinCB = currFrame->skinnedCB->Resource();
			  
			const uint gameObjCount = (uint)gameObject.size();
			for (uint i = 0; i < gameObjCount; ++i)
			{
				JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
				const uint objFrameIndex = RItemFrameIndexInterface::GetObjectFrameIndex(renderItem);
				const uint boundFrameIndex = RItemFrameIndexInterface::GetBoundingFrameIndex(renderItem);

				if (condition.allowCulling && helper.cullUser.UnsafeIsCulled(boundFrameIndex))
					continue;

				JMeshGeometry* mesh = renderItem->GetMesh().Get();
				const D3D12_VERTEX_BUFFER_VIEW vertexPtr = MeshBufferViewInterface::VertexBufferView(mesh);
				const D3D12_INDEX_BUFFER_VIEW indexPtr = MeshBufferViewInterface::IndexBufferView(mesh);

				cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
				cmdList->IASetIndexBuffer(&indexPtr);
				cmdList->IASetPrimitiveTopology(renderItem->GetPrimitiveType());

				cmdList->SetPipelineState(shadowShader->GetGraphicPso(JShaderType::ConvertToVertexLayout(mesh->GetMeshGeometryType())));

				JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>().Get();
				const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

				for (uint j = 0; j < submeshCount; ++j)
				{
					const bool onSkinned = animator != nullptr && condition.allowAnimation;
					const uint fianlObjFrameIndex = objFrameIndex + j;

					D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + fianlObjFrameIndex * objectCBByteSize;
					cmdList->SetGraphicsRootConstantBufferView(0, objectCBAddress);
					if (onSkinned)
					{
						const uint aniFrameIndex = AniFrameIndexInterface::GetFrameIndex(animator);
						D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + aniFrameIndex * skinCBByteSize;
						cmdList->SetGraphicsRootConstantBufferView(1, skinObjCBAddress);
					}
					if (condition.allowHDOcclusionCulling)
						cmdList->SetPredication(graphicResource->GetOcclusionQueryResult(), fianlObjFrameIndex * 8, D3D12_PREDICATION_OP_EQUAL_ZERO);

					cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
				}
			}
		}
	}
}