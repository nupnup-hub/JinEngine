#include"JDirectionalLight.h"   
#include"JDirectionalLightPrivate.h"
#include"JLightConstants.h"
#include"../Camera/JCameraConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Geometry/JDirectXCollisionEx.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/JGraphic.h"  
#include"../../../Graphic/Frameresource/JLightConstants.h"  
#include"../../../Graphic/Frameresource/JShadowMapConstants.h"  
#include"../../../Graphic/Frameresource/JOcclusionConstants.h"
#include"../../../Graphic/Frameresource/JDepthTestConstants.h" 
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/JGraphicDrawListInterface.h" 
#include"../../../Graphic/ShadowMap/JCsmHandlerInterface.h"  
#include<Windows.h>
#include<fstream>
   
using namespace DirectX;
namespace JinEngine
{
	namespace
	{
		using LitFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder6 <
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT, Graphic::JDirectionalLightConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO, Graphic::JCsmConstants&, const uint>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW, Graphic::JShadowMapArrayDrawConstants&, const uint>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW, Graphic::JShadowMapDrawConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, Graphic::JDepthTestPassConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, Graphic::JHzbOccComputeConstants&, const uint, const uint>>,
			Graphic::JFrameDirty>;
	
		static constexpr uint multiResourceCount = 2;	//shadow map(csm(multi) + ssm(single)) + debug
		static constexpr uint singleResourceCount = 3;	// depth + mip + debug
		 
		using JGraphicWideSingleOneMultiResourceHolder = Graphic::JGraphicWideSingleOneMultiResourceHolder<singleResourceCount, true,
			Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, 
			Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG>;
	}
	namespace
	{ 
		static auto isAvailableoverlapLam = []() {return true; };
		static JDirectionalLightPrivate lPrivate;

		static XMVECTOR GetInitDir()
		{
			return XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		}
		static XMVECTOR CalLightWorldDir(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMVector3Normalize(XMVector3Rotate(GetInitDir(), transform->GetWorldQuaternion().ToXmV()));
		}
		static XMVECTOR CalLightWorldPos(const JUserPtr<JTransform>& transform) noexcept
		{ 
			return XMVectorScale(CalLightWorldDir(transform), -(Constants::lightMaxDistance * 0.5f));
		}
		static XMMATRIX CalView(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMMatrixLookAtLH(CalLightWorldPos(transform),
				XMVectorSet(0, 0, 0, 1),
				XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
		}
		static XMMATRIX CalProj(const BoundingBox& sceneBBox,
			const JMatrix4x4& view,
			const size_t mapSize,
			_Out_ JVector3<float>& outMin,
			_Out_ JVector3<float>& outMax)noexcept
		{
			static constexpr bool isFitToScene = true;
			static constexpr bool canMoveLightTexelSize = true;

			const XMMATRIX viewM = view.LoadXM();

			XMFLOAT3 corner[8];
			sceneBBox.GetCorners(corner);

			XMVECTOR minV = JVector3<float>::PositiveInfV().ToXmV();
			XMVECTOR maxV = JVector3<float>::NegativeInfV().ToXmV();

			for (int i = 0; i < 8; ++i)
			{
				const XMVECTOR v = XMVector3Transform(XMLoadFloat3(&corner[i]), viewM);
				minV = XMVectorMin(minV, v);
				maxV = XMVectorMax(maxV, v);
			}
			 
			XMVECTOR worldUnitsPerTexel = JVector4<float>::Zero().ToXmV();
			XMVECTOR multiplySetzwToZeroV = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);
			if (isFitToScene)
			{
				XMVECTOR vDiagonal = XMLoadFloat3(&corner[0]) - XMLoadFloat3(&corner[6]);
				vDiagonal = XMVector3Length(vDiagonal);

				XMVECTOR vBoarderOffset = (vDiagonal - (maxV - minV)) * XMVectorSet(0.5f, 0.5f, 0.5f, 0.5f);
				vBoarderOffset *= multiplySetzwToZeroV;

				maxV += vBoarderOffset;
				minV -= vBoarderOffset;

				FLOAT fCascadeBound = XMVectorGetX(vDiagonal);	 
				FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)mapSize;
				worldUnitsPerTexel = XMVectorSet(fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f);
			}
			 
			float minZ = XMVectorGetZ(minV);
			float maxZ = XMVectorGetZ(maxV);

			if (canMoveLightTexelSize)
			{ 
				minV /= worldUnitsPerTexel;
				minV = XMVectorFloor(minV);
				minV *= worldUnitsPerTexel;

				maxV /= worldUnitsPerTexel;
				maxV = XMVectorFloor(maxV);
				maxV *= worldUnitsPerTexel;
			}

