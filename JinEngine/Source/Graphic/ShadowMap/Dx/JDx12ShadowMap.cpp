#include"JDx12ShadowMap.h" 
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../Blur/Dx/JDx12Blur.h"
#include"../../Culling/Occlusion/JHZBOccCulling.h"
#include"../../Culling/JCullingInterface.h"
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../FrameResource/JObjectConstants.h" 
#include"../../FrameResource/JAnimationConstants.h" 
#include"../../FrameResource/JShadowMapConstants.h"   

#include"../../GraphicResource/JGraphicResourceInterface.h"
#include"../../GraphicResource/JGraphicResourceUserAccess.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JD3DUtility.h"

#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/Component/Animator/JAnimator.h"
#include"../../../Object/Component/Animator/JAnimatorPrivate.h"
#include"../../../Object/Component/Light/JLight.h"
#include"../../../Object/Component/Light/JLightPrivate.h"
#include"../../../Object/Component/Light/JDirectionalLight.h"
#include"../../../Object/Component/Light/JDirectionalLightPrivate.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/RenderItem/JRenderItemPrivate.h" 
#include"../../../Object/Resource/Scene/JScene.h" 
#include"../../../Object/Resource/Scene/JScenePrivate.h"  
#include"../../../Object/Resource/Mesh/JMeshGeometry.h"
#include"../../../Object/Resource/Mesh/JMeshGeometryPrivate.h" 
#include"../../../Application/JApplicationEngine.h"
 
namespace JinEngine::Graphic
{
	namespace
	{
		using AniFrameIndexInterface = JAnimatorPrivate::FrameIndexInterface;
		using RItemFrameIndexInterface = JRenderItemPrivate::FrameIndexInterface;
		using LitFrameIndexInterface = JLightPrivate::FrameIndexInterface;

