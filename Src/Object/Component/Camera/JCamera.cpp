#include"JCamera.h"  
#include"../JComponentFactory.h"
#include"../Transform/JTransform.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"
#include"../../../Core/Guid/GuidCreator.h"  
#include"../../../Graphic/FrameResource/JCameraConstants.h" 
#include"../../../Application/JApplicationVariable.h"
#include"../../../Application/JApplicationState.h" 
#include"../../../Window/JWindows.h"
#include"../../../Utility/JMathHelper.h"
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	bool JCamera::IsMainCamera()const noexcept
	{
		return isMainCamera;
	}
	JTransform* JCamera::GetTransform()noexcept
	{
		return ownerTransform;
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
		return mCamFrustum;
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
	void JCamera::SetOrthoCamera()noexcept
	{
		isOrtho = true;
		CalOrthoLens();
	}
	void JCamera::SetPerspectiveCamera()noexcept
	{
		isOrtho = false;
		CalPerspectiveLens();
	}
	void JCamera::SetMainCamera()noexcept
	{
		if (camState == J_CAMERA_STATE::IDEL)
			SetCameraState(J_CAMERA_STATE::RENDER);

		JCamera* preMainCam = GetOwner()->GetOwnerScene()->CompInterface()->SetMainCamera(this);
		if (preMainCam != nullptr)
		{
			preMainCam->isMainCamera = false;
			if (preMainCam->GetCameraState() == J_CAMERA_STATE::RENDER)
				preMainCam->SetCameraState(J_CAMERA_STATE::IDEL);
		}
		isMainCamera = true;
	}
	J_COMPONENT_TYPE JCamera::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	J_COMPONENT_TYPE JCamera::GetStaticComponentType()noexcept
	{
		return J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
	}
	bool JCamera::IsAvailableOverlap()const noexcept
	{
		return false;
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
	void JCamera::DoActivate()noexcept
	{
		JComponent::DoActivate();
		if (camState == J_CAMERA_STATE::RENDER)
		{
			if (RegisterComponent())
				CreateRenderTarget();
			SetFrameDirty();
		}
	}
	void JCamera::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		if (camState == J_CAMERA_STATE::RENDER)
		{
			if (DeRegisterComponent())
				DestroyRenderTarget();
			OffFrameDirty();
		}
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
	bool JCamera::UpdateFrame(Graphic::JCameraConstants& constant)
	{
		if (IsFrameDirted())
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
			constant.eyePosW = ownerTransform->GetPosition();
			constant.nearZ = cameraNear;
			constant.farZ = cameraFar;

			MinusFrameDirty();
			return true;
		}
		else
			return false;
	}
	void JCamera::SetFrameDirty()noexcept
	{
		JFrameInterface::SetFrameDirty();
	}
	void JCamera::SetCameraState(const J_CAMERA_STATE state)noexcept
	{
		camState = state;
		if (camState == J_CAMERA_STATE::RENDER)
		{
			if (RegisterComponent())
				CreateRenderTarget();
		}
		else
		{
			if (DeRegisterComponent())
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

		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, camera);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		const XMFLOAT3 pos = camera->GetTransform()->GetPosition();

		stream << (int)camera->camState << '\n';
		stream << pos.x << " " << pos.y << " " << pos.z << '\n';
		stream << camera->viewWidth << '\n' << camera->viewHeight << '\n' << camera->cameraNear << '\n' << camera->cameraFar << '\n' << camera->cameraFov << '\n';
		stream << camera->isMainCamera << '\n';

		return  Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JCamera* JCamera::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open())
			return nullptr;

		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);

		JCamera* newCamera;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newCamera = new JCamera(metadata.guid, metadata.flag, owner);
		else
			newCamera = new JCamera(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);

		int camState;
		XMFLOAT3 pos;
		int viewWidth;
		int viewHeight;
		float cameraNear;
		float cameraFar;
		float camFov;
		bool isMainCam;

		stream >> camState;
		stream >> pos.x >> pos.y >> pos.z;
		stream >> viewWidth >> viewHeight >> cameraNear >> cameraFar >> camFov;
		stream >> isMainCam;

		newCamera->SetCameraState((J_CAMERA_STATE)camState);
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

		if (isMainCam)
			newCamera->SetMainCamera();

		return newCamera;
	}
	void JCamera::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			return new JCamera(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			return new JCamera(guid, objFlag, owner);
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			JCamera* oriCam = static_cast<JCamera*>(ori);
			JCamera* newCam = new JCamera(Core::MakeGuid(), oriCam->GetFlag(), owner);

			newCam->cameraNear = oriCam->cameraNear;
			newCam->cameraFar = oriCam->cameraFar;
			newCam->cameraAspect = oriCam->cameraAspect;
			newCam->cameraFov = oriCam->cameraFov;
			newCam->cameraNearViewHeight = oriCam->cameraNearViewHeight;
			newCam->cameraFarViewHeight = oriCam->cameraFarViewHeight;
			newCam->isOrtho = oriCam->isOrtho;

			if (!newCam->isOrtho)
				newCam->CalPerspectiveLens();
			else
				newCam->CalOrthoLens();
			return newCam;
		};
		JCFI<JCamera>::Regist(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JCamera::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JCamera::StaticTypeInfo };

		static auto setFrameLam = [](JComponent& component)
		{
			static_cast<JCamera*>(&component)->SetFrameDirty();
		};
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{getTypeNameCallable, getTypeInfoCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JCamera::JCamera(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JCameraInterface(TypeName(), guid, objFlag, owner)
	{
		ownerTransform = owner->GetComponent<JTransform>();
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
	JCamera::~JCamera() {}
}