			outMin = minV;
			outMax = maxV;
			outMin.z = minZ;
			outMax.z = maxZ;
			return XMMatrixOrthographicOffCenterLH(XMVectorGetX(minV), XMVectorGetX(maxV),
				XMVectorGetY(minV), XMVectorGetY(maxV),
				minZ, maxZ);
		}
		static XMMATRIX CalProj(const BoundingBox& sceneBBox,
			const JMatrix4x4& view,
			const size_t mapSize)noexcept
		{
			JVector3<float> minV3;
			JVector3<float> maxV3;
			return CalProj(sceneBBox, view, mapSize, minV3, maxV3);
		}
	}

	class JDirectionalLight::JDirectionalLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public JGraphicWideSingleOneMultiResourceHolder,
		//public Graphic::JGraphicMultiResourceHolder,
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCullingInterface,
		public Graphic::JCsmHandlerInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JDirectionalLightImpl)
	public:
		using DirLitFrame = JFrameInterface1;
		using CsmInfoFrame = JFrameInterface2;
		using CsmDrawFrame = JFrameInterface3;
		using ShadowMapNormalDrawFrame = JFrameInterface4;
		using DepthTestFrame = JFrameInterface5;
		using HzbOccComputeFrame = JFrameInterface6; 
	public:
		JWeakPtr<JDirectionalLight> thisPointer;
	public:
		JVector3<float> vSceneBBoxMinF;		 
		JVector3<float> vSceneBBoxMaxF;
		JMatrix4x4 view;
		JMatrix4x4 proj;
	public:
		//REGISTER_PROPERTY_EX(onCsm, IsCsmActivated, SetCsm, GUI_CHECKBOX(GUI_BOOL_CONDITION_REF_USER(OnShadow, true)))
		REGISTER_GUI_BOOL_CONDITION(OnShadow, IsShadowActivated, true)
		REGISTER_GUI_BOOL_CONDITION(OnCsm, onCsm, false)
		REGISTER_PROPERTY_EX(onCsm, IsCsmActivated, SetCsm, GUI_CHECKBOX(GUI_BOOL_CONDITION_USER(OnShadow, true)))
		bool onCsm = false;
	public:  
		bool allowHzbOcclusionCulling = false;
		bool allowHdOcclusionCulling = false;
		bool allowDisplayOccCullingDepthMap = false;
	public:
		JDirectionalLightImpl(const InitData& initData, JDirectionalLight* thisLitRaw)
			:Graphic::JCsmHandlerInterface()
		{ 
		}
		~JDirectionalLightImpl()
		{ }
	public:
		J_LIGHT_TYPE GetLightType()const noexcept
		{
			return J_LIGHT_TYPE::DIRECTIONAL;
		} 
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept
		{
			if (!thisPointer->IsShadowActivated())
				return J_SHADOW_MAP_TYPE::NONE;

			if (CanAllocateCsm())
				return J_SHADOW_MAP_TYPE::CSM; 
			else
				return J_SHADOW_MAP_TYPE::NORMAL;
		} 
		float GetFrustumNear()const noexcept
		{
			JVector3<float> vMinF;
			JVector3<float> vMaxF;
			const XMMATRIX proj = CalProj(GetSceneBBox(), view, thisPointer->GetShadowResolution(), vMinF, vMaxF);
			return vMinF.z;
		}
		float GetFrustumFar()const noexcept
		{
			JVector3<float> vMinF;
			JVector3<float> vMaxF;
			const XMMATRIX proj = CalProj(GetSceneBBox(), view, thisPointer->GetShadowResolution(), vMinF, vMaxF);
			return vMaxF.z;
		}
		DirectX::BoundingBox GetBBox()const noexcept
		{
			const XMMATRIX viewM = view.LoadXM();
			const XMMATRIX inverseViewM = XMMatrixInverse(nullptr, viewM);

			XMFLOAT3 corner[8];
			GetSceneBBox().GetCorners(corner);

			XMVECTOR minV = JVector3<float>::PositiveInfV().ToXmV();
			XMVECTOR maxV = JVector3<float>::NegativeInfV().ToXmV();

			for (int i = 0; i < 8; ++i)
			{
				const XMVECTOR v = XMVector3Transform(XMLoadFloat3(&corner[i]), viewM);
				minV = XMVectorMin(minV, v);
				maxV = XMVectorMax(maxV, v);
			} 

			//world 
			return Core::JDirectXCollisionEx::CreateBoundingBox(XMVector3Transform(minV, inverseViewM), XMVector3Transform(maxV, inverseViewM));
		}
		JUserPtr<JTransform> GetTransform()const noexcept
		{
			return thisPointer->GetOwner()->GetTransform();
		} 
		size_t GetCsmHandlerGuid()const noexcept final
		{
			return thisPointer->GetGuid();
		}
		size_t GetCsmAreaGuid()const noexcept final
		{
			return thisPointer->GetOwner()->GetOwnerGuid();
		}
		BoundingBox GetSceneBBox()const noexcept
		{
			return thisPointer->GetOwner()->GetOwnerScene()->GetSceneBBox();
		}
		XMMATRIX GetShadowMapTransform()const noexcept
		{
			return XMMatrixMultiply(XMMatrixMultiply(view.LoadXM(), proj.LoadXM()), JMatrix4x4::NdcToTextureSpaceXM());
		}
		uint GetSplitCount()const noexcept
		{
			return GetCsmOptionRef().GetSplitCount();
		}
		float GetSplitBlendRate()const noexcept
		{
			return GetCsmOptionRef().GetSplitBlendRate();
		}
		float GetShadowDistance()const noexcept
		{
			return GetCsmOptionRef().GetShadowDistance();
		}
		float GetLevelBlendRate()const noexcept
		{
			return GetCsmOptionRef().GetLevelBlendRate();
		}
	public:  
		void SetShadow(const bool value, const bool isManual = false)noexcept
		{
			if (thisPointer->IsActivated() || isManual)
			{
				if (value)
					CreateShadowMapResource(CanAllocateCsm());
				else
					DestroyShadowMapResource();
			}
			SetFrameDirty();
		} 
		void SetShadowResolution(const J_SHADOW_RESOLUTION newShadowResolution)noexcept
		{  
			if (thisPointer->IsActivated() && thisPointer->IsShadowActivated())
			{
				DestroyShadowMapResource();
				CreateShadowMapResource(CanAllocateCsm());
			}
			SetFrameDirty();
		}
		void SetCsm(const bool value)noexcept
		{
			if (onCsm == value)
				return;

			if (thisPointer->IsActivated())
			{
				if(IsShadowActivated())
					DestroyShadowMapResource();
				if (onCsm)
					DeRegisterCsmHandlerface();
			}
			onCsm = value;
			if (thisPointer->IsActivated())
			{
				if (onCsm)
					RegisterCsmHandlerface();
				if (IsShadowActivated())
					CreateShadowMapResource(CanAllocateCsm());
			}
			SetFrameDirty();
		}
		void SetCsmManual(const bool value)noexcept
		{
			if (IsShadowActivated())
				DestroyShadowMapResource();
			if (onCsm)
				DeRegisterCsmHandlerface();
			if (value)
			{
				if (onCsm)
					RegisterCsmHandlerface();
				if (IsShadowActivated())
					CreateShadowMapResource(CanAllocateCsm());
			}
			SetFrameDirty();
		}
		void SetAllowDisplayShadowMap(const bool value)
		{
			SetAllowDisplayShadowMapEx(value, false);
		}
		void SetAllowDisplayShadowMapEx(const bool value, const bool allowManualControll = false)
		{ 
			if (allowManualControll || thisPointer->IsActivated())
			{
				if (value)
					CreateShadowMapDebugResource(CanAllocateCsm());
				else
					DestroyShadowMapDebugResource();
			}
			SetFrameDirty();
		} 
		void SetAllowHzbOcclusionCulling(const bool value, const bool allowManualControll = false)noexcept
		{ 
			if (!allowManualControll && allowHzbOcclusionCulling == value)
				return;

			if (AllowHdOcclusionCulling())
				SetAllowHdOcclusionCulling(false);

			if (!allowManualControll)
				allowHzbOcclusionCulling = value;
			if (allowManualControll || thisPointer->IsActivated())
			{
				if (value)
				{
					RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS);
					RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS);
					CreateHzbOcclusionResource();
					CreateHzbOccCullingData();
					AddHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMap(true, true);
				}
				else
				{
					DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS);
					DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS);
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
					DestroyCullingData(Graphic::J_CULLING_TYPE::HZB_OCCLUSION);
					PopHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMap(false, true);
				}
			}
			SetFrameDirty();
		}
		void SetAllowHdOcclusionCulling(const bool value, const bool allowManualControll = false)noexcept
		{ 
			if (!allowManualControll && allowHdOcclusionCulling == value)
				return;

			if (AllowHzbOcclusionCulling())
				SetAllowHzbOcclusionCulling(false);

			if (!allowManualControll)
				allowHdOcclusionCulling = value;
			if (allowManualControll || thisPointer->IsActivated())
			{
				if (value)
				{
					RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS); 
					CreateHdOccCullingData();
					AddHdOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMap(true, true);
				}
				else
				{
					DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS); 
					DestroyCullingData(Graphic::J_CULLING_TYPE::HD_OCCLUSION);
					PopHdOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMap(false, true);
				}
			}
			SetFrameDirty();
		}
		void SetAllowDisplayOccCullingDepthMap(const bool value, const bool allowManualControll = false)noexcept
		{
			if (!allowManualControll && allowDisplayOccCullingDepthMap == value)
				return;

			if (!allowManualControll)
				allowDisplayOccCullingDepthMap = value;
			if (allowManualControll || thisPointer->IsActivated())
			{
				if (value)
					CreateOcclusionDepthDebug(AllowHzbOcclusionCulling());
				else
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
			}
			SetFrameDirty();
		}
		void SetSplitCount(const uint newCount)noexcept
		{ 
			GetCsmOptionRef().SetSplitCount(newCount); 
			if (CanAllocateCsm())
			{
				DestroyShadowMapResource();
				CreateShadowMapResource(CanAllocateCsm());
			}
			SetFrameDirty();
		}
		void SetSplitBlendRate(const float value)noexcept
		{
			GetCsmOptionRef().SetSplitBlendRate(value);
			SetFrameDirty();
		}
		void SetShadowDistance(const float value)noexcept
		{
			GetCsmOptionRef().SetShadowDistance(std::clamp(value, Constants::minCamFrustumNear, Constants::maxCamFrustumFar));
			SetFrameDirty();
		}
		void SetLevelBlendRate(const float value)noexcept
		{
			GetCsmOptionRef().SetLevelBlendRate(value);
			SetFrameDirty();
		}
	public:
		REGISTER_METHOD_GUI_WIDGET(CsmSplitCount, GetSplitCount, SetSplitCount, GUI_SLIDER(Graphic::JCsmOption::minCountOfSplit, Graphic::JCsmOption::maxCountOfSplit, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
		REGISTER_METHOD_GUI_WIDGET(CsmSplitBlendRate, GetSplitBlendRate, SetSplitBlendRate, GUI_SLIDER(Graphic::JCsmOption::minSplitRate, Graphic::JCsmOption::maxSplitRate, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
		REGISTER_METHOD_GUI_WIDGET(CsmShadowDistance, GetShadowDistance, SetShadowDistance, GUI_SLIDER(Constants::minCamFrustumNear, Constants::maxCamFrustumFar, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
		REGISTER_METHOD_GUI_WIDGET(CsmLevelBlendRate, GetLevelBlendRate, SetLevelBlendRate, GUI_SLIDER(Graphic::JCsmOption::minLevelRate, Graphic::JCsmOption::maxLevelRate, true, false, 3, GUI_BOOL_CONDITION_USER(OnCsm, true)))
	public:
		REGISTER_METHOD(IsShadowActivated)
		bool IsShadowActivated()const noexcept
		{
			return thisPointer->IsShadowActivated();
		}
		//return trigger value
		bool IsCsmActivated()const noexcept
		{
			return onCsm;
		}
		//return lit type is dirctional light
		bool CanUseCsm()const noexcept
		{
			return GetLightType() == J_LIGHT_TYPE::DIRECTIONAL;
		}
		//return IsCsmActivated() && CanUseCsm()
		bool CanAllocateCsm()const noexcept
		{
			return IsCsmActivated() && CanUseCsm() && GetCsmTargetCount() > 0;
		}  
		bool AllowHzbOcclusionCulling()const noexcept
		{
			return allowHzbOcclusionCulling;
		}
		bool AllowHdOcclusionCulling()const noexcept
		{
			return allowHdOcclusionCulling;
		}
		bool AllowDisplayOccCullingDepthMap()const noexcept
		{
			return allowDisplayOccCullingDepthMap;
		}
	public:
		void CreateShadowMapResource(const bool canAllocCsm)noexcept
		{
			if (canAllocCsm)
			{
				//has order dependency 
				//1. RegisterCsmHandlerface();
				//2. RegisterCsmFrameData
				RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO);
				const uint targetCount = GetCsmTargetCount();
				for (uint i = 0; i < targetCount; ++i)
					CreateShadowMapTextureArray(thisPointer->GetShadowMapSize(), GetCsmOptionRef().GetSplitCount());
			} 
			else
				CreateShadowMapTexture(thisPointer->GetShadowMapSize());

			if (thisPointer->AllowDisplayShadowMap())
				CreateShadowMapDebugResource(canAllocCsm);

			RegisterLightFrameData(JLightType::SmToFrameR(GetLightType(), canAllocCsm));
			AddDrawShadowRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer); 
		}
		void DestroyShadowMapResource()noexcept
		{
			//DeRegisterCsmHandlerface();
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO);
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW);
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW); 

			DestroyShadowMapDebugResource();
			DestroyAllTexture();

			DestroyCullingData(Graphic::J_CULLING_TYPE::FRUSTUM);
			PopDrawRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer); 
		};
		void CreateShadowMapDebugResource(const bool canAllocCsm)
		{ 
			uint count = 1;
			if (canAllocCsm)
				count = GetCsmTargetCount(); 
			 
			for (uint i = 0; i < count; ++i)
				CreateLayerDepthDebugResource(JVector2<uint>(thisPointer->GetShadowMapSize(), thisPointer->GetShadowMapSize()));
		}
		void DestroyShadowMapDebugResource()
		{
			DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG);
		}
	public:
		void Activate()
		{
			if (AllowHdOcclusionCulling() && AllowHzbOcclusionCulling())
				allowHdOcclusionCulling = allowHzbOcclusionCulling = false;
 
			RegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (onCsm)
				SetCsmManual(true);
			if (thisPointer->IsShadowActivated())
				SetShadow(true, true);
			if (allowHzbOcclusionCulling)
				SetAllowHzbOcclusionCulling(true, true);
			if (allowHdOcclusionCulling)
				SetAllowHdOcclusionCulling(true, true);
		}
		void DeActivate()
		{ 
			//has order dependency
			DeRegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (onCsm)
				SetCsmManual(false);
			if (thisPointer->IsShadowActivated())
				SetShadow(false, true);
			if (allowHzbOcclusionCulling)
				SetAllowHzbOcclusionCulling(false, true);
			if (allowHdOcclusionCulling)
				SetAllowHdOcclusionCulling(false, true);
			DestroyAllCullingData();
			DestroyAllTexture();
		}
	public:
		void UpdateFrame(Graphic::JDirectionalLightConstants& constant) noexcept final
		{ 
			const XMMATRIX viewM = view.LoadXM();
			const XMMATRIX projM = proj.LoadXM(); 

			constant.view.StoreXM(XMMatrixTranspose(viewM));
			constant.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM, projM)));
			if (CanAllocateCsm())
			{
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(GetShadowMapTransform()));
				//constant.shadowMapTransform.StoreXM(XMMatrixTranspose(viewM));
				constant.shadowMapIndex = GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, 0);
				constant.csmDataIndex = CsmInfoFrame::GetFrameIndex();
			}
			else if (thisPointer->IsShadowActivated())
			{ 
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(GetShadowMapTransform()));
				constant.shadowMapIndex = GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0);
			}  
		 
			constant.shadowMapType = (uint)GetShadowMapType();
			constant.color = thisPointer->GetColor();
			constant.direction = CalLightWorldDir(GetTransform());
			constant.frustumSize = JVector2F(abs(vSceneBBoxMaxF.x - vSceneBBoxMinF.x), abs(vSceneBBoxMaxF.y - vSceneBBoxMinF.y));
			constant.frustumNear = vSceneBBoxMinF.z;
			constant.frustumFar = vSceneBBoxMaxF.z;
			constant.penumbraScale = thisPointer->GetPenumbraWidth();
			constant.penumbraBlockerScale = thisPointer->GetPenumbraBlockerWidth();
			constant.shadowMapSize = thisPointer->GetShadowMapSize();
			constant.shadowMapInvSize = 1.0f / constant.shadowMapSize;
			constant.tanAngle = XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(constant.direction.ToXmV(), GetInitDir()));
			constant.bias = thisPointer->GetBias();
			DirLitFrame::MinusMovedDirty();  	 
		}
		void UpdateFrame(Graphic::JCsmConstants& constant, const uint index)noexcept final
		{ 
			const auto& result = GetCsmComputeResult(index);
			for (uint i = 0; i < result.subFrustumCount; ++i)
			{ 
				constant.scale[i] = result.scale[i];
				constant.posOffset[i] = result.posOffset[i];
				constant.frustumNear[i] = result.fNear[i];
				constant.frustumFar[i] = result.fFar[i];
			} 
			constant.mapMinBorder = (float)(1.0f / (float)thisPointer->GetShadowMapSize());
			constant.mapMaxBorder = (float)(((float)thisPointer->GetShadowMapSize() - 1.0f) / (float)thisPointer->GetShadowMapSize());
			constant.levelBlendRate = GetCsmOptionRef().GetLevelBlendRate();
			constant.count = result.subFrustumCount; 
			CsmInfoFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapArrayDrawConstants& constant, const uint index)noexcept final
		{
			const auto& result = GetCsmComputeResult(index);
			const XMMATRIX viewM = view.LoadXM(); 
			for (uint i = 0; i < result.subFrustumCount; ++i)
			{
				constant.shadowMapTransform[i].StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM,
					result.shadowProjM[i].LoadXM()))); 
			}
			CsmDrawFrame::MinusMovedDirty();
		}	
		void UpdateFrame(Graphic::JShadowMapDrawConstants& constant)noexcept final
		{
			//for draw shadow map
			constant.shadowMapTransform.StoreXM(XMMatrixTranspose(XMMatrixMultiply(view.LoadXM(), proj.LoadXM())));
			ShadowMapNormalDrawFrame::MinusMovedDirty(); 
		}
		void UpdateFrame(Graphic::JDepthTestPassConstants& constants)noexcept final
		{
			//for draw depth map
			constants.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(view.LoadXM(), proj.LoadXM())));
			DepthTestFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JHzbOccComputeConstants& constant, const uint queryCount, const uint queryOffset)noexcept final
		{
			//only directional light  can execute occ
			auto info = JGraphic::Instance().GetGraphicInfo();
			auto option = JGraphic::Instance().GetGraphicOption();
			 
			const XMMATRIX viewM = view.LoadXM();
			const XMMATRIX projM = proj.LoadXM();

			constant.view.StoreXM(XMMatrixTranspose(viewM));
			constant.proj.StoreXM(XMMatrixTranspose(projM));
			constant.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewM, projM)));
		 
			constant.viewWidth = vSceneBBoxMaxF.x - vSceneBBoxMinF.x;
			constant.viewHeight = vSceneBBoxMaxF.y - vSceneBBoxMinF.y;
			constant.camNear = vSceneBBoxMinF.z;
			constant.camFar = vSceneBBoxMaxF.z;
			constant.validQueryCount = queryCount;
			constant.validQueryOffset = queryOffset;
			constant.occMapCount = info.occlusionMapCount;
			constant.occIndexOffset = JMathHelper::Log2Int(info.occlusionMinSize);
			constant.correctFailTrigger = (int)option.allowHZBCorrectFail;
			constant.usePerspective = false;	//use ortho
			HzbOccComputeFrame::MinusMovedDirty();		 
		}
		void UpdateLightTransform()noexcept
		{ 
			const XMMATRIX viewM = CalView(GetTransform());
			view.StoreXM(viewM);
			proj.StoreXM(CalProj(GetSceneBBox(), view, thisPointer->GetShadowResolution(), vSceneBBoxMinF, vSceneBBoxMaxF));
			if (CanAllocateCsm())
				CsmUpdate(viewM, GetSceneBBox(), thisPointer->GetShadowMapSize());		 
		}
		void UpdateCsmTargetCount(const uint index, const bool isRegister)
		{
			//can alloc csm = true
			const int smArrayCount = GetDataCount(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY);
			const int targetCount = GetCsmTargetCount();
			const bool isSameCount = smArrayCount == targetCount;

			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW);
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO);
			CsmDrawFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW, (CsmDrawFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), GetCsmTargetCount());
			CsmInfoFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO, (CsmInfoFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), GetCsmTargetCount());

			if (isRegister)
			{
				if (!isSameCount)
				{
					if (smArrayCount == 0 && targetCount == 1)
					{
						DestroyShadowMapResource();
						CreateShadowMapResource(CanAllocateCsm());
					}
					else
					{
						CreateShadowMapTextureArray(thisPointer->GetShadowMapSize(), GetCsmOption().GetSplitCount());
						if (thisPointer->AllowDisplayShadowMap())
							CreateLayerDepthDebugResource(JVector2<uint>(thisPointer->GetShadowMapSize(), thisPointer->GetShadowMapSize()));
					}
				}
			}
			else
			{
				CsmInfoFrame::SetMovedDirty();
				CsmDrawFrame::SetMovedDirty();
				if (!isSameCount)
				{
					DestroyMultiTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, index);
					if (thisPointer->AllowDisplayShadowMap())
						DestroyMultiTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, index);
				}
			}
			SetFrameDirty();
		}
	public:
		static bool DoCopy(JDirectionalLight* from, JDirectionalLight* to)
		{
			from->SetCsm(to->IsCsmActivated());   
			from->impl->SetAllowHzbOcclusionCulling(to->impl->AllowHzbOcclusionCulling());
			from->impl->SetAllowHdOcclusionCulling(to->impl->AllowHdOcclusionCulling());
			from->impl->SetAllowDisplayOccCullingDepthMap(to->impl->AllowDisplayOccCullingDepthMap());

			to->impl->SetFrameDirty();
			return true;
		}
	public:
		void NotifyReAlloc()
		{ 
			if (thisPointer.IsValid())
			{
				JLightPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
				JLightPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
			}
			RegisterInterfacePointer();

			JFrameUpdateData::ReRegisterFrameData(JLightType::LitToFrameR(GetLightType()), CastLightFrameInteface());
			JFrameUpdateData::ReRegisterFrameData(JLightType::SmToFrameR(GetLightType(), CanAllocateCsm()), CastShadowMapFrameInteface());
			if (CanAllocateCsm())
				JFrameUpdateData::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO, (CsmInfoFrame*)this);
			JFrameUpdateData::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, (DepthTestFrame*)this);
			JFrameUpdateData::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, (HzbOccComputeFrame*)this);
		}
		void NotifyAddCsmTarget(const uint index)final
		{
			UpdateCsmTargetCount(index, true);
		}
		void NotifyPopCsmTarget(const uint index)final
		{
			UpdateCsmTargetCount(index, false);
		}
		void NotifyCsmTargetZero()final
		{
			SetCsm(false);
		}
	public:
		//util
		JFrameUpdateData* CastLightFrameInteface()
		{
			return (DirLitFrame*)this;
		}
		JFrameUpdateData* CastShadowMapFrameInteface()
		{
			if (!thisPointer->IsShadowActivated())
				return nullptr;

			if (CanAllocateCsm())
				return (CsmDrawFrame*)this;
			else
				return (ShadowMapNormalDrawFrame*)this;
		}
		JFrameUpdateData* CastFrameInterface(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			switch (type)
			{
			case Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT:
				return (DirLitFrame*)this;
			case Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO:
				return (CsmInfoFrame*)this;
			case Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW:
				return (CsmDrawFrame*)this;
			case Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW:
				return (ShadowMapNormalDrawFrame*)this;
			case Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS:
				return (DepthTestFrame*)this;
			case Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS:
				return (HzbOccComputeFrame*)this;
			default:
				break;
			}
			return nullptr;
		}
	public:
		void RegisterThisPointer(JDirectionalLight* lit)
		{
			thisPointer = Core::GetWeakPtr(lit);
		}
		void RegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(this);
			Graphic::JCullingInterface::SetInterfacePointer(this);
			Graphic::JCsmHandlerInterface::SetInterfacePointer(this);
		}
		void RegisterPostCreation()
		{
			JLightPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer.Get(), this, thisPointer->GetGuid());
		}
		void RegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DIRECTIONAL_LIGHT)
				DirLitFrame::RegisterFrameData(type, (DirLitFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CASCADE_SHADOW_MAP_INFO)
				CsmInfoFrame::RegisterFrameData(type, (CsmInfoFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), GetCsmTargetCount());
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_ARRAY_DRAW)
				CsmDrawFrame::RegisterFrameData(type, (CsmDrawFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), GetCsmTargetCount());
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_DRAW)
				ShadowMapNormalDrawFrame::RegisterFrameData(type, (ShadowMapNormalDrawFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS)
				DepthTestFrame::RegisterFrameData(type, (DepthTestFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS)
				HzbOccComputeFrame::RegisterFrameData(type, (HzbOccComputeFrame*)this, thisPointer->GetOwner()->GetOwnerGuid(), 1);
		}
		void DeRegisterPreDestruction()
		{
			if (thisPointer != nullptr)
				JLightPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer.Get(), thisPointer->GetGuid());
		}
		void DeRegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			JFrameUpdateData::DeRegisterFrameData(type, CastFrameInterface(type));
		}
		static void RegisterTypeData()
		{
			Core::JIdentifier::RegisterPrivateInterface(JDirectionalLight::StaticTypeInfo(), lPrivate);
			IMPL_REALLOC_BIND(JDirectionalLight::JDirectionalLightImpl, thisPointer)
			SET_GUI_FLAG(Core::J_GUI_OPTION_FLAG::J_GUI_OPTION_DISPLAY_PARENT_TO_CHILD);
		}
	};
 
	JDirectionalLight::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JLight::InitData(JDirectionalLight::StaticTypeInfo(), owner)
	{}
	JDirectionalLight::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JLight::InitData(JDirectionalLight::StaticTypeInfo(),  guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JDirectionalLight::PrivateInterface()const noexcept
	{
		return lPrivate;
	}  
	const Graphic::JGraphicResourceUserInterface JDirectionalLight::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface(impl.get());
	}
	const Graphic::JCullingUserInterface JDirectionalLight::CullingUserInterface()const noexcept
	{
		return Graphic::JCullingUserInterface(impl.get());
	}
	J_LIGHT_TYPE JDirectionalLight::GetLightType()const noexcept
	{
		return impl->GetLightType();
	} 
	J_SHADOW_MAP_TYPE JDirectionalLight::GetShadowMapType()const noexcept
	{
		return impl->GetShadowMapType();
	}  
	float JDirectionalLight::GetFrustumNear()const noexcept
	{
		return impl->GetFrustumNear();
	}
	float JDirectionalLight::GetFrustumFar()const noexcept
	{
		return impl->GetFrustumFar();
	}	 	 
	DirectX::BoundingBox JDirectionalLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
	}
	uint JDirectionalLight::GetCsmSplitCount()const noexcept
	{
		return impl->GetSplitCount();
	}
	float JDirectionalLight::GetCsmSplitBlendRate()const noexcept
	{
		return impl->GetSplitBlendRate();
	}
	float JDirectionalLight::GetCsmShadowDistance()const noexcept
	{
		return impl->GetShadowDistance();
	}
	float JDirectionalLight::GetCsmLevelBlendRate()const noexcept
	{
		return impl->GetLevelBlendRate();
	}
	void JDirectionalLight::SetShadow(const bool value)noexcept
	{
		if (value == IsShadowActivated())
			return;

		JLight::SetShadow(value);
		impl->SetShadow(value);
	}
	void JDirectionalLight::SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept
	{
		if (sQuality == GetShadowResolutionType())
			return;

		JLight::SetShadowResolution(sQuality);
		impl->SetShadowResolution(sQuality);
	}    
	void JDirectionalLight::SetAllowDisplayShadowMap(const bool value)noexcept
	{
		if (value == AllowDisplayShadowMap())
			return;

		JLight::SetAllowDisplayShadowMap(value);
		impl->SetAllowDisplayShadowMap(value);
	}
	void JDirectionalLight::SetCsm(const bool value)noexcept
	{
		impl->SetCsm(value);
	}
	void JDirectionalLight::SetCsmSplitCount(const uint newCount)noexcept
	{
		impl->SetSplitCount(newCount);
	}
	void JDirectionalLight::SetCsmSplitBlendRate(const float value)noexcept
	{
		impl->SetSplitBlendRate(value);
	}
	void JDirectionalLight::SetCsmShadowDistance(const float value)noexcept
	{
		impl->SetShadowDistance(value);
	}
	void JDirectionalLight::SetCsmLevelBlendRate(const float value)noexcept
	{
		impl->SetLevelBlendRate(value);
	}
	bool JDirectionalLight::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
	} 
	bool JDirectionalLight::IsCsmActivated()const noexcept
	{
		return impl->IsCsmActivated();
	} 
	bool JDirectionalLight::CanAllocateCsm()const noexcept
	{
		return impl->CanAllocateCsm();
	}
	bool JDirectionalLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	} 
	bool JDirectionalLight::AllowFrustumCulling()const noexcept
	{
		return false;
	}
	bool JDirectionalLight::AllowHzbOcclusionCulling()const noexcept
	{
		return impl->AllowHzbOcclusionCulling();
	}
	bool JDirectionalLight::AllowHdOcclusionCulling()const noexcept
	{
		return impl->AllowHdOcclusionCulling();
	}
	bool JDirectionalLight::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
	}
	void JDirectionalLight::DoActivate()noexcept
	{
		JLight::DoActivate(); 
		impl->Activate();
		impl->SetFrameDirty();
	}
	void JDirectionalLight::DoDeActivate()noexcept
	{
		impl->DeActivate();
		impl->OffFrameDirty();
		JLight::DoDeActivate();
	}
	JDirectionalLight::JDirectionalLight(const InitData& initData)
		:JLight(initData), impl(std::make_unique<JDirectionalLightImpl>(initData, this))
	{ }
	JDirectionalLight::~JDirectionalLight()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JDirectionalLightPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JDirectionalLightPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JDirectionalLightPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JDirectionalLightPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JDirectionalLightPrivate::FrameIndexInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JDirectionalLight>(*static_cast<JDirectionalLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JLightPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JDirectionalLight* lit = static_cast<JDirectionalLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit);
		lit->impl->RegisterInterfacePointer();
		lit->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JDirectionalLight::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JDirectionalLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		if (!JLightPrivate::CreateInstanceInterface::Copy(from, to))
			return false;

		return JDirectionalLight::JDirectionalLightImpl::DoCopy(static_cast<JDirectionalLight*>(from.Get()), static_cast<JDirectionalLight*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		static_cast<JDirectionalLight*>(ptr)->impl->DeRegisterPreDestruction();
		JLightPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectionalLight::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid = 0;
		J_OBJECT_FLAG flag = OBJECT_FLAG_NONE;
		bool isActivated = false;
		bool sOnCsm = false; 
		float sSplitBlendRate = 0;
		uint sSplitCount = 0;
		float sShadowDistance = 0;
		float sLevelBlendRate = 0;
		float sPenumbraWidth = 0;
		float sPenumbraBlockerScale = 0; 

		auto loadData = static_cast<JDirectionalLight::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		Graphic::JCsmOption csmOption;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JDirectionalLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JDirectionalLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(tool, litUser);
		JObjectFileIOHelper::LoadAtomicData(tool, sOnCsm, "OnCsm:"); 
		JObjectFileIOHelper::LoadAtomicData(tool, sSplitBlendRate, "CsmSplitBlendRate:");
		JObjectFileIOHelper::LoadAtomicData(tool, sSplitCount, "CsmSplitCount:");
		JObjectFileIOHelper::LoadAtomicData(tool, sShadowDistance, "CsmShadowDistance:");
		JObjectFileIOHelper::LoadAtomicData(tool, sLevelBlendRate, "CsmLevelBlendRate:");

		litUser->SetCsm(sOnCsm);  
		litUser->impl->SetSplitBlendRate(sSplitBlendRate);
		litUser->impl->SetSplitCount(sSplitCount);
		litUser->impl->SetShadowDistance(sShadowDistance);
		litUser->impl->SetLevelBlendRate(sLevelBlendRate);
		if (!isActivated)
			litUser->DeActivate();
		return litUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JDirectionalLight::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JDirectionalLight::StoreData*>(data);
		if (!storeData->HasCorrectType(JDirectionalLight::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JDirectionalLight> lit;
		lit.ConnnectChild(storeData->obj);

		JDirectionalLight::JDirectionalLightImpl* impl = lit->impl.get();
		JFileIOTool& tool = storeData->tool;
		auto option = impl->GetCsmOption();

		JObjectFileIOHelper::StoreComponentIden(tool, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(tool, lit);
		JObjectFileIOHelper::StoreAtomicData(tool, impl->onCsm, "OnCsm:"); 
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetSplitBlendRate(), "CsmSplitBlendRate:");
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetSplitCount(), "CsmSplitCount:");
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetShadowDistance(), "CsmShadowDistance:");
		JObjectFileIOHelper::StoreAtomicData(tool, option.GetLevelBlendRate(), "CsmLevelBlendRate:");
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JLight* lit, const bool isUpdateForced)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		JDirectionalLight* dLit = static_cast<JDirectionalLight*>(lit);
		if (isUpdateForced)
			dLit->impl->SetFrameDirty();

		dLit->impl->SetLastFrameUpdatedTrigger(false);
		dLit->impl->SetLastFrameHotUpdatedTrigger(false);
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->UpdateLightTransform();
		return dLit->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JDirectionalLight* lit, Graphic::JDirectionalLightConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JDirectionalLight* lit, Graphic::JCsmConstants& constant, const uint index)noexcept
	{
		lit->impl->UpdateFrame(constant, index);
	} 
	void FrameUpdateInterface::UpdateFrame(JDirectionalLight* lit, Graphic::JShadowMapArrayDrawConstants& constant, const uint index)noexcept
	{
		lit->impl->UpdateFrame(constant, index);
	} 
	void FrameUpdateInterface::UpdateFrame(JDirectionalLight* lit, Graphic::JShadowMapDrawConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JDirectionalLight* lit, Graphic::JDepthTestPassConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JDirectionalLight* lit, Graphic::JHzbOccComputeConstants& constant, const uint queryCount, const uint queryOffset)noexcept
	{
		lit->impl->UpdateFrame(constant, queryCount, queryOffset);
	}
	void FrameUpdateInterface::UpdateEnd(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return;

		JDirectionalLight* dLit = static_cast<JDirectionalLight*>(lit);
		if (dLit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			dLit->impl->SetLastFrameHotUpdatedTrigger(true);
		dLit->impl->SetLastFrameUpdatedTrigger(true);
		dLit->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return -1;

		return static_cast<JDirectionalLight*>(lit)->impl->CastLightFrameInteface()->GetFrameIndex();
	}
	int FrameUpdateInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return -1;

		auto ptr = static_cast<JDirectionalLight*>(lit)->impl->CastShadowMapFrameInteface();
		return ptr != nullptr ? ptr->GetFrameIndex() : -1;
	}
	int FrameUpdateInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->DepthTestFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->HzbOccComputeFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetCsmFrameSize(JDirectionalLight* lit)noexcept
	{
		return lit->impl->CsmInfoFrame::GetFrameIndexSize();
	}
	bool FrameUpdateInterface::IsHotUpdate(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources;
	}
	bool FrameUpdateInterface::IsLastFrameHotUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->IsLastFrameUpdated();
	}
	bool FrameUpdateInterface::HasLitRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->CastLightFrameInteface()->HasMovedDirty();
	}
	bool FrameUpdateInterface::HasShadowMapRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		auto ptr = static_cast<JDirectionalLight*>(lit)->impl->CastShadowMapFrameInteface();
		return ptr != nullptr ? ptr->HasMovedDirty() : false;
	}
	bool FrameUpdateInterface::HasDepthTestPassRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->DepthTestFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasHzbOccComputeRecopyRequest(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->HzbOccComputeFrame::HasMovedDirty();
	}

	int FrameIndexInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return -1;

		return static_cast<JDirectionalLight*>(lit)->impl->CastLightFrameInteface()->GetFrameIndex();
	}
	int FrameIndexInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return -1;

		auto ptr = static_cast<JDirectionalLight*>(lit)->impl->CastShadowMapFrameInteface();
		return ptr != nullptr ? ptr->GetFrameIndex() : -1;
	}
	int FrameIndexInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->DepthTestFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->HzbOccComputeFrame::GetFrameIndex();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JDirectionalLightPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JDirectionalLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JDirectionalLightPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
	JLightPrivate::FrameUpdateInterface& JDirectionalLightPrivate::GetFrameUpdateInterface()const noexcept
	{
		static FrameUpdateInterface pI;
		return pI;
	}
	JLightPrivate::FrameIndexInterface& JDirectionalLightPrivate::GetFrameIndexInterface()const noexcept
	{
		static FrameIndexInterface pI;
		return pI;
	}
}