		static void ResourceTransition(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
		{
			CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, stateBefore, stateAfter);
			cmdList->ResourceBarrier(1, &rsBarrier);
		}
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
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};
			}
			default:
				break;
			}
			return std::vector<D3D12_INPUT_ELEMENT_DESC>();
		}
		static void StuffMeshMacro(std::vector<JMacroSet>& macro, const Core::J_MESHGEOMETRY_TYPE meshType)noexcept
		{
			switch (meshType)
			{
			case JinEngine::Core::J_MESHGEOMETRY_TYPE::STATIC:
			{
				macro.push_back({ "STATIC", "1" });
				break;
			}
			case JinEngine::Core::J_MESHGEOMETRY_TYPE::SKINNED:
			{
				macro.push_back({ "SKINNED", "2" });
				break;
			}
			default:
				break;
			}
		}
		static void StuffShadowMapMacro(std::vector<JMacroSet>& macro, const J_SHADOW_MAP_TYPE smType)noexcept
		{
			switch (smType)
			{
			case JinEngine::J_SHADOW_MAP_TYPE::NORMAL:
			{
				macro.push_back({ "NORMALSM", "1" });
				break;
			}
			case JinEngine::J_SHADOW_MAP_TYPE::CSM:
			{
				macro.push_back({ "ARRAY", "2" });
				macro.push_back({ "ARRAY_COUNT", std::to_string(JCsmOption::maxCountOfSplit) });
				break;
			}
			case JinEngine::J_SHADOW_MAP_TYPE::CUBE:
			{
				macro.push_back({ "CUBE", "3" });
				break;
			}
			default:
				break;
			}
		}
		static void StuffCsmMacro(std::vector<JMacroSet>& macro, const uint count)noexcept
		{
			macro.push_back({ "ARRAY", "2" });
			macro.push_back({ "ARRAY_COUNT", std::to_string(count) });
		}
		static bool CanUseGs(const J_SHADOW_MAP_TYPE smType)noexcept
		{
			switch (smType)
			{
			case JinEngine::J_SHADOW_MAP_TYPE::NORMAL:
				return false;
			case JinEngine::J_SHADOW_MAP_TYPE::CSM:
				return true;
			case JinEngine::J_SHADOW_MAP_TYPE::CUBE:
				return true;
			default:
				break;
			}
			return false;
		}
		//not use
		template<typename ...Param>
		void CallShadowMapFunc(const JDrawHelper& helper,
			void(JShadowMap::* ptr)(const uint, Param&&...),
			Param&&... param)
		{
			auto gRInterface = helper.lit->GraphicResourceUserInterface();
			const uint dataCount = gRInterface.GetDataCount(JLightType::SmToGraphicR(helper.lit->GetShadowMapType()));
			for (uint i = 0; i < dataCount; ++i)
				(this->*ptr)(i, std::forward<Param>(param)...);
		}
	}
	namespace
	{
		//shadow map draw root
		static constexpr int objCBIndex = 0;	//same as objCB(graphic root)
		static constexpr int aniCBIndex = objCBIndex + 1;
		static constexpr int normalShadowMapDrawCBIndex = aniCBIndex + 1;
		static constexpr int csmDrawCBIndex = normalShadowMapDrawCBIndex + 1;
		static constexpr int cubeShadowMapDrawCBIndex = csmDrawCBIndex + 1;
		static constexpr int slotCount = cubeShadowMapDrawCBIndex + 1;
	}

	void JDx12ShadowMap::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;

		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		DXGI_FORMAT dsvFormat = static_cast<JDx12GraphicResourceManager*>(gM)->GetDepthStencilFormat();
		BuildRootSignature(d3d12Device);
		
		for (uint i = 0; i < (uint)Core::J_MESHGEOMETRY_TYPE::COUNT; ++i)
		{
			normalShadowShaderData[i] = std::make_unique<JDx12GraphicShaderDataHolder>();
			cubeShadowShaderData[i] = std::make_unique<JDx12GraphicShaderDataHolder>();
			 
			Core::J_MESHGEOMETRY_TYPE meshType = (Core::J_MESHGEOMETRY_TYPE)i;

			std::vector<JMacroSet> normalMacro;
			std::vector<JMacroSet> cubeMacro;
			StuffMeshMacro(normalMacro, meshType);
			StuffMeshMacro(cubeMacro, meshType);
			StuffShadowMapMacro(normalMacro, J_SHADOW_MAP_TYPE::NORMAL);
			StuffShadowMapMacro(cubeMacro, J_SHADOW_MAP_TYPE::CUBE);

			BuildPso(d3d12Device, dsvFormat, J_SHADOW_MAP_TYPE::NORMAL, meshType, normalMacro, *normalShadowShaderData[i]);
			BuildPso(d3d12Device, dsvFormat, J_SHADOW_MAP_TYPE::CUBE, meshType, cubeMacro, *cubeShadowShaderData[i]);
		}
		for (uint i = 0; i < JCsmOption::maxCountOfSplit; ++i)
		{
			for (uint j = 0; j < (uint)Core::J_MESHGEOMETRY_TYPE::COUNT; ++j)
			{
				csmShaderData[i][j] = std::make_unique<JDx12GraphicShaderDataHolder>(); 
				Core::J_MESHGEOMETRY_TYPE meshType = (Core::J_MESHGEOMETRY_TYPE)j;
				std::vector<JMacroSet> csmMacro;
				StuffMeshMacro(csmMacro, meshType);
				StuffCsmMacro(csmMacro, i + JCsmOption::minCountOfSplit);
				BuildPso(d3d12Device, dsvFormat, J_SHADOW_MAP_TYPE::CSM, meshType, csmMacro, *csmShaderData[i][j]);
			}
		}
	}
	void JDx12ShadowMap::Clear()
	{
		mRootSignature = nullptr;
		for (uint i = 0; i < (uint)Core::J_MESHGEOMETRY_TYPE::COUNT; ++i)
		{
			normalShadowShaderData[i] = nullptr;
			cubeShadowShaderData[i] = nullptr;
		}

		for (uint i = 0; i < (uint)Core::J_MESHGEOMETRY_TYPE::COUNT; ++i)
		{
			for (uint j = 0; j < JCsmOption::maxCountOfSplit; ++j)
				csmShaderData[i][j] = nullptr;
		}
	}
	J_GRAPHIC_DEVICE_TYPE JDx12ShadowMap::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	JDx12GraphicShaderDataHolder* JDx12ShadowMap::GetShaderDataHolder(const JDrawHelper& helper, const Core::J_MESHGEOMETRY_TYPE mType)
	{
		const J_LIGHT_TYPE litType = helper.lit->GetLightType();
		const J_SHADOW_MAP_TYPE smType = helper.lit->GetShadowMapType();
 
		if (smType == J_SHADOW_MAP_TYPE::CSM)
			return csmShaderData[static_cast<JDirectionalLight*>(helper.lit.Get())->GetCsmSplitCount() - JCsmOption::minCountOfSplit][(uint)mType].get();
		else if (smType == J_SHADOW_MAP_TYPE::CUBE)
			return cubeShadowShaderData[(uint)mType].get();
		else
			return normalShadowShaderData[(uint)mType].get();
	}
	bool JDx12ShadowMap::HasPreprocessing()const noexcept
	{
		return true;
	}
	bool JDx12ShadowMap::HasPostprocessing()const noexcept
	{
		return true;
	}
	void JDx12ShadowMap::BindResource(const JGraphicBindSet* bindSet)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		dx12BindSet->cmdList->SetGraphicsRootSignature(mRootSignature.Get());
	}
	void JDx12ShadowMap::BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const uint smDataCount = gRInterface.GetDataCount(grType); 	 
		for (uint i = 0; i < smDataCount; ++i)
		{
			const int smVecIndex = gRInterface.GetResourceArrayIndex(grType, i);
			const int dsvHeapIndex = gRInterface.GetHeapIndexStart(grType, J_GRAPHIC_BIND_TYPE::DSV, i);

			ID3D12Resource* shdowMapResource = dx12Gm->GetResource(grType, smVecIndex);
			ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			D3D12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}
	}
	void JDx12ShadowMap::EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12BindSet->graphicResourceM);
		ID3D12GraphicsCommandList* cmdList = dx12BindSet->cmdList;

		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const uint smDataCount = gRInterface.GetDataCount(grType); 
		for (uint i = 0; i < smDataCount; ++i)
		{
			auto gRInterface = helper.lit->GraphicResourceUserInterface();
			const int smVecIndex = gRInterface.GetResourceArrayIndex(grType, i);

			ID3D12Resource* shdowMapResource = dx12Gm->GetResource(grType, smVecIndex);
			ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
		}
	}
	void JDx12ShadowMap::DrawSceneShadowMap(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(shadowDrawSet) || !helper.CanDrawShadowMap())
			return;

		const JDx12GraphicShadowMapDrawSet* dx12SmDrawSet = static_cast<const JDx12GraphicShadowMapDrawSet*>(shadowDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12SmDrawSet->device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12SmDrawSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12SmDrawSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12SmDrawSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12SmDrawSet->cmdList;

		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const uint smDataCount = gRInterface.GetDataCount(grType); 
		for (uint i = 0; i < smDataCount; ++i)
		{
			const uint shadowWidth = gRInterface.GetResourceWidth(grType, i);
			const uint shadowHeight = gRInterface.GetResourceHeight(grType, i);
			const int smVecIndex = gRInterface.GetResourceArrayIndex(grType, i);
			const int dsvHeapIndex = gRInterface.GetHeapIndexStart(grType, J_GRAPHIC_BIND_TYPE::DSV, i);

			cmdList->SetGraphicsRootSignature(mRootSignature.Get());

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)shadowWidth, (float)shadowHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, shadowWidth, shadowHeight };

			cmdList->RSSetViewports(1, &mViewport);
			cmdList->RSSetScissorRects(1, &mScissorRect);

			ID3D12Resource* shdowMapResource = dx12Gm->GetResource(grType, smVecIndex);
			ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			D3D12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);

			BindLightFrameResource(cmdList, dx12Frame, helper, i);

			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

			DrawShadowMapGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec00, helper, JDrawCondition(helper, false, true, false), i);
			DrawShadowMapGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, false), i);
			//if (helper.allowOcclusionCulling && helper.lit->AllowHdOcclusionCulling())
			//	cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);

			ResourceTransition(cmdList, shdowMapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
		}
	}
	void JDx12ShadowMap::DrawSceneShadowMapMultiThread(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(shadowDrawSet) || !helper.CanDrawShadowMap())
			return;

		const JDx12GraphicShadowMapDrawSet* dx12SmDrawSet = static_cast<const JDx12GraphicShadowMapDrawSet*>(shadowDrawSet);
		JDx12GraphicDevice* dx12Device = static_cast<JDx12GraphicDevice*>(dx12SmDrawSet->device);
		JDx12FrameResource* dx12Frame = static_cast<JDx12FrameResource*>(dx12SmDrawSet->currFrame);
		JDx12GraphicResourceManager* dx12Gm = static_cast<JDx12GraphicResourceManager*>(dx12SmDrawSet->graphicResourceM);
		JDx12CullingManager* dx12Cm = static_cast<JDx12CullingManager*>(dx12SmDrawSet->cullingM);
		ID3D12GraphicsCommandList* cmdList = dx12SmDrawSet->cmdList;

		//BindRootSignature(cmdList);
		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const uint smDataCount = gRInterface.GetDataCount(grType); 
		for (uint i = 0; i < smDataCount; ++i)
		{
			const uint shadowWidth = gRInterface.GetResourceWidth(grType, i);
			const uint shadowHeight = gRInterface.GetResourceHeight(grType, i);
			const uint dsvHeapIndex = gRInterface.GetHeapIndexStart(grType, J_GRAPHIC_BIND_TYPE::DSV, i);

			cmdList->SetGraphicsRootSignature(mRootSignature.Get());

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)shadowWidth, (float)shadowHeight, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, shadowWidth, shadowHeight };

			cmdList->RSSetViewports(1, &mViewport);
			cmdList->RSSetScissorRects(1, &mScissorRect);

			D3D12_CPU_DESCRIPTOR_HANDLE dsv = dx12Gm->GetCpuDsvDescriptorHandle(dsvHeapIndex);
			cmdList->OMSetRenderTargets(0, nullptr, false, &dsv);
			BindLightFrameResource(cmdList, dx12Frame, helper, i);

			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

			DrawShadowMapGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec00, helper, JDrawCondition(helper, false, true, false), i);
			DrawShadowMapGameObject(cmdList, dx12Frame, dx12Gm, dx12Cm, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, false), i);
			//if (helper.allowOcclusionCulling && helper.lit->AllowHdOcclusionCulling())
			//	cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
		}
	} 
	void JDx12ShadowMap::DrawShadowMapGameObject(ID3D12GraphicsCommandList* cmdList,
		JDx12FrameResource* dx12Frame,
		JDx12GraphicResourceManager* dx12Gm,
		JDx12CullingManager* dx12Cm,
		const std::vector<JUserPtr<JGameObject>>& gameObject,
		const JDrawHelper& helper,
		const JDrawCondition& condition,
		const uint dataIndex)
	{
		uint objectCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JObjectConstants));
		uint skinCBByteSize = JD3DUtility::CalcConstantBufferByteSize(sizeof(JAnimationConstants));

		auto objectCB = dx12Frame->objectCB->GetResource();
		auto skinCB = dx12Frame->skinnedCB->GetResource();

		const uint gameObjCount = (uint)gameObject.size();
		uint st = 0;
		uint ed = gameObjCount;
		if (helper.CanDispatchWorkIndex())
			helper.DispatchWorkIndex(gameObjCount, st, ed);

		uint smIndex = (uint)helper.lit->GetShadowMapType();
		auto cullUser = helper.GetCullInterface();
		for (uint i = st; i < ed; ++i)
		{
			JRenderItem* renderItem = gameObject[i]->GetRenderItem().Get();
			const uint objFrameIndex = helper.GetObjectFrameIndex(renderItem);
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem);

			//share same inedx culling and shadow
			//if (condition.allowCulling && cullUser.IsCulled(boundFrameIndex))
			//	continue;

			JUserPtr<JMeshGeometry> mesh = renderItem->GetMesh();
			const D3D12_VERTEX_BUFFER_VIEW vertexPtr = dx12Gm->VertexBufferView(mesh);
			const D3D12_INDEX_BUFFER_VIEW indexPtr = dx12Gm->IndexBufferView(mesh);

			cmdList->IASetVertexBuffers(0, 1, &vertexPtr);
			cmdList->IASetIndexBuffer(&indexPtr);
			cmdList->IASetPrimitiveTopology(JD3DUtility::ConvertRenderPrimitive(renderItem->GetPrimitiveType()));

			JAnimator* animator = gameObject[i]->GetComponentWithParent<JAnimator>().Get(); 
			const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();

			const bool onSkinned = animator != nullptr && condition.allowAnimation;
			const Core::J_MESHGEOMETRY_TYPE meshType = onSkinned ? Core::J_MESHGEOMETRY_TYPE::SKINNED : Core::J_MESHGEOMETRY_TYPE::STATIC;
			cmdList->SetPipelineState(GetShaderDataHolder(helper, meshType)->pso.Get());

			for (uint j = 0; j < submeshCount; ++j)
			{
				D3D12_GPU_VIRTUAL_ADDRESS objectCBAddress = objectCB->GetGPUVirtualAddress() + (objFrameIndex + j) * objectCBByteSize;
				cmdList->SetGraphicsRootConstantBufferView(objCBIndex, objectCBAddress);
				if (onSkinned)
				{
					D3D12_GPU_VIRTUAL_ADDRESS skinObjCBAddress = skinCB->GetGPUVirtualAddress() + helper.GetAnimationFrameIndex(animator) * skinCBByteSize;
					cmdList->SetGraphicsRootConstantBufferView(aniCBIndex, skinObjCBAddress);
				}
				cmdList->DrawIndexedInstanced(mesh->GetSubmeshIndexCount(j), 1, mesh->GetSubmeshStartIndexLocation(j), mesh->GetSubmeshBaseVertexLocation(j), 0);
			}
		}
	}
	void JDx12ShadowMap::BindLightFrameResource(ID3D12GraphicsCommandList* cmdList, JDx12FrameResource* dx12Frame, const JDrawHelper& helper, const int offset)
	{
		J_SHADOW_MAP_TYPE smType = helper.lit->GetShadowMapType();
		if (smType == J_SHADOW_MAP_TYPE::NONE)
			return;

		const int frameIndex = helper.GetShadowMapDrawFrameIndex() + offset; 
		if (smType == J_SHADOW_MAP_TYPE::NORMAL)
			dx12Frame->smDrawCB->SetGraphicCBBufferView(cmdList, normalShadowMapDrawCBIndex, frameIndex);
		else if (smType == J_SHADOW_MAP_TYPE::CSM)
			dx12Frame->smArrayDrawCB->SetGraphicCBBufferView(cmdList, csmDrawCBIndex, frameIndex);
		else if (smType == J_SHADOW_MAP_TYPE::CUBE)
			dx12Frame->smCubeDrawCB->SetGraphicCBBufferView(cmdList, cubeShadowMapDrawCBIndex, frameIndex);
	}
	void JDx12ShadowMap::BuildRootSignature(ID3D12Device* device)
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[slotCount];
		slotRootParameter[objCBIndex].InitAsConstantBufferView(objCBIndex);
		slotRootParameter[aniCBIndex].InitAsConstantBufferView(aniCBIndex);
		slotRootParameter[normalShadowMapDrawCBIndex].InitAsConstantBufferView(normalShadowMapDrawCBIndex);
		slotRootParameter[csmDrawCBIndex].InitAsConstantBufferView(csmDrawCBIndex);
		slotRootParameter[cubeShadowMapDrawCBIndex].InitAsConstantBufferView(cubeShadowMapDrawCBIndex);

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

		mRootSignature->SetName(L"ShadowMapDraw RootSignature");
	}
	void JDx12ShadowMap::BuildPso(ID3D12Device* device,
		const DXGI_FORMAT dsvFormat,
		const J_SHADOW_MAP_TYPE smType, 
		const Core::J_MESHGEOMETRY_TYPE meshType,
		const std::vector<JMacroSet>& macroSet,
		_Out_ JDx12GraphicShaderDataHolder& data)
	{
		std::wstring gShaderPath = JApplicationEngine::ShaderPath() + L"\\ShadowMapDraw.hlsl";
		const uint smIndex = (uint)smType;

		data.inputLayout = GetInputLayout(meshType);
		std::vector<D3D_SHADER_MACRO> d3dMacro = JDxShaderDataUtil::ToD3d12Macro(macroSet);
		data.vs = JDxShaderDataUtil::CompileShader(gShaderPath, d3dMacro.data(), "VS", "vs_5_1"); 
		if (CanUseGs(smType))
			data.gs = JDxShaderDataUtil::CompileShader(gShaderPath, d3dMacro.data(), "GS", "gs_5_1");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC newShaderPso;
		ZeroMemory(&newShaderPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		newShaderPso.InputLayout = { data.inputLayout.data(), (uint)data.inputLayout.size() };
		newShaderPso.pRootSignature = mRootSignature.Get();
		newShaderPso.VS =
		{
			reinterpret_cast<BYTE*>(data.vs->GetBufferPointer()),
			data.vs->GetBufferSize()
		};
		if (data.gs != nullptr)
		{
			newShaderPso.GS =
			{
				reinterpret_cast<BYTE*>(data.gs->GetBufferPointer()),
				data.gs->GetBufferSize()
			};
		}
		if (data.gs != nullptr)
		{
			newShaderPso.GS =
			{
				reinterpret_cast<BYTE*>(data.gs->GetBufferPointer()),
				data.gs->GetBufferSize()
			};
		}

		newShaderPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		newShaderPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		newShaderPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		newShaderPso.SampleMask = UINT_MAX;
		newShaderPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//newShaderPso.DSVFormat = dsvFormat;
	
		newShaderPso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		newShaderPso.DepthStencilState.StencilEnable = false;
		newShaderPso.RasterizerState.DepthBias = 100000 * 0.25f;
		newShaderPso.RasterizerState.DepthBiasClamp = 0.0f;
		newShaderPso.RasterizerState.SlopeScaledDepthBias = 1.0f;
		newShaderPso.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		newShaderPso.NumRenderTargets = 0;

		newShaderPso.SampleDesc.Count = 1;
		newShaderPso.SampleDesc.Quality = 0;
		ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(data.pso.GetAddressOf())));
	}
}