#include"JDirectionalLight.h"   
#include"JDirectionalLightPrivate.h"
#include"JLightConstants.h"
#include"../Transform/JTransform.h" 
#include"../JComponentHint.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
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
//#include"../../../Develop/Debug/JDevelopDebug.h"
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
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JDirectionalLightPrivate lPrivate;

		static XMVECTOR CalLightWorldDir(const JUserPtr<JTransform>& transform, const JVector3<float>& initDir = JVector3<float>(0, -1, 0)) noexcept
		{
			return XMVector3Normalize(XMVector3Rotate(initDir.ToXmV(), transform->GetWorldQuaternion().ToXmV()));
		}
		static XMVECTOR CalLightWorldPos(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMVectorScale(CalLightWorldDir(transform), -Constants::lightMaxDistance);
		}
		static XMMATRIX CalView(const JUserPtr<JTransform>& transform) noexcept
		{
			return XMMatrixLookAtLH(CalLightWorldPos(transform),
				XMVectorSet(0, 0, 0, 1),
				XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
		}
		static XMMATRIX CalProj(const BoundingBox& sceneBBox,
			const JMatrix4x4& view,
			_Out_ JVector3<float>& outMin,
			_Out_ JVector3<float>& outMax)noexcept
		{
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
			outMin = minV;
			outMax = maxV;

			return XMMatrixOrthographicOffCenterLH(XMVectorGetX(minV), XMVectorGetX(maxV),
				XMVectorGetY(minV), XMVectorGetY(maxV),
				XMVectorGetZ(minV), XMVectorGetZ(maxV));
		}
		static XMMATRIX CalProj(const BoundingBox& sceneBBox,
			const JMatrix4x4& view)noexcept
		{
			JVector3<float> minV3;
			JVector3<float> maxV3;
			return CalProj(sceneBBox, view, minV3, maxV3);
		}
	}

	class JDirectionalLight::JDirectionalLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public Graphic::JGraphicMultiResourceHolder,
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
		REGISTER_PROPERTY_EX(onCsm, IsCsmActivated, SetCsm, GUI_CHECKBOX(GUI_BOOL_CONDITION_REF_USER(OnShadow, true)))
		bool onCsm = false;
	public:  
		bool allowHzbOcclusionCulling = false;
		bool allowHdOcclusionCulling = false;
		bool allowDisplayOccCullingDepthMap = false;
	public:
		//test
		REGISTER_PROPERTY_EX(csmNearFactor, GetCsmNearFactor, SetCsmNearFactor, GUI_SLIDER(-100, 100))
		float csmNearFactor = 0;
		REGISTER_PROPERTY_EX(csmFarFactor, GetCsmFarFactor, SetCsmFarFactor, GUI_SLIDER(-100, 100))
		float csmFarFactor = 0;
	public:
		J_SIMPLE_GET_SET(float, csmNearFactor, CsmNearFactor)
		J_SIMPLE_GET_SET(float, csmFarFactor, CsmFarFactor)
	public:
		JDirectionalLightImpl(const InitData& initData, JDirectionalLight* thisLitRaw)
			:Graphic::JCsmHandlerInterface()
		{ }
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
		float GetNear()const noexcept
		{
			JVector3<float> vMinF;
			JVector3<float> vMaxF;
			const XMMATRIX proj = CalProj(GetSceneBBox(), view, vMinF, vMaxF);
			return vMinF.z;
		}
		float GetFar()const noexcept
		{
			JVector3<float> vMinF;
			JVector3<float> vMaxF;
			const XMMATRIX proj = CalProj(GetSceneBBox(), view, vMinF, vMaxF);
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
	public: 
		//value가 bool type일경우에만 justCallFunc을 사용할수있다
		//justCallFunc는 값을 변경하지않고 함수내에서 value per 기능을 수행한다
		void SetShadow(const bool value)
		{
			SetShadowEx(value, false);
		}
		void SetShadowEx(const bool value, const bool justCallFunc)noexcept
		{
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (thisPointer->IsShadowActivated())
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
			if (value == onCsm)
				return;
		 
			if (thisPointer->IsActivated())
			{
				DestroyShadowMapResource();
				if (onCsm)
					DeRegisterCsmHandlerface();
			}
			onCsm = value;
			if (thisPointer->IsActivated())
			{
				if (onCsm)
					RegisterCsmHandlerface();
				CreateShadowMapResource(CanAllocateCsm());
			}
			SetFrameDirty();
		}
		void SetAllowDisplayShadowMap(const bool value)
		{
			SetAllowDisplayShadowMapEx(value, false);
		}
		void SetAllowDisplayShadowMapEx(const bool value, const bool justCallFunc = false)
		{ 		 
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
					CreateShadowMapDebugResource(CanAllocateCsm());
				else
					DestroyShadowMapDebugResource();
			}
			SetFrameDirty();
		} 
		void SetAllowHzbOcclusionCulling(const bool value, const bool justCallFunc = false)noexcept
		{ 
			if (!justCallFunc && allowHzbOcclusionCulling == value)
				return;

			if (AllowHdOcclusionCulling())
				SetAllowHdOcclusionCulling(false);

			if (!justCallFunc)
				allowHzbOcclusionCulling = value;
			if (justCallFunc || thisPointer->IsActivated())
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
					DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
					DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
					DestroyCullingData(Graphic::J_CULLING_TYPE::HZB_OCCLUSION);
					PopHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMap(false, true);
				}
			}
			SetFrameDirty();
		}
		void SetAllowHdOcclusionCulling(const bool value, const bool justCallFunc = false)noexcept
		{ 
			if (!justCallFunc && allowHdOcclusionCulling == value)
				return;

			if (AllowHzbOcclusionCulling())
				SetAllowHzbOcclusionCulling(false);

			if (!justCallFunc)
				allowHdOcclusionCulling = value;
			if (justCallFunc || thisPointer->IsActivated())
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
		void SetAllowDisplayOccCullingDepthMap(const bool value, const bool justCallFunc = false)noexcept
		{
			if (!justCallFunc && allowDisplayOccCullingDepthMap == value)
				return;

			if (!justCallFunc)
				allowDisplayOccCullingDepthMap = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
					CreateOcclusionDepthDebug(AllowHzbOcclusionCulling());
				else
					DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
			}
			SetFrameDirty();
		}
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
					CreateShadowMapArrayTexture(thisPointer->GetShadowMapSize(), GetCsmOption().GetSplitCount());
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
			DeRegisterCsmHandlerface();
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
			DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG);
		}
	public:
		void Activate()noexcept
		{
			if (AllowHdOcclusionCulling() && AllowHzbOcclusionCulling())
				allowHdOcclusionCulling = allowHzbOcclusionCulling = false;

			RegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (thisPointer->IsShadowActivated())
				SetShadowEx(true, true); 
			if (allowHzbOcclusionCulling)
				SetAllowHzbOcclusionCulling(true, true);
			if (allowHdOcclusionCulling)
				SetAllowHdOcclusionCulling(true, true);
		}
		void DeActivate()noexcept
		{
			//has order dependency
			DeRegisterLightFrameData(JLightType::LitToFrameR(GetLightType()));
			if (thisPointer->IsShadowActivated())
				SetShadowEx(false, true); 
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
			constant.color = thisPointer->GetColor();
			if (CanAllocateCsm())
			{
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(view.LoadXM()));
				constant.shadowMapIndex = GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, 0);
				constant.csmDataIndex = CsmInfoFrame::GetUploadIndex();
			}
			else if (thisPointer->IsShadowActivated())
			{
				constant.shadowMapTransform.StoreXM(XMMatrixTranspose(GetShadowMapTransform()));
				constant.shadowMapIndex = GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP, 0);
			}
			constant.shadowMapType = (uint)GetShadowMapType();
			constant.direction = CalLightWorldDir(GetTransform());
			DirLitFrame::MinusMovedDirty(); 
		}
		void UpdateFrame(Graphic::JCsmConstants& constant, const uint index)noexcept final
		{ 
			const auto& result = GetCsmComputeResult(index);
			for (uint i = 0; i < result.subFrustumCount; ++i)
			{
				constant.scale[i] = result.scale[i];
				constant.posOffset[i] = result.posOffset[i];
			}

			constant.mapMinBorder = (float)(1.0f / (float)thisPointer->GetShadowMapSize());
			constant.mapMaxBorder = (float)(((float)thisPointer->GetShadowMapSize() - 1.0f) / (float)thisPointer->GetShadowMapSize());
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
			proj.StoreXM(CalProj(GetSceneBBox(), view, vSceneBBoxMinF, vSceneBBoxMaxF));
			if (CanAllocateCsm())
				CsmUpdate(viewM, GetSceneBBox(), thisPointer->GetShadowMapSize(), DebugDataSet{ csmNearFactor, csmFarFactor });
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
				SetFrameDirty();
				if (!isSameCount)
				{
					if (smArrayCount == 0 && targetCount == 1)
					{
						DestroyShadowMapResource();
						CreateShadowMapResource(CanAllocateCsm());
					}
					else
					{
						CreateShadowMapArrayTexture(thisPointer->GetShadowMapSize(), GetCsmOption().GetSplitCount());
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
					DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP_ARRAY, index);
					if (thisPointer->AllowDisplayShadowMap())
						DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG, index);
				}
			}
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
	float JDirectionalLight::GetNear()const noexcept
	{
		return impl->GetNear();
	}
	float JDirectionalLight::GetFar()const noexcept
	{
		return impl->GetFar();
	}	 	 
	DirectX::BoundingBox JDirectionalLight::GetBBox()const noexcept
	{
		return impl->GetBBox();
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
		if (sQuality == GetShadowResolution())
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
		size_t guid;
		J_OBJECT_FLAG flag;
		bool sOnCsm; 

		auto loadData = static_cast<JDirectionalLight::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JUserPtr<JGameObject> owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JDirectionalLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JDirectionalLight> litUser;
		litUser.ConnnectChild(idenUser);

		JLightPrivate::AssetDataIOInterface::LoadLightData(stream, litUser);		
		JFileIOHelper::LoadAtomicData(stream, sOnCsm);
		litUser->SetCsm(sOnCsm); 
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
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, lit.Get());
		JLightPrivate::AssetDataIOInterface::StoreLightData(stream, lit);
		JFileIOHelper::StoreAtomicData(stream, L"OnCsm:", impl->onCsm);

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
		if (dLit->impl->IsFrameDirted())
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

		return static_cast<JDirectionalLight*>(lit)->impl->CastLightFrameInteface()->GetUploadIndex();
	}
	int FrameUpdateInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return -1;

		auto ptr = static_cast<JDirectionalLight*>(lit)->impl->CastShadowMapFrameInteface();
		return ptr != nullptr ? ptr->GetUploadIndex() : -1;
	}
	int FrameUpdateInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->DepthTestFrame::GetUploadIndex();
	}
	int FrameUpdateInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->HzbOccComputeFrame::GetUploadIndex();
	}
	int FrameUpdateInterface::GetCsmFrameSize(JDirectionalLight* lit)noexcept
	{
		return lit->impl->CsmInfoFrame::GetIndexSize();
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

		return static_cast<JDirectionalLight*>(lit)->impl->CastLightFrameInteface()->GetUploadIndex();
	}
	int FrameIndexInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return -1;

		auto ptr = static_cast<JDirectionalLight*>(lit)->impl->CastShadowMapFrameInteface();
		return ptr != nullptr ? ptr->GetUploadIndex() : -1;
	}
	int FrameIndexInterface::GetDepthTestPassFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->DepthTestFrame::GetUploadIndex();
	}
	int FrameIndexInterface::GetHzbOccComputeFrameIndex(JLight* lit)noexcept
	{
		if (lit->GetLightType() != J_LIGHT_TYPE::DIRECTIONAL)
			return false;

		return static_cast<JDirectionalLight*>(lit)->impl->HzbOccComputeFrame::GetUploadIndex();
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