#include"JCamera.h"
#include"JCameraPrivate.h"
#include"../JComponentHint.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../../GameObject/JGameObject.h"  
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"  
#include"../../../Core/Guid/JGuidCreator.h"  
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/JGraphicDrawListInterface.h"
#include"../../../Graphic/Upload/Frameresource/JCameraConstants.h"    
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/Culling/JCullingConstants.h" 
#include"../../../Graphic/Upload/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Upload/Frameresource/JOcclusionConstants.h"
#include"../../../Window/JWindow.h"
#include"../../../Utility/JMathHelper.h"
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
		using CameraFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder2<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, Graphic::JCameraConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_REQUESTOR, Graphic::JHzbOccRequestorConstants&, const uint, const uint>>,
			Graphic::JFrameDirty>;
		//first extra is occlusion 
	}

	class JCamera::JCameraImpl : public Core::JTypeImplBase,
		public CameraFrameUpdate,
		public Graphic::JGraphicMultiResourceInterface,
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCullingInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JCameraImpl)
		REGISTER_GUI_BOOL_CONDITION(IsOrthoCam, isOrtho)
	public:
		using CamFrame = JFrameInterface1;
		using HzbOccReqFrame = JFrameInterface2;
	public:
		JWeakPtr<JCamera> thisPointer = nullptr;
	public:
		//JTransform* ownerTransform;
		J_CAMERA_STATE camState = J_CAMERA_STATE::RENDER;
		// Cache frustum properties.
		REGISTER_PROPERTY_EX(camNear, GetNear, SetNear, GUI_SLIDER(0, 1000, true))
		float camNear = 0.0f;
		REGISTER_PROPERTY_EX(camFar, GetFar, SetFar, GUI_SLIDER(0, 1000, true))
		float camFar = 0.0f;
		REGISTER_PROPERTY_EX(camAspect, GetAspect, SetAspect, GUI_SLIDER(0.1f, 32.0f, true, false, GUI_BOOL_CONDITION_USER(IsOrthoCam, false)))
		float camAspect = 0.0f;		// Perspective일때 사용
		REGISTER_PROPERTY_EX(camFov, GetFovYDegree, SetFovYDegree, GUI_SLIDER(0, 360, true))
		float camFov = 0.0f;	//fovY
	public:
		REGISTER_PROPERTY_EX(camOrthoViewWidth, GetOrthoViewWidth, SetOrthoViewWidth, GUI_SLIDER(1, 3840, true, false, GUI_BOOL_CONDITION_USER(IsOrthoCam, true)))
		float camOrthoViewWidth = 0.0f;		// Ortho일때 사용
		REGISTER_PROPERTY_EX(camOrthoViewHeight, GetOrthoViewHeight, SetOrthoViewHeight, GUI_SLIDER(1, 2160, true, false, GUI_BOOL_CONDITION_USER(IsOrthoCam, true)))
		float camOrthoViewHeight = 0.0f;	// Ortho일때 사용
		float camNearViewHeight = 0.0f;
		float camFarViewHeight = 0.0f;
	public:
		REGISTER_PROPERTY_EX(isOrtho, IsOrthoCamera, SetOrthoCamera, GUI_CHECKBOX())
		bool isOrtho = false;
		REGISTER_PROPERTY_EX(allowDisplayDepthMap, AllowDisplayDepthMap, SetAllowDisplayDepthMap, GUI_CHECKBOX())
		bool allowDisplayDepthMap = false;
		REGISTER_PROPERTY_EX(allowDisplayDebug, AllowDisplayDebug, SetAllowDisplayDebug, GUI_CHECKBOX())
		bool allowDisplayDebug = false;
		REGISTER_PROPERTY_EX(allowFrustumCulling, AllowFrustumCulling, SetAllowFrustumCulling, GUI_CHECKBOX())
		bool allowFrustumCulling = false;
		REGISTER_PROPERTY_EX(allowHzbOcclusionCulling, AllowHzbOcclusionCulling, SetAllowHzbOcclusionCulling, GUI_CHECKBOX())
		bool allowHzbOcclusionCulling = false;
		REGISTER_PROPERTY_EX(allowDisplayOccCullingDepthMap, AllowDisplayOccCullingDepthMap, SetAllowDisplayOccCullingDepthMap, GUI_CHECKBOX())
		bool allowDisplayOccCullingDepthMap = false;
		bool allowAllCamCullResult = false;	//use editor cam for check space spatial result
	public:
		//Culling Option
		REGISTER_PROPERTY_EX(frustumCulingFrequency, GetFrustumCullingFrequency, SetFrustumCullingFrequency, GUI_SLIDER(Graphic::Constants::cullingUpdateFrequencyMin, Graphic::Constants::cullingUpdateFrequencyMax, true))
		float frustumCulingFrequency = 0;
		REGISTER_PROPERTY_EX(occlusionCulingFrequency, GetOcclusionCullingFrequency, SetOcclusionCullingFrequency, GUI_SLIDER(Graphic::Constants::cullingUpdateFrequencyMin, Graphic::Constants::cullingUpdateFrequencyMax, true))
		float occlusionCulingFrequency = 0;
	public:
		// Cache View/Proj matrices.
		DirectX::XMFLOAT4X4 mView;
		DirectX::XMFLOAT4X4 mProj;
		DirectX::BoundingFrustum mCamFrustum;
	public:
		//Caution
		//Impl생성자에서 interface class 참조시 interface class가 함수내에서 impl을 참조할 경우 error
		//impl이 아직 생성되지 않았으므로
		JCameraImpl(const InitData& initData, JCamera* thisCamRaw) { }
		~JCameraImpl() {}
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
		DirectX::BoundingFrustum GetBoundingFrustum()const noexcept
		{
			DirectX::BoundingFrustum worldCamFrustum;
			mCamFrustum.Transform(worldCamFrustum, thisPointer->GetOwner()->GetTransform()->GetWorldMatrix());
			return worldCamFrustum;
		}
	public:
		void SetNear(const float value)noexcept
		{
			camNear = value;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFar(const float value) noexcept
		{
			camFar = value;
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
		void SetAllowDisplayDepthMap(const bool value)noexcept
		{
			if (allowDisplayDepthMap == value)
				return;

			allowDisplayDepthMap = value;
			if (thisPointer->IsActivated())
			{
				if (allowDisplayDepthMap)
					CreateSceneDepthStencilDebug();
				else
					DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG);
			}
			SetFrameDirty();
		}
		void SetAllowDisplayDebug(const bool value)noexcept
		{
			if (allowDisplayDebug == value)
				return;

			allowDisplayDebug = value;
			if (thisPointer->IsActivated())
			{
				if (allowDisplayDebug)
					CreateDebugDepthStencil();
				else
					DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::SCENE_DEPTH_STENCIL_DEBUG);
			}
			SetFrameDirty();
		}
		void SetAllowFrustumCulling(const bool value)noexcept
		{
			if (allowFrustumCulling == value)
				return;

			allowFrustumCulling = value;
			if (thisPointer->IsActivated())
			{
				if (allowFrustumCulling)
				{
					CreateFrustumCullingData();
					AddFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
				}
				else
				{
					DestroyCullingData(this, Graphic::J_CULLING_TYPE::FRUSTUM);
					PopFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
				}
			}
			SetFrameDirty();
		}
		void SetAllowHzbOcclusionCulling(const bool value)noexcept
		{
			if (allowHzbOcclusionCulling == value)
				return;

			allowHzbOcclusionCulling = value;
			if (thisPointer->IsActivated())
			{
				if (allowHzbOcclusionCulling)
				{
					RegisterOccPassFrameData();
					CreateOccCullingData(this);
					AddHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
				}
				else
				{
					DeRegisterOccPassFrameData();
					DestroyCullingData(this, Graphic::J_CULLING_TYPE::OCCLUSION);
					PopHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
				}
			}
			SetFrameDirty();
		}
		void SetAllowDisplayOccCullingDepthMap(const bool value)noexcept
		{
			if (allowDisplayOccCullingDepthMap == value)
				return;

			allowDisplayOccCullingDepthMap = value;
			if (thisPointer->IsActivated())
			{
				if (allowDisplayOccCullingDepthMap)
					CreateOcclusionDepthDebug();
				else
					DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP_DEBUG);
			}
			SetFrameDirty();
		}
		void SetAllowAllCullingResult(const bool value)noexcept
		{
			allowAllCamCullResult = value;
			SetFrameDirty();
		}
		void SetFrustumCullingFrequency(const float value)noexcept
		{
			frustumCulingFrequency = value;
		}
		void SetOcclusionCullingFrequency(const float value)noexcept
		{
			occlusionCulingFrequency = value;
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
		bool AllowDisplayOccCullingDepthMap()const noexcept
		{
			return allowDisplayOccCullingDepthMap;
		}
		bool AllowAllCullingResult()const noexcept
		{
			return allowAllCamCullResult;
		}
	public:
		void CalPerspectiveLens() noexcept
		{
			camNearViewHeight = 2.0f * camNear * tanf(0.5f * camFov);
			camFarViewHeight = 2.0f * camFar * tanf(0.5f * camFov);

			const XMMATRIX P = XMMatrixPerspectiveFovLH(camFov, camAspect, camNear, camFar);
			XMStoreFloat4x4(&mProj, P);
			BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
			SetFrameDirty();
		}
		void CalOrthoLens() noexcept
		{
			//XMMatrixOrthographicOffCenterLH
			const XMMATRIX P = XMMatrixOrthographicLH(camOrthoViewWidth, camOrthoViewHeight, camNear, camFar);
			XMStoreFloat4x4(&mProj, P);
			BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
			SetFrameDirty();
		}
	public:
		void Activate()noexcept
		{
			//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			RegisterCameraFrameData();
			CreateRenderTargetTexture();
			CreateSceneDepthStencil();
			if (allowDisplayDepthMap)
				CreateSceneDepthStencilDebug();
			if (allowDisplayDebug)
				CreateDebugDepthStencil();
			if (allowFrustumCulling)
			{
				CreateFrustumCullingData();
				AddFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			}
			if (allowHzbOcclusionCulling)
			{
				RegisterOccPassFrameData();
				CreateOccCullingData(this);
				AddHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			}
			if (allowDisplayOccCullingDepthMap)
				CreateOcclusionDepthDebug();

			if (thisPointer->GetOwner()->GetOwnerScene()->GetUseCaseType() == J_SCENE_USE_CASE_TYPE::TWO_DIMENSIONAL_PREVIEW)
				AddDrawSceneRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			else
				AddDrawSceneRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::ALWAYS);
		}
		void DeActivate()noexcept
		{
			//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			DeRegisterCameraFrameData();
			DeRegisterOccPassFrameData();
			PopDrawRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			PopHzbOccCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			PopFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			DestroyAllCullingData(this);
			DestroyAllTexture();
		}
	public:
		void UpdateFrame(Graphic::JCameraConstants& constant)noexcept final
		{
			UpdateViewMatrix();
			const XMMATRIX view = XMLoadFloat4x4(&mView);
			const XMMATRIX proj = XMLoadFloat4x4(&mProj);

			XMVECTOR viewVec = XMMatrixDeterminant(view);
			XMVECTOR projVec = XMMatrixDeterminant(proj);

			const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
			const XMMATRIX invView = XMMatrixInverse(&viewVec, view);
			const XMMATRIX invProj = XMMatrixInverse(&projVec, proj);
			XMVECTOR viewProjVec = XMMatrixDeterminant(viewProj);
			const XMMATRIX invViewProj = XMMatrixInverse(&viewProjVec, viewProj);

			XMStoreFloat4x4(&constant.view, XMMatrixTranspose(view));
			XMStoreFloat4x4(&constant.invView, XMMatrixTranspose(invView));
			XMStoreFloat4x4(&constant.proj, XMMatrixTranspose(proj));
			XMStoreFloat4x4(&constant.invProj, XMMatrixTranspose(invProj));
			XMStoreFloat4x4(&constant.viewProj, XMMatrixTranspose(viewProj));
			XMStoreFloat4x4(&constant.invViewProj, XMMatrixTranspose(invViewProj));
			constant.renderTargetSize = XMFLOAT2(GetRenderViewWidth(), GetRenderViewHeight());
			constant.invRenderTargetSize = XMFLOAT2(1.0f / GetRenderViewWidth(), 1.0f / GetRenderViewHeight());
			constant.eyePosW = thisPointer->GetTransform()->GetPosition();
			constant.nearZ = camNear;
			constant.farZ = camFar;
			CamFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JHzbOccRequestorConstants& constant, const uint queryCount, const uint queryOffset)noexcept final
		{
			//static const BoundingBox drawBBox = _JResourceManager::Instance().Instance().GetDefaultMeshGeometry(J_DEFAULT_SHAPE::BOUNDING_BOX_TRIANGLE)->GetBoundingBox();
			auto info = JGraphic::Instance().GetGraphicInfo();
			auto option = JGraphic::Instance().GetGraphicOption();

			const XMMATRIX view = XMLoadFloat4x4(&mView);
			const XMMATRIX proj = XMLoadFloat4x4(&mProj);
			XMStoreFloat4x4(&constant.view, XMMatrixTranspose(view));
			XMStoreFloat4x4(&constant.proj, XMMatrixTranspose(proj));
			XMStoreFloat4x4(&constant.viewProj, XMMatrixTranspose(XMMatrixMultiply(view, proj)));

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
			HzbOccReqFrame::MinusMovedDirty();
		}
		void UpdateViewMatrix() noexcept
		{
			//if (gameObjectDirty->GetTransformDirty() > 0)
			{
				JTransform* ownerTransform = thisPointer->GetTransform().Get();
				XMVECTOR R = ownerTransform->GetRight();
				XMVECTOR U = ownerTransform->GetUp();
				XMVECTOR L = ownerTransform->GetFront();
				XMFLOAT3 pos = ownerTransform->GetPosition();
				XMVECTOR P = XMLoadFloat3(&pos);

				// Fill in the view matrix entries.
				const float x = -XMVectorGetX(XMVector3Dot(P, R));
				const float y = -XMVectorGetX(XMVector3Dot(P, U));
				const float z = -XMVectorGetX(XMVector3Dot(P, L));

				XMFLOAT3 rightVector;
				XMFLOAT3 upVector;
				XMFLOAT3 lookVector;

				XMStoreFloat3(&rightVector, R);
				XMStoreFloat3(&upVector, U);
				XMStoreFloat3(&lookVector, L);

				mView(0, 0) = rightVector.x;
				mView(1, 0) = rightVector.y;
				mView(2, 0) = rightVector.z;
				mView(3, 0) = x;

				mView(0, 1) = upVector.x;
				mView(1, 1) = upVector.y;
				mView(2, 1) = upVector.z;
				mView(3, 1) = y;

				mView(0, 2) = lookVector.x;
				mView(1, 2) = lookVector.y;
				mView(2, 2) = lookVector.z;
				mView(3, 2) = z;

				mView(0, 3) = 0.0f;
				mView(1, 3) = 0.0f;
				mView(2, 3) = 0.0f;
				mView(3, 3) = 1.0f;
			}
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
			CamFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, (CamFrame*)this);
			HzbOccReqFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_REQUESTOR, (HzbOccReqFrame*)this);
		}
	public:
		void Initialize()
		{
			camFov = 60.0f * JMathHelper::DegToRad;
			//camFov = 0.25f * JMathHelper::Pi;
			camNear = 1;
			camFar = 1000;
			mView = JMathHelper::Identity4x4();
			mProj = JMathHelper::Identity4x4();

			const float disWidth = JWindow::GetDisplayWidth();
			const float disHeight = JWindow::GetDisplayHeight(); 

			SetAspect(disWidth / disHeight);
			SetOrthoViewWidth(disWidth);
			SetOrthoViewHeight(disHeight);
			//ownerTransform->SetPosition(XMFLOAT3(0.0f, 2.0f, -15.0f));
			CalPerspectiveLens();

			BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
		}
		void RegisterThisPointer(JCamera* cam)
		{
			thisPointer = Core::GetWeakPtr(cam);
		}
		void RegisterPostCreation()
		{
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), this, thisPointer->GetGuid());
		}
		void DeRegisterPreDestruction()
		{
			if (thisPointer->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisPointer->GetOwner()->GetTransform().Get(), thisPointer->GetGuid());
		}
		void RegisterCameraFrameData()
		{
			CamFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, (CamFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void DeRegisterCameraFrameData()
		{
			CamFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::CAMERA, (CamFrame*)this);
		}
		void RegisterOccPassFrameData()
		{
			HzbOccReqFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_REQUESTOR, (HzbOccReqFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void DeRegisterOccPassFrameData()
		{
			HzbOccReqFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_REQUESTOR, (HzbOccReqFrame*)this);
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
	Graphic::JFrameUpdateUserAccess JCamera::FrameUserInterface() noexcept
	{
		return Graphic::JFrameUpdateUserAccess(Core::GetUserPtr(this), impl.get());
	}
	const Graphic::JGraphicMultiResourceUserInterface JCamera::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicMultiResourceUserInterface{ impl.get() };
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
		return XMLoadFloat4x4(&impl->mView);
	}
	XMMATRIX JCamera::GetProj()const noexcept
	{
		return XMLoadFloat4x4(&impl->mProj);
	}
	XMFLOAT4X4 JCamera::GetView4x4f()const noexcept
	{
		return impl->mView;
	}
	XMFLOAT4X4 JCamera::GetProj4x4f()const noexcept
	{
		return impl->mProj;
	}
	DirectX::BoundingFrustum JCamera::GetBoundingFrustum()const noexcept
	{
		return impl->GetBoundingFrustum();
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
	void JCamera::SetAllowDisplayDepthMap(const bool value)noexcept
	{
		impl->SetAllowDisplayDepthMap(value);
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
	void JCamera::SetAllowDisplayOccCullingDepthMap(const bool value)noexcept
	{
		impl->SetAllowDisplayOccCullingDepthMap(value);
	}
	void JCamera::SetCameraState(const J_CAMERA_STATE state)noexcept
	{
		impl->SetCameraState(state);
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
	bool JCamera::AllowDisplayOccCullingDepthMap()const noexcept
	{
		return impl->AllowDisplayOccCullingDepthMap();
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
		JComponent::DoDeActivate();
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			if (DeRegisterComponent(impl->thisPointer))
				impl->DeActivate();
		}
		impl->OffFrameDirty();
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
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
		static_cast<JCamera*>(ptr)->impl->DeRegisterPreDestruction();
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JCamera::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		J_CAMERA_STATE camState;
		XMFLOAT3 pos;
		float camNear;
		float camFar;
		float camFov;
		float camAspect;
		float camOrthoViewWidth;
		float camOrthoViewHeight;
		bool isOrtho;
		bool allowDisplayDepthMap;
		bool allowDisplayDebug;
		bool allowFrustumCulling;
		bool allowHzbOcclusionCulling;
		bool allowDisplayOccCullingDepthMap;

		auto loadData = static_cast<JCamera::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JUserPtr<JGameObject> owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadEnumData(stream, camState);
		JFileIOHelper::LoadXMFloat3(stream, pos);
		JFileIOHelper::LoadAtomicData(stream, camNear);
		JFileIOHelper::LoadAtomicData(stream, camFar);
		JFileIOHelper::LoadAtomicData(stream, camFov);
		JFileIOHelper::LoadAtomicData(stream, camAspect);
		JFileIOHelper::LoadAtomicData(stream, camOrthoViewWidth);
		JFileIOHelper::LoadAtomicData(stream, camOrthoViewHeight);
		JFileIOHelper::LoadAtomicData(stream, isOrtho);
		JFileIOHelper::LoadAtomicData(stream, allowDisplayDepthMap);
		JFileIOHelper::LoadAtomicData(stream, allowDisplayDebug);
		JFileIOHelper::LoadAtomicData(stream, allowFrustumCulling);
		JFileIOHelper::LoadAtomicData(stream, allowHzbOcclusionCulling);
		JFileIOHelper::LoadAtomicData(stream, allowDisplayOccCullingDepthMap);

		auto idenUser = cPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JCamera::InitData>(guid, flag, owner), &cPrivate);
		JUserPtr<JCamera> camUser;
		camUser.ConnnectChild(idenUser);

		JCamera::JCameraImpl* impl = camUser->impl.get();
		impl->camNear = camNear;
		impl->camFar = camFar;
		impl->camFov = camFov;
		impl->camAspect = camAspect;
		impl->camOrthoViewWidth = camOrthoViewWidth;
		impl->camOrthoViewHeight = camOrthoViewHeight;
		camUser->GetTransform()->SetPosition(pos);

		if (isOrtho)
			impl->CalOrthoLens();
		else
			impl->CalPerspectiveLens();

		impl->SetAllowDisplayDepthMap(allowDisplayDepthMap);
		impl->SetAllowDisplayDebug(allowDisplayDebug);
		impl->SetAllowFrustumCulling(allowFrustumCulling);
		impl->SetAllowHzbOcclusionCulling(allowHzbOcclusionCulling);
		impl->SetAllowDisplayOccCullingDepthMap(allowDisplayOccCullingDepthMap);
		impl->SetCameraState(camState);
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
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, cam.Get());
		JFileIOHelper::StoreEnumData(stream, L"CamState:", impl->camState);
		JFileIOHelper::StoreXMFloat3(stream, L"Pos:", cam->GetTransform()->GetPosition());
		JFileIOHelper::StoreAtomicData(stream, L"CamNear:", impl->camNear);
		JFileIOHelper::StoreAtomicData(stream, L"CamFar:", impl->camFar);
		JFileIOHelper::StoreAtomicData(stream, L"CamFov:", impl->camFov);
		JFileIOHelper::StoreAtomicData(stream, L"CamAspect:", impl->camAspect);
		JFileIOHelper::StoreAtomicData(stream, L"camOrthoViewWidth:", impl->camOrthoViewWidth);
		JFileIOHelper::StoreAtomicData(stream, L"CamOrthoViewHeight:", impl->camOrthoViewHeight);
		JFileIOHelper::StoreAtomicData(stream, L"IsOrtho:", impl->isOrtho);
		JFileIOHelper::StoreAtomicData(stream, L"AllowDepthMap:", impl->allowDisplayDepthMap);
		JFileIOHelper::StoreAtomicData(stream, L"AllowDebug:", impl->allowDisplayDebug);
		JFileIOHelper::StoreAtomicData(stream, L"AllowFrustumCulling:", impl->allowFrustumCulling);
		JFileIOHelper::StoreAtomicData(stream, L"AllowHzbOcclusionCulling:", impl->allowHzbOcclusionCulling);
		JFileIOHelper::StoreAtomicData(stream, L"AllowDislplayCullingDepthMap:", impl->allowDisplayOccCullingDepthMap);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JCamera* cam, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			cam->impl->SetFrameDirty();

		cam->impl->SetLastFrameUpdatedTrigger(false);
		cam->impl->SetLastFrameHotUpdatedTrigger(false);
		return cam->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JCamera* cam, Graphic::JCameraConstants& constants)noexcept
	{
		cam->impl->UpdateFrame(constants);
	}
	void FrameUpdateInterface::UpdateFrame(JCamera* cam, Graphic::JHzbOccRequestorConstants& constants, const uint queryCount, const uint queryOffset)noexcept
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
		return cam->impl->CamFrame::GetUploadIndex();
	}
	int FrameUpdateInterface::GetHzbOccReqFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->HzbOccReqFrame::GetUploadIndex();
	}
	bool FrameUpdateInterface::IsHotUpdated(JCamera* cam)noexcept
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
	bool FrameUpdateInterface::HasOccPassRecopyRequest(JCamera* cam)noexcept
	{
		return cam->impl->HzbOccReqFrame::HasMovedDirty();
	}

	int FrameIndexInterface::GetCamFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->CamFrame::GetUploadIndex();
	}
	int FrameIndexInterface::GetHzbOccReqFrameIndex(JCamera* cam)noexcept
	{
		return cam->impl->HzbOccReqFrame::GetUploadIndex();
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
