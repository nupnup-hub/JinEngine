#include"JCamera.h"
#include"JCameraPrivate.h"
#include"../JComponentHint.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../../JFrameUpdate.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h" 
#include"../../../Core/Guid/GuidCreator.h"  
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/FrameResource/JCameraConstants.h"    
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
 
	class JCamera::JCameraImpl : public Core::JIdentifierImplBase,
		public JFrameUpdate<JFrameUpdate1<JFrameUpdateBase<Graphic::JCameraConstants&>>, JFrameDirtyListener, FrameUpdate::nonBuff>,
		public Graphic::JGraphicResourceInterface
	{ 
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JCameraImpl)
	public:
		JCamera* thisCam = nullptr;
	public:
		//JTransform* ownerTransform;
		J_CAMERA_STATE camState = J_CAMERA_STATE::IDEL;

		// Cache frustum properties.
		REGISTER_PROPERTY_EX(cameraNear, GetNear, SetNear, GUI_SLIDER(0, 1000, true))
		float cameraNear = 0.0f;
		REGISTER_PROPERTY_EX(cameraFar, GetFar, SetFar, GUI_SLIDER(0, 1000, true))
		float cameraFar = 0.0f;

		float cameraAspect = 0.0f;
		REGISTER_PROPERTY_EX(cameraFov, GetFovYDegree, SetFovDegree, GUI_SLIDER(0, 360, true))
		float cameraFov = 0.0f;
		float cameraNearViewHeight = 0.0f;
		float cameraFarViewHeight = 0.0f;

		int viewWidth;
		int viewHeight;

		REGISTER_PROPERTY_EX(isOrtho, IsOrthoCamera, SetOrthoCamera, GUI_CHECKBOX())
		bool isOrtho = false;
		bool isMainCamera = false;

		// Cache View/Proj matrices.
		DirectX::XMFLOAT4X4 mView;
		DirectX::XMFLOAT4X4 mProj;
		DirectX::BoundingFrustum mCamFrustum;
	public:
		//Caution
		//Impl생성자에서 interface class 참조시 interface class가 함수내에서 impl을 참조할 경우 error
		//impl이 아직 생성되지 않았으므로
		JCameraImpl(const InitData& initData, JCamera* thisCam)
			:thisCam(thisCam)
		{
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisCam->GetOwner()->GetTransform(), this);
		}
		~JCameraImpl()
		{
			if (thisCam->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisCam->GetOwner()->GetTransform(), this);
		}
	public:
		void Initialize()
		{
			//CalOrthoLens(1, 1000.0f);
			cameraFov = 0.25f * JMathHelper::Pi;
			cameraNear = 1;
			cameraFar = 1000;
			mView = JMathHelper::Identity4x4();
			mProj = JMathHelper::Identity4x4();

			SetViewSize(JWindow::GetClientWidth(), JWindow::GetClientHeight());
			//ownerTransform->SetPosition(XMFLOAT3(0.0f, 2.0f, -15.0f));
			CalPerspectiveLens();

			BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
		}
	public:  
		float GetNear()const noexcept
		{
			return cameraNear;
		}
		float GetFar()const noexcept
		{
			return cameraFar;
		}
		float GetFovYDegree()const noexcept
		{
			return JMathHelper::RadToDeg * cameraFov;
		} 
	public:
		void SetNear(float value)noexcept
		{
			cameraNear = value;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFar(float value) noexcept
		{
			cameraFar = value;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetFov(float value) noexcept
		{
			cameraFov = value;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetFovDegree(float value) noexcept
		{
			cameraFov = value * JMathHelper::DegToRad;
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetViewSize(int width, int height) noexcept
		{
			viewWidth = width;
			viewHeight = height;
			SetAspect(static_cast<float>(viewWidth) / viewHeight);
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetOrthoCamera(bool value)noexcept
		{
			isOrtho = value;
			if (isOrtho)
				CalOrthoLens();
			else
				CalPerspectiveLens();
		}
		void SetMainCamera(bool value)noexcept
		{
			if (isMainCamera == value)
				return;

			if (value)
			{
				JCamera* mainCam = thisCam->GetOwner()->GetOwnerScene()->GetMainCamera();
				if (mainCam != nullptr)
					mainCam->impl->SetCameraState(J_CAMERA_STATE::IDEL);

				SetCameraState(J_CAMERA_STATE::RENDER);
				if (thisCam->IsActivated())
					JScenePrivate::CompSettingInterface::SetMainCamera(thisCam->GetOwner()->GetOwnerScene(), thisCam);
			}
			else
			{
				if (thisCam->IsActivated())
					JScenePrivate::CompSettingInterface::SetMainCamera(thisCam->GetOwner()->GetOwnerScene(), nullptr);
			}
			isMainCamera = value;
		}
		void SetAspect(float value) noexcept
		{
			cameraAspect = value;
			CalPerspectiveLens();
			if (!isOrtho)
				CalPerspectiveLens();
		}
		void SetCameraState(const J_CAMERA_STATE state)noexcept
		{
			if (camState == state)
				return;

			camState = state;
			if (camState == J_CAMERA_STATE::RENDER)
			{
				if (thisCam->IsActivated() && thisCam->RegisterComponent())
					CreateRenderTarget();
			}
			else
			{
				if (thisCam->IsActivated() && thisCam->DeRegisterComponent())
					DestroyRenderTarget();
			}
			SetFrameDirty();
		}
	public:
		bool IsOrthoCamera()const noexcept
		{
			return isOrtho;
		}
	public:
		void CalPerspectiveLens() noexcept
		{
			cameraNearViewHeight = 2.0f * cameraNear * tanf(0.5f * cameraFov);
			cameraFarViewHeight = 2.0f * cameraFar * tanf(0.5f * cameraFov);

			const XMMATRIX P = XMMatrixPerspectiveFovLH(cameraFov, cameraAspect, cameraNear, cameraFar);
			XMStoreFloat4x4(&mProj, P);
			BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
			SetFrameDirty();
		}
		void CalOrthoLens() noexcept
		{
			//XMMatrixOrthographicOffCenterLH
			const XMMATRIX P = XMMatrixOrthographicLH((float)viewWidth, (float)viewHeight, cameraNear, cameraFar);
			XMStoreFloat4x4(&mProj, P);
			BoundingFrustum::CreateFromMatrix(mCamFrustum, XMLoadFloat4x4(&mProj));
			SetFrameDirty();
		}
	public:
		void CreateRenderTarget()noexcept
		{
			//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			CreateRenderTargetTexture();
			AddDrawRequest(thisCam->GetOwner()->GetOwnerScene(), thisCam);
		}
		void DestroyRenderTarget()noexcept
		{
			//if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			PopDrawRequest(thisCam->GetOwner()->GetOwnerScene(), thisCam);
			DestroyTexture();
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

			constant.renderTargetSize = XMFLOAT2((float)viewWidth, (float)viewHeight);
			constant.invRenderTargetSize = XMFLOAT2(1.0f / viewWidth, 1.0f / viewHeight);
			constant.eyePosW = thisCam->GetTransform()->GetPosition();
			constant.nearZ = cameraNear;
			constant.farZ = cameraFar;
		}
		void UpdateViewMatrix() noexcept
		{
			//if (gameObjectDirty->GetTransformDirty() > 0)
			{
				JTransform* ownerTransform = thisCam->GetTransform();
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
			to->impl->cameraNear = from->impl->cameraNear;
			to->impl->cameraFar = from->impl->cameraFar;
			to->impl->cameraAspect = from->impl->cameraAspect;
			to->impl->cameraFov = from->impl->cameraFov;
			to->impl->cameraNearViewHeight = from->impl->cameraNearViewHeight;
			to->impl->cameraFarViewHeight = from->impl->cameraFarViewHeight;
			to->impl->isOrtho = from->impl->isOrtho;
			if (!to->impl->isOrtho)
				to->impl->CalPerspectiveLens();
			else
				to->impl->CalOrthoLens(); 
			return true;
		}
	public:
		static void RegisterCallOnce()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JCamera::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](JGameObject* parent, InitUnq&& parentClassInitData) -> InitUnq
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
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable, nullptr };

			JComponent::RegisterCTypeInfo(JCamera::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JCamera::StaticTypeInfo(), cPrivate);
		}
	};

	JCamera::InitData::InitData(JGameObject* owner)
		:JComponent::InitData(JCamera::StaticTypeInfo(), owner)
	{}
	JCamera::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		: JComponent::InitData(JCamera::StaticTypeInfo(), GetDefaultName(JCamera::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JCamera::GetPrivateInterface()const noexcept
	{
		return cPrivate;
	}
	JFrameUpdateUserAccess JCamera::GetFrameUserInterface() noexcept
	{
		return JFrameUpdateUserAccess(Core::GetUserPtr(this), impl.get());
	}
	const Graphic::JGraphicResourceUserInterface JCamera::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface{ impl.get() };
	}
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
		DirectX::BoundingFrustum worldCamFrustum;
		impl->mCamFrustum.Transform(worldCamFrustum, GetOwner()->GetTransform()->GetWorldMatrix());
		return worldCamFrustum;
	}
	float JCamera::GetNear()const noexcept
	{
		return impl->GetNear();
	}
	float JCamera::GetFar()const noexcept
	{
		return impl->GetFar();
	}
	float JCamera::GetAspect()const noexcept
	{
		return impl->cameraAspect;
	}
	float JCamera::GetFovY()const noexcept
	{
		return impl->cameraFov;
	}
	float JCamera::GetFovYDegree()const noexcept
	{
		return impl->GetFovYDegree();
	}
	float JCamera::GetFovX()const noexcept
	{
		float halfWidth = 0.5f * GetNearViewWidth();
		return 2.0f * atan(halfWidth / impl->GetNear());
	}
	int JCamera::GetViewWidth()const noexcept
	{
		return impl->viewWidth;
	}
	int JCamera::GetViewHeight()const noexcept
	{
		return impl->viewHeight;
	}
	float JCamera::GetNearViewWidth()const noexcept
	{
		return impl->cameraAspect * impl->cameraNearViewHeight;
	}
	float JCamera::GetNearViewHeight()const noexcept
	{
		return impl->cameraNearViewHeight;
	}
	float JCamera::GetFarViewWidth()const noexcept
	{
		return impl->cameraAspect * impl->cameraFarViewHeight;
	}
	float JCamera::GetFarViewHeight()const noexcept
	{
		return impl->cameraFarViewHeight;
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
		impl->SetFov(value); 
	}
	void JCamera::SetFovDegree(float value) noexcept
	{
		impl->SetFovDegree(value);
	}
	void JCamera::SetViewSize(int width, int height) noexcept
	{
		impl->SetViewSize(width, height);
	}
	void JCamera::SetOrthoCamera(bool value)noexcept
	{
		impl->SetOrthoCamera(value);
	}
	void JCamera::SetMainCamera(bool value)noexcept
	{
		impl->SetMainCamera(value);
	}
	bool JCamera::IsOrthoCamera()const noexcept
	{
		return impl->isOrtho;
	}
	bool JCamera::IsMainCamera()const noexcept
	{
		return impl->isMainCamera;
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
	 
	void JCamera::DoActivate()noexcept
	{
		JComponent::DoActivate();
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			if (IsMainCamera())
				JScenePrivate::CompSettingInterface::SetMainCamera(GetOwner()->GetOwnerScene(), this);
			if (RegisterComponent())
				impl->CreateRenderTarget();
		}
		impl->SetFrameDirty();
	}
	void JCamera::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		if (impl->camState == J_CAMERA_STATE::RENDER)
		{
			if (IsMainCamera())
				JScenePrivate::CompSettingInterface::SetMainCamera(GetOwner()->GetOwnerScene(), nullptr);
			if (DeRegisterComponent())
				impl->DestroyRenderTarget();
		}
		impl->OffFrameDirty();
	}
	JCamera::JCamera(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JCameraImpl>(initData, this))
	{
		impl->Initialize();
	}
	JCamera::~JCamera()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JCameraPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JCameraPrivate::AssetDataIOInterface;
	using CameraStateInterface = JCameraPrivate::CameraStateInterface;
	using FrameUpdateInterface = JCameraPrivate::FrameUpdateInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JCamera>(*static_cast<JCamera::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JCamera::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JCamera::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JCamera::JCameraImpl::DoCopy(static_cast<JCamera*>(from), static_cast<JCamera*>(to));
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JCamera::LoadData::StaticTypeInfo()))
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

		auto loadData = static_cast<JCamera::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JGameObject* owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadEnumData(stream, camState);
		JFileIOHelper::LoadXMFloat3(stream, pos);
		JFileIOHelper::LoadAtomicData(stream, viewWidth);
		JFileIOHelper::LoadAtomicData(stream, viewHeight);
		JFileIOHelper::LoadAtomicData(stream, cameraNear);
		JFileIOHelper::LoadAtomicData(stream, cameraFar);
		JFileIOHelper::LoadAtomicData(stream, camFov);
		JFileIOHelper::LoadAtomicData(stream, isMainCam);

		auto rawPtr = cPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JCamera::InitData>(guid, flag, owner), &cPrivate);
		JCamera* newCamera = static_cast<JCamera*>(rawPtr);
		JCamera::JCameraImpl* impl = newCamera->impl.get();
		impl->cameraFov = camFov;
		impl->cameraNear = cameraNear;
		impl->cameraFar = cameraFar;
		impl->viewWidth = viewWidth;
		impl->viewHeight = viewHeight;
		newCamera->GetTransform()->SetPosition(pos);

		if (impl->isOrtho)
			impl->CalOrthoLens();
		else
			impl->CalPerspectiveLens();

		impl->SetCameraState(camState);
		if (isMainCam)
			impl->SetMainCamera(true);
		return newCamera;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JCamera::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JCamera::StoreData*>(data);
		if (!storeData->HasCorrectType(JCamera::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JCamera* camera = static_cast<JCamera*>(storeData->obj);
		JCamera::JCameraImpl* impl = camera->impl.get();
		std::wofstream& stream = storeData->stream;
		 
		JFileIOHelper::StoreObjectIden(stream, camera);
		JFileIOHelper::StoreEnumData(stream, L"CamState:", impl->camState);
		JFileIOHelper::StoreXMFloat3(stream, L"Pos:", camera->GetTransform()->GetPosition());
		JFileIOHelper::StoreAtomicData(stream, L"CamViewWidth:", impl->viewWidth);
		JFileIOHelper::StoreAtomicData(stream, L"CamViewHeight:", impl->viewHeight);
		JFileIOHelper::StoreAtomicData(stream, L"CamNear:", impl->cameraNear);
		JFileIOHelper::StoreAtomicData(stream, L"CamFar:", impl->cameraFar);
		JFileIOHelper::StoreAtomicData(stream, L"CamFov:", impl->cameraFov);
		JFileIOHelper::StoreAtomicData(stream, L"IsMainCam:", impl->isMainCamera);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void CameraStateInterface::SetCameraState(JCamera* cam, const J_CAMERA_STATE state)noexcept
	{
		cam->impl->SetCameraState(state);
	}

	bool FrameUpdateInterface::UpdateStart(JCamera* cam, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			cam->impl->SetFrameDirty();

		return cam->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JCamera* cam, Graphic::JCameraConstants& constants)noexcept
	{
		cam->impl->UpdateFrame(constants);
	}
	void FrameUpdateInterface::UpdateEnd(JCamera* cam)noexcept
	{
		cam->impl->UpdateEnd();
	}
	bool FrameUpdateInterface::IsHotUpdated(JCamera* cam)noexcept
	{
		return cam->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JCameraPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JCameraPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}
