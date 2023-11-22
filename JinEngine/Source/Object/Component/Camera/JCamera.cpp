#include"JCamera.h"
#include"JCameraPrivate.h"
#include"JCameraConstants.h"
#include"../JComponentHint.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../../JObjectFileIOHelper.h"
#include"../../GameObject/JGameObject.h"  
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"  
#include"../../../Core/Guid/JGuidCreator.h"  
#include"../../../Core/File/JFileConstant.h" 
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Math/JMathHelper.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/JGraphicDrawListInterface.h"
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/Culling/JCullingConstants.h" 
#include"../../../Graphic/Frameresource/JCameraConstants.h"    
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Frameresource/JOcclusionConstants.h"
#include"../../../Graphic/Frameresource/JDepthTestConstants.h"
#include"../../../Graphic/ShadowMap/JCsmTargetInterface.h"
#include"../../../Window/JWindow.h" 
#include<fstream>
 
namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JCameraPrivate cPrivate;
	}

	namespace
	{
		using CameraFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder3<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, Graphic::JCameraConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, Graphic::JDepthTestPassConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, Graphic::JHzbOccComputeConstants&, const uint, const uint>>,
			Graphic::JFrameDirty>;
		//first extra is occlusion 
	}
	namespace Private
	{
		static constexpr float minSsaoRadius = 0.0f;
		static constexpr float maxSsaoRadius = 16.0f;
		static constexpr float minSsaoFade = minSsaoRadius;
		static constexpr float maxSsaoFade = maxSsaoRadius;
		static constexpr float minSsaoEpsilon = minSsaoRadius;
		static constexpr float maxSsaoEpsilon = maxSsaoRadius;
	}

	class JCamera::JCameraImpl : public Core::JTypeImplBase,
		public CameraFrameUpdate,
		public Graphic::JGraphicWideSingleResourceHolder<7>,// main) rtv, dsv , debug) dsv(out line), srv, hzb) mip, depth map, debug
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCullingInterface,
		public Graphic::JCsmTargetInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JCameraImpl)
		REGISTER_GUI_BOOL_CONDITION(IsOrthoCam, isOrtho, false)
	public:
		using CamFrame = JFrameInterface1;
		using DepthTestFrame = JFrameInterface2;
		using HzbOccReqFrame = JFrameInterface3;
	public:
		JWeakPtr<JCamera> thisPointer = nullptr;
	public:
		// Cache View/Proj matrices.
		JMatrix4x4 mView;
		JMatrix4x4 mProj;
		DirectX::BoundingFrustum mCamFrustum;
	public:
		JVector2F rtSizeRate = JVector2F::One();
	public:
		//JTransform* ownerTransform;
		J_CAMERA_STATE camState = J_CAMERA_STATE::RENDER;
		REGISTER_GUI_GROUP(camera)
		// Cache frustum properties.
		REGISTER_PROPERTY_EX(camNear, GetNear, SetNear, GUI_SLIDER(Constants::minCamFrustumNear, Constants::maxCamFrustumFar, true))
		float camNear = 0.0f;
		REGISTER_PROPERTY_EX(camFar, GetFar, SetFar, GUI_SLIDER(Constants::minCamFrustumNear, Constants::maxCamFrustumFar, true))
		float camFar = 0.0f;
		REGISTER_PROPERTY_EX(camAspect, GetAspect, SetAspect, GUI_SLIDER(0.1f, 32.0f, true, false, 3, GUI_BOOL_CONDITION_USER(IsOrthoCam, false)))
		float camAspect = 0.0f;		// Perspective일때 사용
		REGISTER_PROPERTY_EX(camFov, GetFovYDegree, SetFovYDegree, GUI_SLIDER(1, 185, true))
		float camFov = 0.0f;		//fovY
	public:
		REGISTER_PROPERTY_EX(camOrthoViewWidth, GetOrthoViewWidth, SetOrthoViewWidth, GUI_SLIDER(1, 3840, true, false, 3, GUI_BOOL_CONDITION_USER(IsOrthoCam, true)))
		float camOrthoViewWidth = 0.0f;		// Ortho일때 사용
		REGISTER_PROPERTY_EX(camOrthoViewHeight, GetOrthoViewHeight, SetOrthoViewHeight, GUI_SLIDER(1, 2160, true, false, 3, GUI_BOOL_CONDITION_USER(IsOrthoCam, true)))
		float camOrthoViewHeight = 0.0f;	// Ortho일때 사용
		float camNearViewHeight = 0.0f;
		float camFarViewHeight = 0.0f;
	public:
		//Culling Option
		REGISTER_PROPERTY_EX(frustumCulingFrequency, GetFrustumCullingFrequency, SetFrustumCullingFrequency, GUI_SLIDER(Graphic::Constants::cullingUpdateFrequencyMin, Graphic::Constants::cullingUpdateFrequencyMax, true))
		float frustumCulingFrequency = 0;
		REGISTER_PROPERTY_EX(occlusionCulingFrequency, GetOcclusionCullingFrequency, SetOcclusionCullingFrequency, GUI_SLIDER(Graphic::Constants::cullingUpdateFrequencyMin, Graphic::Constants::cullingUpdateFrequencyMax, true))
		float occlusionCulingFrequency = 0;
	public:
		REGISTER_GUI_GROUP(Ssao)
		REGISTER_METHOD_GUI_WIDGET(SsaoRadius, GetSsaoRadius, SetSsaoRadius, GUI_SLIDER(Private::minSsaoRadius, Private::maxSsaoRadius, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_METHOD_GUI_WIDGET(SsaoFadeStart, GetSsaoFadeStart, SetSsaoFadeStart, GUI_SLIDER(Private::minSsaoFade, Private::maxSsaoFade, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_METHOD_GUI_WIDGET(SsaoFadeEnd, GetSsaoFadeEnd, SetSsaoFadeEnd, GUI_SLIDER(Private::minSsaoFade, Private::maxSsaoFade, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_METHOD_GUI_WIDGET(SsaoSurfaceEpsilon, GetSsaoSurfaceEpsilon, SetSsaoSurfaceEpsilon, GUI_SLIDER(Private::minSsaoEpsilon, Private::maxSsaoEpsilon, true, false, 3, GUI_GROUP_USER(Ssao)))
		REGISTER_METHOD_GUI_WIDGET(SsaoType, GetSsaoType, SetSsaoType, GUI_ENUM_COMBO(Graphic::J_SSAO_TYPE, "", GUI_GROUP_USER(Ssao)))
		Graphic::JSsaoDesc ssaoDesc;
	public:
		REGISTER_PROPERTY_EX(isOrtho, IsOrthoCamera, SetOrthoCamera, GUI_CHECKBOX())
		bool isOrtho = false;
		REGISTER_PROPERTY_EX(allowDisplayDepthMap, AllowDisplayDepthMap, SetAllowDisplaySceneDepthMap, GUI_CHECKBOX())
		bool allowDisplayDepthMap = false;
		REGISTER_PROPERTY_EX(allowDisplayDebug, AllowDisplayDebug, SetAllowDisplayDebug, GUI_CHECKBOX())
		bool allowDisplayDebug = false;
		REGISTER_PROPERTY_EX(allowFrustumCulling, AllowFrustumCulling, SetAllowFrustumCulling, GUI_CHECKBOX())
		bool allowFrustumCulling = false;
		REGISTER_PROPERTY_EX(allowHzbOcclusionCulling, AllowHzbOcclusionCulling, SetAllowHzbOcclusionCulling, GUI_CHECKBOX())
		bool allowHzbOcclusionCulling = false;
		REGISTER_PROPERTY_EX(allowHdOcclusionCulling, AllowHdOcclusionCulling, SetAllowHdOcclusionCulling, GUI_CHECKBOX())
		bool allowHdOcclusionCulling = false;
		REGISTER_PROPERTY_EX(allowDisplayOccCullingDepthMap, AllowDisplayOccCullingDepthMap, SetAllowDisplayOccCullingDepthMap, GUI_CHECKBOX())
		bool allowDisplayOccCullingDepthMap = false;
		bool allowAllCamCullResult = false;	//use editor cam for check space spatial result
	public:
		REGISTER_PROPERTY_EX(allowSsao, AllowSsao, SetAllowSsao, GUI_CHECKBOX())
		bool allowSsao = false;
	public:
		//Caution
		//Impl생성자에서 interface class 참조시 interface class가 함수내에서 impl을 참조할 경우 error
		//impl이 아직 생성되지 않았으므로
		JCameraImpl(const InitData& initData, JCamera* thisCamRaw)
		{
			rtSizeRate = initData.rtSizeRate;
		}
		~JCameraImpl()
		{ }
	public:
		float GetNear()const noexcept
		{    
			return camNear;
		}
		float GetFar()const noexcept
		{
			return camFar;
		}
		float GetFovX()const noexcept
		{
			float halfWidth = 0.5f * GetNearViewWidth();
			return 2.0f * atan(halfWidth / camNear);
		}
		float GetFovXDegree()const noexcept
		{
			return JMathHelper::RadToDeg * GetFovX();
		}
		float GetFovY()const noexcept
		{ 
			return camFov;
		}
		float GetFovYDegree()const noexcept
		{
			return JMathHelper::RadToDeg * camFov;
		}
		float GetAspect()const noexcept
		{
			return camAspect;
		}
		float GetNearViewWidth()const noexcept
		{
			return camAspect * camNearViewHeight;
		}
		float GetNearViewHeight()const noexcept
		{
			return camNearViewHeight;
		}
		float GetFarViewWidth()const noexcept
		{
			return camAspect * camFarViewHeight; 
		}
		float GetFarViewHeight()const noexcept
		{
			return camFarViewHeight;
		}
		float GetOrthoViewWidth()const noexcept
		{
			return camOrthoViewWidth;
		}
		float GetOrthoViewHeight()const noexcept
		{
			return camOrthoViewHeight;
		}
		float GetRenderViewWidth()const noexcept
		{
			return isOrtho ? camOrthoViewWidth : GetFarViewWidth();
		}
		float GetRenderViewHeight()const noexcept
		{
			return isOrtho ? camOrthoViewHeight : GetFarViewHeight();
		}
		float GetFrustumCullingFrequency()const noexcept
		{
			return frustumCulingFrequency;
		}
		float GetOcclusionCullingFrequency()const noexcept
		{
			return occlusionCulingFrequency;
		}
		DirectX::BoundingFrustum GetBoundingFrustum()const noexcept final
		{
			DirectX::BoundingFrustum worldCamFrustum;
			mCamFrustum.Transform(worldCamFrustum, thisPointer->GetOwner()->GetTransform()->GetWorldMatrix());
			return worldCamFrustum;
		} 
		size_t GetCsmTargetGuid()const noexcept final
		{
			return thisPointer->GetGuid();
		}
		size_t GetCsmAreaGuid()const noexcept final
		{
			return thisPointer->GetOwner()->GetOwnerGuid();
		} 
		JVector2F GetRtSize()const noexcept
		{
			return JWindow::GetDisplaySize() * rtSizeRate;
		}
		float GetSsaoRadius()const noexcept
		{
			return ssaoDesc.radius;
		}
		float GetSsaoFadeStart()const noexcept
		{
			return ssaoDesc.fadeStart;
		}
		float GetSsaoFadeEnd()const noexcept
		{
			return ssaoDesc.fadeEnd;
		}
		float GetSsaoSurfaceEpsilon()const noexcept
		{
			return ssaoDesc.surfaceEpsilon;
		}
		Graphic::J_SSAO_TYPE GetSsaoType()const noexcept
		{
			return ssaoDesc.ssaoType;
		}
	public:
		void SetNear(const float value)noexcept
		{
			camNear = std::clamp(value, Constants::minCamFrustumNear, camFar);
			if (camNear < 0.1f)
				camNear = 0.1f;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFar(const float value) noexcept
		{
			camFar = std::clamp(value, camNear, Constants::maxCamFrustumFar);
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFovY(const float value) noexcept
		{
			camFov = value;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetFovYDegree(const float value) noexcept
		{
			camFov = value * JMathHelper::DegToRad;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetAspect(float value) noexcept
		{
			camAspect = value;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetOrthoViewWidth(const float value)noexcept
		{
			camOrthoViewWidth = value;
			if (isOrtho)
				CalOrthoLens();
		}
		void SetOrthoViewHeight(const float value)noexcept
		{
			camOrthoViewHeight = value;
			if (isOrtho)
				CalOrthoLens();
		}
		void SetOrthoViewSize(const float width, const float height)
		{
			camOrthoViewWidth = width;
			camOrthoViewHeight = height;
			if (isOrtho)
				CalOrthoLens();
		}
		void SetOrthoCamera(const bool value)noexcept
		{
			isOrtho = value;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFrustumCullingFrequency(const float value)noexcept
		{
			frustumCulingFrequency = value;
		}
		void SetOcclusionCullingFrequency(const float value)noexcept
		{
			occlusionCulingFrequency = value;
		}
		//value가 bool type일경우에만 justCallFunc을 사용할수있다
		//justCallFunc는 값을 변경하지않고 함수내에서 value per 기능을 수행한다
		void SetAllowDisplaySceneDepthMap(const bool value)noexcept
		{
			SetAllowDisplaySceneDepthMapEx(value, false);
		}
		void SetAllowDisplaySceneDepthMapEx(const bool value, const bool justCallFunc)noexcept
		{
			if (!justCallFunc && allowDisplayDepthMap == value)
				return;

			if (!justCallFunc)
				allowDisplayDepthMap = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
					CreateLayerDepthDebugResource(GetRtSize());
				else
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::LAYER_DEPTH_MAP_DEBUG);
			}
			SetFrameDirty();
		}
		void SetAllowDisplayDebug(const bool value)noexcept
		{
			SetAllowDisplayDebugEx(value, false);
		}
		void SetAllowDisplayDebugEx(const bool value, const bool justCallFunc)noexcept
		{
			if (!justCallFunc && allowDisplayDebug == value)
				return;

			if(!justCallFunc)
				allowDisplayDebug = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
					CreateDebugDepthStencil(GetRtSize());
				else
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::DEBUG_LAYER_DEPTH_STENCIL);
			}
			SetFrameDirty();
		}
		void SetAllowFrustumCulling(const bool value)noexcept
		{
			SetAllowFrustumCullingEx(value, false);
		}
		void SetAllowFrustumCullingEx(const bool value, const bool justCallFunc)noexcept
		{
			if (!justCallFunc && allowFrustumCulling == value)
				return;

			if (!justCallFunc)
				allowFrustumCulling = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
				{
					CreateFrustumCullingData();
					AddFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
				}
				else
				{
					DestroyCullingData(Graphic::J_CULLING_TYPE::FRUSTUM);
					PopFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
				}
			}
			SetFrameDirty();
		}
		void SetAllowHzbOcclusionCulling(const bool value)noexcept
		{
			SetAllowHzbOcclusionCullingEx(value, false);
		}
		void SetAllowHzbOcclusionCullingEx(const bool value, const bool justCallFunc)noexcept
		{
			if (!justCallFunc && allowHzbOcclusionCulling == value)
				return;
			 
			if (AllowHdOcclusionCulling())
				SetAllowHdOcclusionCulling(false);

			if(!justCallFunc)
				allowHzbOcclusionCulling = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
				{
					RegisterOccPassFrameData();
					RegisterHzbOccPassFrameData();
					CreateHzbOcclusionResource();
					CreateHzbOccCullingData();
					AddHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMapEx(true, true);
				}
				else
				{
					DeRegisterOccPassFrameData();
					DeRegisterHzbOccPassFrameData();
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
					DestroyCullingData(Graphic::J_CULLING_TYPE::HZB_OCCLUSION);
					PopHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMapEx(false, true);
				}
			}
			SetFrameDirty();
		}
		void SetAllowHdOcclusionCulling(const bool value)noexcept
		{
			SetAllowHdOcclusionCullingEx(value, false);
		}
		void SetAllowHdOcclusionCullingEx(const bool value, const bool justCallFunc)noexcept
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
					RegisterOccPassFrameData(); 
					CreateHdOccCullingData();
					AddHdOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMapEx(true, true);
				}
				else
				{
					DeRegisterOccPassFrameData();   
					DestroyCullingData(Graphic::J_CULLING_TYPE::HD_OCCLUSION);
					PopHdOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
					if (AllowDisplayOccCullingDepthMap())
						SetAllowDisplayOccCullingDepthMapEx(false, true);
				}
			}
			SetFrameDirty();
		}
		void SetAllowDisplayOccCullingDepthMap(const bool value)noexcept
		{
			SetAllowDisplayOccCullingDepthMapEx(value, false);
		}
		void SetAllowDisplayOccCullingDepthMapEx(const bool value, const bool justCallFunc)noexcept
		{
			if (!justCallFunc && allowDisplayOccCullingDepthMap == value)
				return;

			if(!justCallFunc)
				allowDisplayOccCullingDepthMap = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
					CreateOcclusionDepthDebug(allowHzbOcclusionCulling);
				else
					DestroyGraphicResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
			}
			SetFrameDirty();
		}
		void SetAllowAllCullingResult(const bool value)noexcept
		{
			allowAllCamCullResult = value;
			SetFrameDirty();
		}
		void SetAllowSsao(const bool value)
		{
			SetAllowSsaoEx(value, false);
		}
		void SetAllowSsaoEx(const bool value, const bool justCallFunc)
		{
			if (!justCallFunc && allowSsao == value)
				return;

			if (!justCallFunc)
				allowSsao = value;
			if (justCallFunc || thisPointer->IsActivated())
			{
				if (value)
				{
					auto gUser = GetInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
					CreateOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
					CreateOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP);
				}
				else
				{
					auto gUser = GetInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
					DestroyGraphicOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
					DestroyGraphicOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP);
				}
			}
			SetFrameDirty();
		}
		void SetCameraState(const J_CAMERA_STATE state)noexcept
		{
			if (camState == state)
				return;

			camState = state;
			if (camState == J_CAMERA_STATE::RENDER)
			{
				if (thisPointer->IsActivated() && RegisterComponent(thisPointer))
					Activate();
			}
			else
			{
				if (thisPointer->IsActivated() && DeRegisterComponent(thisPointer))
					DeActivate();
			}
			SetFrameDirty();
		}
		void SetRenderTargetRate(const JVector2F rate)noexcept
		{
			if (rtSizeRate == rate || rate.x == 0 || rate.y == 0)
				return;

			rtSizeRate = rate;
			if (HasResource(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON))
			{
				DeActivate();
				Activate();
			}
			SetFrameDirty();
		}
		void SetSsaoDesc(const Graphic::JSsaoDesc& newDesc)
		{
			if (ssaoDesc == newDesc)
				return;
			ssaoDesc = newDesc;
			if (thisPointer->IsActivated() && allowSsao)
			{
				auto gUser = GetInfo(Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON, 0);
				DestroyGraphicOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
				DestroyGraphicOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP);
				CreateOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::NORMAL_MAP);
				CreateOption(gUser, Graphic::J_GRAPHIC_RESOURCE_OPTION_TYPE::AMBIENT_OCCLISION_MAP);
			}
			SetFrameDirty();
		}
		void SetSsaoRadius(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.radius = std::clamp(value, Private::minSsaoRadius, Private::maxSsaoRadius);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoFadeStart(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.fadeStart = std::clamp(value, Private::minSsaoFade, newDesc.fadeEnd);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoFadeEnd(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.fadeEnd = std::clamp(value, newDesc.fadeStart, Private::maxSsaoFade);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoSurfaceEpsilon(const float value)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.surfaceEpsilon = std::clamp(value, Private::minSsaoEpsilon, Private::maxSsaoEpsilon);
			SetSsaoDesc(newDesc);
		}
		void SetSsaoType(const Graphic::J_SSAO_TYPE type)noexcept
		{
			Graphic::JSsaoDesc newDesc = ssaoDesc;
			newDesc.ssaoType = type;
			SetSsaoDesc(newDesc);
		}
	public:
		bool IsOrthoCamera()const noexcept
		{
			return isOrtho;
		}
		bool AllowDisplayDepthMap()const noexcept
		{
			return allowDisplayDepthMap;
		}
		bool AllowDisplayDebug()const noexcept
		{
			return allowDisplayDebug;
		}
		bool AllowFrustumCulling()const noexcept
		{
			return allowFrustumCulling;
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
		bool AllowAllCullingResult()const noexcept
		{
			return allowAllCamCullResult;
		}
		bool AllowOccCulling()
		{
			return AllowHzbOcclusionCulling() || AllowHdOcclusionCulling();
		}
		bool AllowSsao()const noexcept
		{
			return allowSsao;
		}
	public:
		void CalPerspectiveLens() noexcept
		{
			camNearViewHeight = 2.0f * camNear * tanf(0.5f * camFov);
			camFarViewHeight = 2.0f * camFar * tanf(0.5f * camFov);

			const XMMATRIX P = XMMatrixPerspectiveFovLH(camFov, camAspect, camNear, camFar);
			mProj.StoreXM(P); 
			BoundingFrustum::CreateFromMatrix(mCamFrustum, mProj.LoadXM());
			SetFrameDirty();
		}
		void CalOrthoLens() noexcept
		{
			//XMMatrixOrthographicOffCenterLH
			const XMMATRIX P = XMMatrixOrthographicLH(camOrthoViewWidth, camOrthoViewHeight, camNear, camFar);
			mProj.StoreXM(P);
			BoundingFrustum::CreateFromMatrix(mCamFrustum, mProj.LoadXM());
			SetFrameDirty();
		}
	public:
		void Activate()noexcept
		{  
			if (AllowHdOcclusionCulling() && AllowHzbOcclusionCulling())
				allowHdOcclusionCulling = allowHzbOcclusionCulling = false;

			//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			RegisterCameraFrameData();
			RegisterCsmTargetInterface();
			CreateRenderTargetTexture(GetRtSize());
			CreateSceneDepthStencil(GetRtSize());
			if (allowDisplayDepthMap)
				SetAllowDisplaySceneDepthMapEx(true, true);
			if (allowDisplayDebug)
				SetAllowDisplayDebugEx(true, true);
			if (allowFrustumCulling)
				SetAllowFrustumCullingEx(true, true);
			if (allowHzbOcclusionCulling)
				SetAllowHzbOcclusionCullingEx(true, true);
			if (allowHdOcclusionCulling)
				SetAllowHdOcclusionCullingEx(true, true); 
			if(allowSsao)
				SetAllowSsaoEx(true, true);
			if (thisPointer->GetOwner()->GetOwnerScene()->GetUseCaseType() == J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW)
				AddDrawSceneRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			else
				AddDrawSceneRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::ALWAYS);
		}
		void DeActivate()noexcept
		{ 
			//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			DeRegisterCameraFrameData();	
			DeRegisterCsmTargetInterface();
			PopDrawRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			if (allowDisplayDepthMap)
				SetAllowDisplaySceneDepthMapEx(false, true);
			if (allowDisplayDebug)
				SetAllowDisplayDebugEx(false, true);
			if (allowFrustumCulling)
				SetAllowFrustumCullingEx(false, true);
			if (allowHzbOcclusionCulling)
				SetAllowHzbOcclusionCullingEx(false, true);
			if (allowHdOcclusionCulling)
				SetAllowHdOcclusionCullingEx(false, true);
			if (allowSsao)
				SetAllowSsaoEx(false, true);
			//if (allowDisplayOccCullingDepthMap)
			//	SetAllowDisplayOccCullingDepthMapEx(false, true);
			 
			DestroyAllCullingData();
			DestroyAllTexture();
		}
	public:
		//has order dependency
		//1. update JCameraConstants
		//2. rest all
		void UpdateFrame(Graphic::JCameraConstants& constant)noexcept final
		{ 
			const XMMATRIX view = mView.LoadXM();
			const XMMATRIX proj = mProj.LoadXM();
			const XMMATRIX tex = JMatrix4x4::NdcToTextureSpaceXM();

			XMVECTOR viewVec = XMMatrixDeterminant(view);
			XMVECTOR projVec = XMMatrixDeterminant(proj);

			const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
			XMVECTOR viewProjVec = XMMatrixDeterminant(viewProj);
			const XMMATRIX invView = XMMatrixInverse(&viewVec, view);
			const XMMATRIX invProj = XMMatrixInverse(&projVec, proj);
			const XMMATRIX invViewProj = XMMatrixInverse(&viewProjVec, viewProj);
			  
			//constant.view.StoreXM(XMMatrixTranspose(view));
			//constant.invView.StoreXM(XMMatrixTranspose(invView));
			constant.proj.StoreXM(XMMatrixTranspose(proj));
			constant.projTex.StoreXM(XMMatrixTranspose(XMMatrixMultiply(proj, tex)));
			constant.viewProj.StoreXM(XMMatrixTranspose(viewProj));
			//constant.viewProjTex.StoreXM(XMMatrixTranspose(XMMatrixMultiply(viewProj, tex)));
			//constant.invProj.StoreXM(XMMatrixTranspose(invProj));
			//constant.invViewProj.StoreXM(XMMatrixTranspose(invViewProj));		 
			constant.renderTargetSize = JVector2<float>(GetRenderViewWidth(), GetRenderViewHeight());
			constant.invRenderTargetSize = JVector2<float>(1.0f / GetRenderViewWidth(), 1.0f / GetRenderViewHeight());
			constant.eyePosW = thisPointer->GetTransform()->GetPosition();
			constant.nearZ = camNear;
			constant.farZ = camFar; 
			if (HasCsmTargetInfo())
				constant.csmLocalIndex = GetCsmTargetIndex();
			CamFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JDepthTestPassConstants& constant)noexcept final
		{ 
			constant.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(mView.LoadXM(), mProj.LoadXM())));
			DepthTestFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JHzbOccComputeConstants& constant, const uint queryCount, const uint queryOffset)noexcept final
		{
			//static const BoundingBox drawBBox = _JResourceManager::Instance().Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE)->GetBoundingBox();
			auto info = JGraphic::Instance().GetGraphicInfo();
			auto option = JGraphic::Instance().GetGraphicOption();

			const XMMATRIX view = mView.LoadXM();
			const XMMATRIX proj = mProj.LoadXM();
			 
			constant.view.StoreXM(XMMatrixTranspose(view)); 
			constant.proj.StoreXM(XMMatrixTranspose(proj));
			constant.viewProj.StoreXM(XMMatrixTranspose(XMMatrixMultiply(view, proj)));
			 
			const BoundingFrustum frustum = GetBoundingFrustum();
			XMVECTOR planeV[6];
			frustum.GetPlanes(&planeV[0], &planeV[1], &planeV[2], &planeV[3], &planeV[4], &planeV[5]);
			for (uint i = 0; i < 6; ++i)
				XMStoreFloat4(&constant.frustumPlane[i], planeV[i]);
 
			constant.frustumDir = frustum.Orientation;
			constant.frustumPos = frustum.Origin;
			constant.viewWidth = GetRenderViewWidth();
			constant.viewHeight = GetRenderViewHeight();
			constant.camNear = camNear;
			constant.camFar = camFar;
			constant.validQueryCount = queryCount;
			constant.validQueryOffset = queryOffset;
			constant.occMapCount = info.occlusionMapCount;
			constant.occIndexOffset = JMathHelper::Log2Int(info.occlusionMinSize);
			constant.correctFailTrigger = (int)option.allowHZBCorrectFail; 
			constant.usePerspective = true;
			 
			HzbOccReqFrame::MinusMovedDirty();
		}
		void UpdateViewMatrix() noexcept
		{
			thisPointer->GetTransform()->CalTransformMatrix(mView);
		}
	public:
		static bool DoCopy(JCamera* from, JCamera* to)
		{
			to->impl->camNear = from->impl->camNear;
			to->impl->camFar = from->impl->camFar;
			to->impl->camAspect = from->impl->camAspect;
			to->impl->camFov = from->impl->camFov;
			to->impl->camNearViewHeight = from->impl->camNearViewHeight;
			to->impl->camFarViewHeight = from->impl->camFarViewHeight;
			to->impl->isOrtho = from->impl->isOrtho;
			if (!to->impl->isOrtho)
				to->impl->CalPerspectiveLens();
			else
				to->impl->CalOrthoLens();
			return true;
		}
	public:
		void NotifyReAlloc()
		{
			auto transform = thisPointer->GetOwner()->GetTransform();
			if (transform.IsValid())
			{
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(transform.Get(), thisPointer->GetGuid());
				JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(transform.Get(), this, thisPointer->GetGuid());
			}
			RegisterInterfacePointer();
			TrySetGetFrustumPtr();

			CamFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, (CamFrame*)this);
			HzbOccReqFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, (HzbOccReqFrame*)this);
		}
	public:
		void Initialize()
		{
			camFov = Constants::defaultCamFovDeg * JMathHelper::DegToRad;
			//camFov = 0.25f * JMathHelper::Pi;
			camNear = Constants::defaultCamFrustumNear;
			camFar = Constants::defaultCamFrustumFar;
			mView = JMatrix4x4::Identity();
			mProj = JMatrix4x4::Identity();
			 
			const JVector2F displaySize = JWindow::GetDisplaySize();
			SetAspect(displaySize.x / displaySize.y);
			SetOrthoViewWidth(displaySize.x);
			SetOrthoViewHeight(displaySize.y);
			//ownerTransform->SetPosition(XMFLOAT3(0.0f, 2.0f, -15.0f));
			CalPerspectiveLens();

			BoundingFrustum::CreateFromMatrix(mCamFrustum, mProj.LoadXM());
		}
		void RegisterThisPointer(JCamera* cam)
		{
			thisPointer = Core::GetWeakPtr(cam);
		}
		void RegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(this);
			Graphic::JCullingInterface::SetInterfacePointer(this);
			Graphic::JCsmTargetInterface::SetInterfacePointer(this);
		}
		void RegisterPostCreation()
		{
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), this, thisPointer->GetGuid());
		}
		void RegisterCameraFrameData()
		{
			CamFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, (CamFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void RegisterOccPassFrameData()
		{
			HzbOccReqFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, (DepthTestFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void RegisterHzbOccPassFrameData()
		{
			HzbOccReqFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, (HzbOccReqFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void DeRegisterPreDestruction()
		{
			if (thisPointer->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), thisPointer->GetGuid());
		}
		void DeRegisterCameraFrameData()
		{
			CamFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, (CamFrame*)this);
		}
		void DeRegisterOccPassFrameData()
		{
			HzbOccReqFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::DEPTH_TEST_PASS, (DepthTestFrame*)this);
		}
		void DeRegisterHzbOccPassFrameData()
		{
			HzbOccReqFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_COMPUTE_PASS, (HzbOccReqFrame*)this);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JCamera::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](const Core::JTypeInfo& typeInfo, JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JCamera::InitData>(ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JCamera::InitData>(parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static auto setFrameLam = [](JComponent* component) {static_cast<JCamera*>(component)->impl->SetFrameDirty(); };
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable, isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

			JComponent::RegisterCTypeInfo(JCamera::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JCamera::StaticTypeInfo(), cPrivate);

			IMPL_REALLOC_BIND(JCamera::JCameraImpl, thisPointer)
		}
	};

	JCamera::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(JCamera::StaticTypeInfo(), owner)
	{}
	JCamera::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(JCamera::StaticTypeInfo(), GetDefaultName(JCamera::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JCamera::PrivateInterface()const noexcept
	{
		return cPrivate;
	}
	const Graphic::JGraphicResourceUserInterface JCamera::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface{ impl.get() };
	}
	const Graphic::JCullingUserInterface JCamera::CullingUserInterface()const noexcept
	{
		return Graphic::JCullingUserInterface{ impl.get() };
	}
	J_COMPONENT_TYPE JCamera::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JUserPtr<JTransform> JCamera::GetTransform()noexcept
	{
		return GetOwner()->GetTransform();
	}
	XMMATRIX JCamera::GetView()const noexcept
	{
		return impl->mView.LoadXM();
	}
	JMatrix4x4 JCamera::GetView4x4()const noexcept
	{
		return impl->mView;
	}
	XMMATRIX JCamera::GetProj()const noexcept
	{
		return impl->mProj.LoadXM();
	} 
	JMatrix4x4 JCamera::GetProj4x4()const noexcept
	{
		return impl->mProj;
	} 
	DirectX::BoundingFrustum JCamera::GetBoundingFrustum()const noexcept
	{
		return impl->GetBoundingFrustum();
	}   
	DirectX::BoundingFrustum JCamera::GetLocalBoundingLoFrustum()const noexcept
	{
		return impl->mCamFrustum;
	}
	float JCamera::GetNear()const noexcept
	{
		return impl->GetNear();
	}
	float JCamera::GetFar()const noexcept
	{
		return impl->GetFar();
	}
	float JCamera::GetFovX()const noexcept
	{
		return impl->GetFovX();
	}
	float JCamera::GetFovXDegree()const noexcept
	{
		return impl->GetFovXDegree();
	}
	float JCamera::GetFovY()const noexcept
	{
		return impl->camFov;
	}
	float JCamera::GetFovYDegree()const noexcept
	{
		return impl->GetFovYDegree();
	}
	float JCamera::GetAspect()const noexcept
	{
		return impl->camAspect;
	}
	float JCamera::GetOrthoViewWidth()const noexcept
	{
		return impl->GetOrthoViewWidth();
	}
	float JCamera::GetOrthoViewHeight()const noexcept
	{
		return impl->GetOrthoViewHeight();
	}
	float JCamera::GetNearViewWidth()const noexcept
	{
		return impl->GetNearViewWidth();
	}
	float JCamera::GetNearViewHeight()const noexcept
	{
		return impl->camNearViewHeight;
	}
	float JCamera::GetFarViewWidth()const noexcept
	{
		return impl->GetFarViewWidth();
	}
	float JCamera::GetFarViewHeight()const noexcept
	{
		return impl->camFarViewHeight;
	}
	J_CAMERA_STATE JCamera::GetCameraState()const noexcept
	{
		return impl->camState;
	}
	JVector2F JCamera::GetRenderTargetRate()const noexcept
	{
		return impl->rtSizeRate;
	}
	Graphic::JSsaoDesc JCamera::GetSsaoDesc()const noexcept
	{
		return impl->ssaoDesc;
	}
	void JCamera::SetNear(float value)noexcept
	{
		impl->SetNear(value);
	}
	void JCamera::SetFar(float value) noexcept
	{
		impl->SetFar(value);
	}
	void JCamera::SetFov(float value) noexcept
	{
		impl->SetFovY(value);
	}
	void JCamera::SetFovDegree(float value) noexcept
	{
		impl->SetFovYDegree(value);
	}
	void JCamera::SetAspect(const float value) noexcept
	{
		impl->SetAspect(value);
	}
	void JCamera::SetOrthoViewWidth(const float value) noexcept
	{
		impl->SetOrthoViewWidth(value);
	}
	void JCamera::SetOrthoViewHeight(const float value) noexcept
	{
		impl->SetOrthoViewHeight(value);
	}
	void JCamera::SetOrthoViewSize(const float width, const float height) noexcept
	{
		impl->SetOrthoViewSize(width, height);
	}
	void JCamera::SetOrthoCamera(bool value)noexcept
	{
		impl->SetOrthoCamera(value);
	}
	void JCamera::SetAllowDisplaySceneDepthMap(const bool value)noexcept
	{
		impl->SetAllowDisplaySceneDepthMap(value);
	}
	void JCamera::SetAllowDisplayDebug(const bool value)noexcept
	{
		impl->SetAllowDisplayDebug(value);
	}
	void JCamera::SetAllowFrustumCulling(const bool value)noexcept
	{
		impl->SetAllowFrustumCulling(value);
	}
	void JCamera::SetAllowHzbOcclusionCulling(const bool value)noexcept
	{
		impl->SetAllowHzbOcclusionCulling(value);
	}
	void JCamera::SetAllowHdOcclusionCulling(const bool value)noexcept
	{
		impl->SetAllowHdOcclusionCulling(value);
	}
	void JCamera::SetAllowDisplayOccCullingDepthMap(const bool value)noexcept
	{
		impl->SetAllowDisplayOccCullingDepthMap(value);
	}
	void JCamera::SetAllowSsao(const bool value)noexcept
	{
		impl->SetAllowSsao(value);
	}
	void JCamera::SetCameraState(const J_CAMERA_STATE state)noexcept
	{
		impl->SetCameraState(state);
	}
	void JCamera::SetRenderTargetRate(const JVector2F rate)noexcept
	{
		impl->SetRenderTargetRate(rate);
	}
	void JCamera::SetSsaoDesc(const Graphic::JSsaoDesc& desc)noexcept
	{
		impl->SetSsaoDesc(desc);
	}
	bool JCamera::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
	}
	bool JCamera::IsOrthoCamera()const noexcept
	{
		return impl->isOrtho;
	} 
	bool JCamera::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JCamera::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && impl->camState == J_CAMERA_STATE::RENDER)
			return true;
		else
			return false;
	}
	bool JCamera::AllowDisplayDepthMap()const noexcept
	{
		return impl->AllowDisplayDepthMap();
	}
	bool JCamera::AllowDisplayDebug()const noexcept
	{
		return impl->AllowDisplayDebug();
	}
	bool JCamera::AllowFrustumCulling()const noexcept
	{
		return impl->AllowFrustumCulling();
	}
	bool JCamera::AllowHzbOcclusionCulling()const noexcept
	{
		return impl->AllowHzbOcclusionCulling();
	}
	bool JCamera::AllowHdOcclusionCulling()const noexcept
	{
		return impl->AllowHdOcclusionCulling();
	}
	bool JCamera::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
	}
	bool JCamera::AllowSsao()const noexcept
	{
		return impl->AllowSsao();
	}
	void JCamera::DoActivate()noexcept
	{
		JComponent::DoActivate();
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			if (RegisterComponent(impl->thisPointer))
				impl->Activate();
		}
		impl->SetFrameDirty();
	}
	void JCamera::DoDeActivate()noexcept
	{
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			if (DeRegisterComponent(impl->thisPointer))
				impl->DeActivate();
		}
		impl->OffFrameDirty();
		JComponent::DoDeActivate();
	}
	JCamera::JCamera(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JCameraImpl>(initData, this))
	{}
	JCamera::~JCamera()
	{ 
		impl.reset();
	}

	using CreateInstanceInterface = JCameraPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JCameraPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JCameraPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JCameraPrivate::FrameIndexInterface;
	using EditorSettingInterface = JCameraPrivate::EditorSettingInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JCamera>(*static_cast<JCamera::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JCamera* cam = static_cast<JCamera*>(createdPtr);
		cam->impl->RegisterThisPointer(cam);
		cam->impl->RegisterInterfacePointer();
		cam->impl->RegisterPostCreation();
		cam->impl->Initialize();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JCamera::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JCamera::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JCamera::JCameraImpl::DoCopy(static_cast<JCamera*>(from.Get()), static_cast<JCamera*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)noexcept
	{
		static_cast<JCamera*>(ptr)->impl->DeRegisterPreDestruction();
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{ 
		if (!Core::JDITypeDataBase::IsValidChildData(data, JCamera::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;

		J_CAMERA_STATE camState;
		JVector3F pos = JVector3F::Zero();
		float camNear = 0;
		float camFar = 0;
		float camFov = 0;
		float camAspect = 0;
		float camOrthoViewWidth = 0;
		float camOrthoViewHeight = 0;
		bool isOrtho = false;
		bool allowDisplayDepthMap = false;
		bool allowDisplayDebug = false;
		bool allowFrustumCulling = false;
		bool allowHzbOcclusionCulling = false;
		bool allowHdOcclusionCulling = false;
		bool allowDisplayOccCullingDepthMap = false;
		bool allowSsao = false;
		JVector2F rtSizeRate = JVector2F::One();
		Graphic::JSsaoDesc ssaoDesc;

		auto loadData = static_cast<JCamera::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		JObjectFileIOHelper::LoadEnumData(tool, camState, "CamState:");
		JObjectFileIOHelper::LoadVector3(tool, pos, "Pos:");
		JObjectFileIOHelper::LoadAtomicData(tool, camNear, "CamNear:");
		JObjectFileIOHelper::LoadAtomicData(tool, camFar, "CamFar:");
		JObjectFileIOHelper::LoadAtomicData(tool, camFov, "CamFov:");
		JObjectFileIOHelper::LoadAtomicData(tool, camAspect, "CamAspect:");
		JObjectFileIOHelper::LoadAtomicData(tool, camOrthoViewWidth, "camOrthoViewWidth:");
		JObjectFileIOHelper::LoadAtomicData(tool, camOrthoViewHeight, "CamOrthoViewHeight:");
		JObjectFileIOHelper::LoadAtomicData(tool, isOrtho, "IsOrtho:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowDisplayDepthMap, "AllowDepthMap:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowDisplayDebug, "AllowDebug:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowFrustumCulling, "AllowFrustumCulling:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowHzbOcclusionCulling, "AllowHzbOcclusionCulling:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowHdOcclusionCulling, "AllowHdOcclusionCulling:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowDisplayOccCullingDepthMap, "AllowDislplayCullingDepthMap:");
		JObjectFileIOHelper::LoadAtomicData(tool, allowSsao, "AllowSsao:");
		JObjectFileIOHelper::LoadVector2(tool, rtSizeRate, "RtSizeRate:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.radius, "SsaoRadius:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.fadeStart, "SsaoFadeStart:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.fadeEnd, "SsaoFadeEnd:");
		JObjectFileIOHelper::LoadAtomicData(tool, ssaoDesc.surfaceEpsilon, "SsaoSurfaceEpsilon:");
		JObjectFileIOHelper::LoadEnumData(tool, ssaoDesc.ssaoType, "SsaoType:");

		auto initData = std::make_unique<JCamera::InitData>(guid, flag, owner);
		initData->rtSizeRate = rtSizeRate;

		auto idenUser = cPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &cPrivate);
		JUserPtr<JCamera> camUser;
		camUser.ConnnectChild(idenUser);

		JCamera::JCameraImpl* impl = camUser->impl.get();
		impl->camNear = camNear;
		impl->camFar = camFar;
		camUser->SetNear(camNear);
		camUser->SetFar(camFar);

		impl->camFov = camFov;
		impl->camAspect = camAspect;
		impl->camOrthoViewWidth = camOrthoViewWidth;
		impl->camOrthoViewHeight = camOrthoViewHeight;
		camUser->GetTransform()->SetPosition(pos);

		if (isOrtho)
			impl->CalOrthoLens();
		else
			impl->CalPerspectiveLens();

		impl->SetSsaoDesc(ssaoDesc);
		impl->SetAllowDisplaySceneDepthMap(allowDisplayDepthMap);
		impl->SetAllowDisplayDebug(allowDisplayDebug);
		impl->SetAllowFrustumCulling(allowFrustumCulling);
		impl->SetAllowHzbOcclusionCulling(allowHzbOcclusionCulling);
		impl->SetAllowHdOcclusionCulling(allowHdOcclusionCulling);
		impl->SetAllowDisplayOccCullingDepthMap(allowDisplayOccCullingDepthMap);
		impl->SetAllowSsao(allowSsao);
		impl->SetRenderTargetRate(rtSizeRate);
		impl->SetCameraState(camState);
		if (!isActivated)
			camUser->DeActivate();

		return camUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JCamera::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JCamera::StoreData*>(data);
		if (!storeData->HasCorrectType(JCamera::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JCamera> cam;
		cam.ConnnectChild(storeData->obj);
		JCamera::JCameraImpl* impl = cam->impl.get();
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, cam.Get());
		JObjectFileIOHelper::StoreEnumData(tool, impl->camState, "CamState:");
		JObjectFileIOHelper::StoreVector3(tool, cam->GetTransform()->GetPosition(), "Pos:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camNear, "CamNear:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camFar, "CamFar:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camFov, "CamFov:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camAspect, "CamAspect:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camOrthoViewWidth, "camOrthoViewWidth:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->camOrthoViewHeight, "CamOrthoViewHeight:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->isOrtho, "IsOrtho:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowDisplayDepthMap, "AllowDepthMap:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowDisplayDebug, "AllowDebug:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowFrustumCulling, "AllowFrustumCulling:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowHzbOcclusionCulling, "AllowHzbOcclusionCulling:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowHdOcclusionCulling, "AllowHdOcclusionCulling:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowDisplayOccCullingDepthMap, "AllowDislplayCullingDepthMap:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->allowSsao, "AllowSsao:");
		JObjectFileIOHelper::StoreVector2(tool, impl->rtSizeRate, "RtSizeRate:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.radius, "SsaoRadius:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.fadeStart, "SsaoFadeStart:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.fadeEnd, "SsaoFadeEnd:");
		JObjectFileIOHelper::StoreAtomicData(tool, impl->ssaoDesc.surfaceEpsilon, "SsaoSurfaceEpsilon:");
		JObjectFileIOHelper::StoreEnumData(tool, impl->ssaoDesc.ssaoType, "SsaoType:");
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JCamera* cam, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			cam->impl->SetFrameDirty();

		cam->impl->SetLastFrameUpdatedTrigger(false);
		cam->impl->SetLastFrameHotUpdatedTrigger(false);
		if(cam->impl->IsFrameDirted())
			cam->impl->UpdateViewMatrix();
		return cam->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JCamera* cam, Graphic::JCameraConstants& constants)noexcept
	{
		cam->impl->UpdateFrame(constants);
	}
	void FrameUpdateInterface::UpdateFrame(JCamera* cam, Graphic::JDepthTestPassConstants& constant)noexcept
	{
		cam->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JCamera* cam, Graphic::JHzbOccComputeConstants& constants, const uint queryCount, const uint queryOffset)noexcept
	{
		cam->impl->UpdateFrame(constants, queryCount, queryOffset);
	}
	void FrameUpdateInterface::UpdateEnd(JCamera* cam)noexcept
	{
		if (cam->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			cam->impl->SetLastFrameHotUpdatedTrigger(true);
		cam->impl->SetLastFrameUpdatedTrigger(true);
		cam->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetCamFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->CamFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetDepthTestPassFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->DepthTestFrame::GetFrameIndex();
	}
	int FrameUpdateInterface::GetHzbOccComputeFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->HzbOccReqFrame::GetFrameIndex();
	}
	bool FrameUpdateInterface::IsLastFrameHotUpdated(JCamera* cam)noexcept
	{
		return cam->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JCamera* cam)noexcept
	{
		return cam->impl->IsLastFrameUpdated();
	}
	bool FrameUpdateInterface::HasCamRecopyRequest(JCamera* cam)noexcept
	{
		return cam->impl->CamFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasDepthTestPassRecopyRequest(JCamera* cam)noexcept
	{
		return cam->impl->DepthTestFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasHzbOccComputeRecopyRequest(JCamera* cam)noexcept
	{
		return cam->impl->HzbOccReqFrame::HasMovedDirty();
	}

	int FrameIndexInterface::GetCamFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->CamFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetDepthTestPassFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->DepthTestFrame::GetFrameIndex();
	}
	int FrameIndexInterface::GetHzbOccComputeFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->HzbOccReqFrame::GetFrameIndex();
	}

	void EditorSettingInterface::SetAllowAllCullingResult(const JUserPtr<JCamera>& cam, const bool value)noexcept
	{
		cam->impl->SetAllowAllCullingResult(value);
	}
	bool EditorSettingInterface::AllowAllCullingResult(const JUserPtr<JCamera>& cam)noexcept
	{
		return cam->impl->AllowAllCullingResult();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JCameraPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JCameraPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JCameraPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}
