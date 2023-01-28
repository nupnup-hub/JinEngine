#include"JCamera.h"  
#include"../JComponentFactory.h"
#include"../Transform/JTransform.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Guid/GuidCreator.h"  
#include"../../../Core/File/JFileConstant.h"
#include"../../../Graphic/FrameResource/JCameraConstants.h" 
#include"../../../Application/JApplicationVariable.h"
#include"../../../Application/JApplicationState.h" 
#include"../../../Window/JWindows.h"
#include"../../../Utility/JMathHelper.h"
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	static auto isAvailableoverlapLam = []() {return false; };

	J_COMPONENT_TYPE JCamera::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JTransform* JCamera::GetTransform()noexcept
	{
		return GetOwner()->GetTransform();
	}
	XMMATRIX JCamera::GetView()const noexcept
	{
		return XMLoadFloat4x4(&mView);
	}
	XMMATRIX JCamera::GetProj()const noexcept
	{
		return XMLoadFloat4x4(&mProj);
	}
	XMFLOAT4X4 JCamera::GetView4x4f()const noexcept
	{
		return mView;
	}
	XMFLOAT4X4 JCamera::GetProj4x4f()const noexcept
	{
		return mProj;
	}
	DirectX::BoundingFrustum JCamera::GetBoundingFrustum()const noexcept
	{ 
		DirectX::BoundingFrustum worldCamFrustum;
		mCamFrustum.Transform(worldCamFrustum, GetOwner()->GetTransform()->GetWorldMatrix());
		return worldCamFrustum;
	}
	float JCamera::GetNear()const noexcept
	{
		return cameraNear;
	}
	float JCamera::GetFar()const noexcept
	{
		return cameraFar;
	}
	float JCamera::GetAspect()const noexcept
	{
		return cameraAspect;
	}
	float JCamera::GetFovY()const noexcept
	{
		return cameraFov;
	}
	float JCamera::GetFovYDegree()const noexcept
	{
		return JMathHelper::RadToDeg * cameraFov;
	}
	float JCamera::GetFovX()const noexcept
	{
		float halfWidth = 0.5f * GetNearViewWidth();
		return 2.0f * atan(halfWidth / cameraNear);
	}
	int JCamera::GetViewWidth()const noexcept
	{
		return viewWidth;
	}
	int JCamera::GetViewHeight()const noexcept
	{
		return viewHeight;
	}
	float JCamera::GetNearViewWidth()const noexcept
	{
		return cameraAspect * cameraNearViewHeight;
	}
	float JCamera::GetNearViewHeight()const noexcept
	{
		return cameraNearViewHeight;
	}
	float JCamera::GetFarViewWidth()const noexcept
	{
		return cameraAspect * cameraFarViewHeight;
	}
	float JCamera::GetFarViewHeight()const noexcept
	{
		return cameraFarViewHeight;
	}
	J_CAMERA_STATE JCamera::GetCameraState()const noexcept
	{
		return camState;
	}
	void JCamera::SetNear(float value)noexcept
	{
		cameraNear = value;
		if (isOrtho)
			CalOrthoLens();
		else
			CalPerspectiveLens();
	}
	void JCamera::SetFar(float value) noexcept
	{
		cameraFar = value;
		if (isOrtho)
			CalOrthoLens();
		else
			CalPerspectiveLens();
	}
	void JCamera::SetFov(float value) noexcept
	{
		cameraFov = value;
		if (!isOrtho)
			CalPerspectiveLens();
	}
	void JCamera::SetFovDegree(float value) noexcept
	{
		cameraFov = value * JMathHelper::DegToRad;
		if (!isOrtho)
			CalPerspectiveLens();
	}
	void JCamera::SetViewSize(int width, int height) noexcept
	{
		viewWidth = width;
		viewHeight = height;
		SetAspect(static_cast<float>(viewWidth) / viewHeight);
		if (isOrtho)
			CalOrthoLens();
		else
			CalPerspectiveLens();
	}
	void JCamera::SetOrthoCamera(bool value)noexcept
	{
		isOrtho = value;
		if (isOrtho)
			CalOrthoLens();
		else
			CalPerspectiveLens();
	}
	void JCamera::SetMainCamera(bool value)noexcept
	{
		if (isMainCamera == value)
			return;

		if (value)
		{
			JCamera* mainCam = GetOwner()->GetOwnerScene()->GetMainCamera();
			if (mainCam != nullptr)
				mainCam->SetCameraState(J_CAMERA_STATE::IDEL);

			SetCameraState(J_CAMERA_STATE::RENDER);
			if(IsActivated())
				GetOwner()->GetOwnerScene()->CompInterface()->SetMainCamera(this);
		}
		else
		{
			if (IsActivated())
				GetOwner()->GetOwnerScene()->CompInterface()->SetMainCamera(nullptr);
		}
		isMainCamera = value;
	}
	bool JCamera::IsOrthoCamera()const noexcept
	{
		return isOrtho;
	}
	bool JCamera::IsMainCamera()const noexcept
	{
		return isMainCamera;
	}
	bool JCamera::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JCamera::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && camState == J_CAMERA_STATE::RENDER)
			return true;
		else
			return false;
	}
	JCameraStateInterface* JCamera::StateInterface()
	{
		return this;
	}
	void JCamera::DoCopy(JObject* ori)
	{
		JCamera* oriCam = static_cast<JCamera*>(ori);
		cameraNear = oriCam->cameraNear;
		cameraFar = oriCam->cameraFar;
		cameraAspect = oriCam->cameraAspect;
		cameraFov = oriCam->cameraFov;
		cameraNearViewHeight = oriCam->cameraNearViewHeight;
		cameraFarViewHeight = oriCam->cameraFarViewHeight;
		isOrtho = oriCam->isOrtho;
		if (!isOrtho)
			CalPerspectiveLens();
		else
			CalOrthoLens();
		SetFrameDirty();
	}
	void JCamera::DoActivate()noexcept
	{
		JComponent::DoActivate();
		if (camState == J_CAMERA_STATE::RENDER)
		{
			if (IsMainCamera())
				GetOwner()->GetOwnerScene()->CompInterface()->SetMainCamera(this);
			if (RegisterComponent())
				CreateRenderTarget();
		}
		SetFrameDirty();
	}
	void JCamera::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		if (camState == J_CAMERA_STATE::RENDER)
		{
			if (IsMainCamera())
				GetOwner()->GetOwnerScene()->CompInterface()->SetMainCamera(nullptr);
			if (DeRegisterComponent())
				DestroyRenderTarget();
		}
		OffFrameDirty();
	}
	void JCamera::SetAspect(float value) noexcept
	{
		cameraAspect = value;
		CalPerspectiveLens();
		if (!isOrtho)
			CalPerspectiveLens();
	}
	void JCamera::CalPerspectiveLens() noexcept
	{
		cameraNearViewHeight = 2.0f * cameraNear * tanf(0.5f * cameraFov);
		cameraFarViewHeight = 2.0f * cameraFar * tanf(0.5f * cameraFov);
		  
		const XMMATRIX P = XMMatrixPerspectiveFovLH(cameraFov, cameraAspect, cameraNear, cameraFar);
		XMStoreFloat4x4(&mProj, P);
		BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
		SetFrameDirty();
	}
	void JCamera::CalOrthoLens() noexcept
	{
		//XMMatrixOrthographicOffCenterLH
		const XMMATRIX P = XMMatrixOrthographicLH((float)viewWidth, (float)viewHeight, cameraNear, cameraFar);
		XMStoreFloat4x4(&mProj, P);
		BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
		SetFrameDirty();
	}
	void JCamera::CreateRenderTarget()noexcept
	{ 
		//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
		CreateRenderTargetTexture();
		AddDrawRequest(GetOwner()->GetOwnerScene(), this);
	}
	void JCamera::DestroyRenderTarget()noexcept
	{ 
		//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
		PopDrawRequest(GetOwner()->GetOwnerScene(), this);
		DestroyTxtHandle();
	}
	void JCamera::UpdateViewMatrix() noexcept
	{
		//if (gameObjectDirty->GetTransformDirty() > 0)
		{
			JTransform* ownerTransform = GetTransform();
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
	void JCamera::UpdateFrame(Graphic::JCameraConstants& constant)
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

		constant.renderTargetSize = XMFLOAT2((float)viewWidth, (float)viewHeight);
		constant.invRenderTargetSize = XMFLOAT2(1.0f / viewWidth, 1.0f / viewHeight);
		constant.eyePosW = GetTransform()->GetPosition();
		constant.nearZ = cameraNear;
		constant.farZ = cameraFar;
	}
	void JCamera::SetCameraState(const J_CAMERA_STATE state)noexcept
	{
		if (camState == state)
			return;

		camState = state;
		if (camState == J_CAMERA_STATE::RENDER)
		{
			if (IsActivated() && RegisterComponent())
				CreateRenderTarget();
		}
		else
		{
			if (IsActivated() && DeRegisterComponent())
				DestroyRenderTarget();
		}
		SetFrameDirty();
	}
	Core::J_FILE_IO_RESULT JCamera::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JCamera::StoreObject(std::wofstream& stream, JCamera* camera)
	{
		if (camera == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)camera->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::StoreObjectIden(stream, camera);
		JFileIOHelper::StoreEnumData(stream, L"CamState:", camera->camState);
		JFileIOHelper::StoreXMFloat3(stream, L"Pos:", camera->GetTransform()->GetPosition());
		JFileIOHelper::StoreAtomicData(stream, L"CamViewWidth:", camera->viewWidth);
		JFileIOHelper::StoreAtomicData(stream, L"CamViewHeight:", camera->viewHeight);
		JFileIOHelper::StoreAtomicData(stream, L"CamNear:", camera->cameraNear);
		JFileIOHelper::StoreAtomicData(stream, L"CamFar:", camera->cameraFar);
		JFileIOHelper::StoreAtomicData(stream, L"CamFov:", camera->cameraFov);
		JFileIOHelper::StoreAtomicData(stream, L"IsMainCam:", camera->isMainCamera); 

		return  Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JCamera* JCamera::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open() || stream.eof())
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		J_CAMERA_STATE camState;
		XMFLOAT3 pos;
		int viewWidth;
		int viewHeight;
		float cameraNear;
		float cameraFar;
		float camFov;
		bool isMainCam;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadEnumData(stream, camState);
		JFileIOHelper::LoadXMFloat3(stream, pos);
		JFileIOHelper::LoadAtomicData(stream, viewWidth);
		JFileIOHelper::LoadAtomicData(stream, viewHeight);
		JFileIOHelper::LoadAtomicData(stream, cameraNear);
		JFileIOHelper::LoadAtomicData(stream, cameraFar);
		JFileIOHelper::LoadAtomicData(stream, camFov);
		JFileIOHelper::LoadAtomicData(stream, isMainCam);

		Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JCamera>(guid, flag, owner);
		JCamera* newCamera = ownerPtr.Get();
		if (!AddInstance(std::move(ownerPtr)))
			return nullptr;

		newCamera->cameraFov = camFov;
		newCamera->cameraNear = cameraNear;
		newCamera->cameraFar = cameraFar;
		newCamera->viewWidth = viewWidth;
		newCamera->viewHeight = viewHeight;
		newCamera->GetTransform()->SetPosition(pos);

		if (newCamera->isOrtho)
			newCamera->CalOrthoLens();
		else
			newCamera->CalPerspectiveLens();

		newCamera->SetCameraState(camState);
		if (isMainCam)
			newCamera->SetMainCamera(true);
		return newCamera;
	}
	void JCamera::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JCamera>(Core::MakeGuid(), owner->GetFlag(), owner);
			JCamera* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JCamera>(guid, Core::AddSQValueEnum(owner->GetFlag(), objFlag), owner);
			JCamera* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JCamera>(Core::MakeGuid(), ori->GetFlag(), owner);
			JCamera* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				if (newComp->Copy(ori))
					return newComp;
				else
				{
					BegineForcedDestroy(newComp);
					return nullptr;
				}
			}
			else
				return nullptr;
		};
		JCFI<JCamera>::Register(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JCamera::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JCamera::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

		static auto setFrameLam = [](JComponent& component) {static_cast<JCamera*>(&component)->SetFrameDirty(); };
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{ getTypeNameCallable, getTypeInfoCallable, isAvailableOverlapCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ &setFrameDirtyCallable, nullptr };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JCamera::JCamera(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JCameraInterface(TypeName(), guid, objFlag, owner)
	{ 
		RegisterFrameDirtyListener(*GetOwner()->GetTransform());

		cameraFov = 0.25f * JMathHelper::Pi;
		cameraNear = 1;
		cameraFar = 1000;
		mView = JMathHelper::Identity4x4();
		mProj = JMathHelper::Identity4x4();

		SetViewSize(JWindow::Instance().GetClientWidth(), JWindow::Instance().GetClientHeight());
		//ownerTransform->SetPosition(XMFLOAT3(0.0f, 2.0f, -15.0f));
		CalPerspectiveLens();

		BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
		//CalOrthoLens(1, 1000.0f);
	}
	JCamera::~JCamera() 
	{
		if (GetOwner()->GetTransform() != nullptr)
			DeRegisterFrameDirtyListener(*GetOwner()->GetTransform());
	}
}
