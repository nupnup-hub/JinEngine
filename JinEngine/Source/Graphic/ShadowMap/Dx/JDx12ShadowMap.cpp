#include"JDx12ShadowMap.h" 
#include"../../JGraphicInfo.h"
#include"../../JGraphicOption.h"
#include"../../JGraphicUpdateHelper.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h" 
#include"../../Culling/Occlusion/JHZBOccCulling.h"
#include"../../Culling/JCullingInterface.h"
#include"../../Culling/Dx/JDx12CullingManager.h"
#include"../../Command/Dx/JDx12CommandContext.h"
#include"../../FrameResource/Dx/JDx12FrameResource.h"
#include"../../FrameResource/JObjectConstants.h" 
#include"../../FrameResource/JAnimationConstants.h" 
#include"../../FrameResource/JLightConstants.h"   
#include"../../Utility/Dx/JDx12ObjectCreation.h"

#include"../../GraphicResource/JGraphicResourceInterface.h"
#include"../../GraphicResource/JGraphicResourceUserAccess.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceInfo.h" 
#include"../../Device/Dx/JDx12GraphicDevice.h"
#include"../../Utility/Dx/JDx12Utility.h"

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
				macro.push_back({ L"STATIC", L"1" });
				break;
			}
			case JinEngine::Core::J_MESHGEOMETRY_TYPE::SKINNED:
			{
				macro.push_back({ L"SKINNED", L"2" });
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
				macro.push_back({ L"NORMALSM", L"1" });
				break;
			}
			case JinEngine::J_SHADOW_MAP_TYPE::CSM:
			{
				macro.push_back({ L"ARRAY", L"2" });
				macro.push_back({ L"ARRAY_COUNT", std::to_wstring(JCsmOption::maxCountOfSplit) });
				break;
			}
			case JinEngine::J_SHADOW_MAP_TYPE::CUBE:
			{
				macro.push_back({ L"CUBE", L"3" });
				break;
			}
			default:
				break;
			}
		}
		static void StuffCsmMacro(std::vector<JMacroSet>& macro, const uint count)noexcept
		{
			macro.push_back({ L"ARRAY", L"2" });
			macro.push_back({ L"ARRAY_COUNT", std::to_wstring(count) });
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

	JDx12ShadowMap::~JDx12ShadowMap()
	{
		ClearResource();
	}
	void JDx12ShadowMap::Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)
	{
		if (!IsSameDevice(device) || !IsSameDevice(gM))
			return;
 
		BuildResource(device, gM, GetGraphicInfo());
	}
	void JDx12ShadowMap::Clear()
	{
		ClearResource();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12ShadowMap::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	JDx12ShadowMap::JDx12GraphicShaderDataHolder* JDx12ShadowMap::GetShaderDataHolder(const JDrawHelper& helper, const Core::J_MESHGEOMETRY_TYPE mType)
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
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12BindSet->context);
		context->SetGraphicsRootSignature(mRootSignature.Get());
	}
	void JDx12ShadowMap::BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12BindSet->context);

		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const int offset = gRInterface.GetResourceDataIndex(grType, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		const uint smDataCount = gRInterface.GetDataCount(grType); 	 
		for (uint i = 0; i < smDataCount; ++i)
		{ 
			auto shadowSet = context->ComputeSet(gRInterface, grType, offset + i); 
			context->Transition(shadowSet.holder, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
			context->ClearDepthStencilView(shadowSet);
		}
	}
	void JDx12ShadowMap::EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(bindSet))
			return;

		const JDx12GraphicBindSet* dx12BindSet = static_cast<const JDx12GraphicBindSet*>(bindSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12BindSet->context);

		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const int offset = gRInterface.GetResourceDataIndex(grType, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		const uint smDataCount = gRInterface.GetDataCount(grType);
		for (uint i = 0; i < smDataCount; ++i)
		{
			auto shadowSet = context->ComputeSet(gRInterface, grType, offset + i);
			context->Transition(shadowSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		} 
	}
	void JDx12ShadowMap::DrawSceneShadowMap(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(shadowDrawSet) || !helper.allowDrawShadowMap)
			return;

		const JDx12GraphicShadowMapDrawSet* dx12SmDrawSet = static_cast<const JDx12GraphicShadowMapDrawSet*>(shadowDrawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12SmDrawSet->context);

		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const int offset = gRInterface.GetResourceDataIndex(grType, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		const uint smDataCount = gRInterface.GetDataCount(grType); 
		for (uint i = 0; i < smDataCount; ++i)
		{
			const uint dataIndex = offset + i; 
			auto shadowSet = context->ComputeSet(gRInterface, grType, offset + i);
			const JVector2<uint> size = shadowSet.info->GetResourceSize();

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)size.x, (float)size.y, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, size.x, size.y };

			context->SetGraphicsRootSignature(mRootSignature.Get());
			context->SetViewport(mViewport);
			context->SetScissorRect(mScissorRect); 
			context->Transition(shadowSet.holder, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
			context->ClearDepthStencilView(shadowSet);
			context->SetDepthStencilView(shadowSet);
			 
			BindLightFrameResource(context, helper, i);

			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

			DrawShadowMapGameObject(context, objVec00, helper, JDrawCondition(helper, false, true, false), i);
			DrawShadowMapGameObject(context, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, false), i);
			//if (helper.allowOcclusionCulling && helper.lit->AllowHdOcclusionCulling())
			//	cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);

			//scene draw전에 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE상태로
			//전환시킬 필요
			//context End에서 수행 불가(single thread에 darw process는 하나의 context(Begin)로 update하므로...)
			context->Transition(shadowSet.holder, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		}
	}
	void JDx12ShadowMap::DrawSceneShadowMapMultiThread(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper)
	{
		if (!IsSameDevice(shadowDrawSet) || !helper.allowDrawShadowMap)
			return;

		const JDx12GraphicShadowMapDrawSet* dx12SmDrawSet = static_cast<const JDx12GraphicShadowMapDrawSet*>(shadowDrawSet);
		JDx12CommandContext* context = static_cast<JDx12CommandContext*>(dx12SmDrawSet->context);

		//BindRootSignature(cmdList);
		auto gRInterface = helper.lit->GraphicResourceUserInterface();
		const J_GRAPHIC_RESOURCE_TYPE grType = JLightType::SmToGraphicR(helper.lit->GetShadowMapType()); 

		const int offset = gRInterface.GetResourceDataIndex(grType, J_GRAPHIC_TASK_TYPE::SHADOW_MAP_DRAW);
		const uint smDataCount = gRInterface.GetDataCount(grType); 
		for (uint i = 0; i < smDataCount; ++i)
		{
			const uint dataIndex = offset + i;
			auto shadowSet = context->ComputeSet(gRInterface, grType, offset + i);
			const JVector2<uint> size = shadowSet.info->GetResourceSize();

			D3D12_VIEWPORT mViewport = { 0.0f, 0.0f,(float)size.x, (float)size.y, 0.0f, 1.0f };
			D3D12_RECT mScissorRect = { 0, 0, size.x, size.y };

			context->SetGraphicsRootSignature(mRootSignature.Get());
			context->SetViewport(mViewport);
			context->SetScissorRect(mScissorRect);
			context->SetDepthStencilView(shadowSet);
			BindLightFrameResource(context, helper, i); 

			const std::vector<JUserPtr<JGameObject>>& objVec00 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::STATIC);
			const std::vector<JUserPtr<JGameObject>>& objVec01 = helper.GetGameObjectCashVec(J_RENDER_LAYER::OPAQUE_OBJECT, Core::J_MESHGEOMETRY_TYPE::SKINNED);

			DrawShadowMapGameObject(context, objVec00, helper, JDrawCondition(helper, false, true, false), i);
			DrawShadowMapGameObject(context, objVec01, helper, JDrawCondition(helper, helper.scene->IsActivatedSceneTime(), true, false), i);
			//if (helper.allowOcclusionCulling && helper.lit->AllowHdOcclusionCulling())
			//	cmdList->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
		}
	} 
	void JDx12ShadowMap::DrawShadowMapGameObject(JDx12CommandContext* context,
		const std::vector<JUserPtr<JGameObject>>& gameObject,
		const JDrawHelper& helper,
		const JDrawCondition& condition,
		const uint dataIndex)
	{
		const uint gameObjCount = (uint)gameObject.size();
		uint st, ed = 0;
		helper.DispatchWorkIndex(gameObject.size(), st, ed);

		uint smIndex = (uint)helper.lit->GetShadowMapType();
		auto cullUser = helper.GetCullInterface();
		for (uint i = st; i < ed; ++i)
		{
			auto renderItem = gameObject[i]->GetRenderItem();
			const uint objFrameIndex = helper.GetObjectFrameIndex(renderItem.Get());
			const uint boundFrameIndex = helper.GetBoundingFrameIndex(renderItem.Get());

			//share same inedx culling and shadow
			if (condition.allowCulling && !renderItem->IsIgnoreCullingResult() && cullUser.IsCulled(J_CULLING_TYPE::FRUSTUM, J_CULLING_TARGET::RENDERITEM,boundFrameIndex))
				continue;

			JUserPtr<JMeshGeometry> mesh = renderItem->GetMesh();	 
			JUserPtr<JAnimator> animator = gameObject[i]->GetComponentWithParent<JAnimator>();

			const bool onSkinned = animator != nullptr && condition.allowAnimation;
			const Core::J_MESHGEOMETRY_TYPE meshType = onSkinned ? Core::J_MESHGEOMETRY_TYPE::SKINNED : Core::J_MESHGEOMETRY_TYPE::STATIC;
			
			context->SetMeshGeometryData(renderItem);
			context->SetPipelineState(GetShaderDataHolder(helper, meshType));
			if (onSkinned)
				context->SetGraphicsRootConstantBufferView(aniCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, helper.GetAnimationFrameIndex(animator.Get()));
			
			const uint submeshCount = (uint)mesh->GetTotalSubmeshCount();
			for (uint j = 0; j < submeshCount; ++j)
			{
				context->SetGraphicsRootConstantBufferView(objCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::OBJECT, objFrameIndex + j);
				context->DrawIndexedInstanced(mesh, j);
			}
		}
	}
	void JDx12ShadowMap::BindLightFrameResource(JDx12CommandContext* context, const JDrawHelper& helper, const int offset)
	{
		J_SHADOW_MAP_TYPE smType = helper.lit->GetShadowMapType();
		if (smType == J_SHADOW_MAP_TYPE::NONE)
			return;
		 
		const int frameIndex = helper.GetLitShadowFrameIndex() + offset;
		if (smType == J_SHADOW_MAP_TYPE::NORMAL)
			context->SetGraphicsRootConstantBufferView(normalShadowMapDrawCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, frameIndex);
		else if (smType == J_SHADOW_MAP_TYPE::CSM)
			context->SetGraphicsRootConstantBufferView(csmDrawCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW, frameIndex);
		else if (smType == J_SHADOW_MAP_TYPE::CUBE)
			context->SetGraphicsRootConstantBufferView(cubeShadowMapDrawCBIndex, J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_CUBE_DRAW, frameIndex);
	}
	void JDx12ShadowMap::BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)
	{
		ID3D12Device* d3d12Device = static_cast<JDx12GraphicDevice*>(device)->GetDevice();
		DXGI_FORMAT dsvFormat = Constants::GetDepthStencilFormat();
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
	void JDx12ShadowMap::BuildRootSignature(ID3D12Device* device)
	{
		JDx12RootSignatureBuilder<slotCount> builder; 
		builder.PushConstantsBuffer(objCBIndex);
		builder.PushConstantsBuffer(aniCBIndex);
		builder.PushConstantsBuffer(normalShadowMapDrawCBIndex);
		builder.PushConstantsBuffer(csmDrawCBIndex);
		builder.PushConstantsBuffer(cubeShadowMapDrawCBIndex);
		builder.Create(device, L"ShadowMapDraw RootSignature", mRootSignature.GetAddressOf(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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
		data.vs = JDxShaderDataUtil::CompileShader(gShaderPath, macroSet, L"VS", L"vs_6_0");
		if (CanUseGs(smType))
			data.gs = JDxShaderDataUtil::CompileShader(gShaderPath, macroSet, L"GS", L"gs_6_0");

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
		ThrowIfFailedG(device->CreateGraphicsPipelineState(&newShaderPso, IID_PPV_ARGS(data.GetPsoAddress())));
	}
	void JDx12ShadowMap::ClearResource()
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
}