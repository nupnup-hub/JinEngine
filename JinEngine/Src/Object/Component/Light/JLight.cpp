#include"JLight.h"   
#include"JLightPrivate.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../JComponentHint.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Graphic/JGraphic.h"  
#include"../../../Graphic/Upload/Frameresource/JLightConstants.h" 
#include"../../../Graphic/Upload/Frameresource/JShadowMapConstants.h" 
#include"../../../Graphic/Upload/Frameresource/JOcclusionConstants.h"
#include"../../../Graphic/Upload/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/Culling/JCullingInterface.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Graphic/JGraphicDrawListInterface.h" 
#include<Windows.h>
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	namespace
	{
		using LitFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder4 <
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT, Graphic::JLightConstants&>, 
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT, Graphic::JShadowMapLightConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP, Graphic::JShadowMapConstants&>,
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_PASS, Graphic::JHzbOccPassConstants&, const uint, const uint>>,
			Graphic::JFrameDirty>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JLightPrivate lPrivate;
	}

	class JLight::JLightImpl : public Core::JTypeImplBase,
		public LitFrameUpdate,
		public Graphic::JGraphicMultiResourceInterface,
		public Graphic::JGraphicDrawListCompInterface,
		public Graphic::JCullingInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JLightImpl)
	public:
		using LitFrame = JFrameInterface1;
		using ShadowLitFrame = JFrameInterface2;
		using ShadowMapFrame = JFrameInterface3;
		using HzbOccPassFrame = JFrameInterface4;
	public:
		JWeakPtr<JLight> thisPointer;
	public:
		REGISTER_GUI_ENUM_CONDITION(LightType, J_LIGHT_TYPE, lightType)
		REGISTER_PROPERTY_EX(lightType, GetLightType, SetLightType, GUI_ENUM_COMBO(J_LIGHT_TYPE))
		J_LIGHT_TYPE lightType = J_LIGHT_TYPE::DIRECTIONAL;
	public:
		REGISTER_PROPERTY_EX(strength, GetStrength, SetStrength, GUI_COLOR_PICKER(false))
		DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
		REGISTER_PROPERTY_EX(falloffStart, GetFalloffStart, SetFalloffStart, GUI_SLIDER(1, 100, true, false, GUI_ENUM_CONDITION_USER(LightType, J_LIGHT_TYPE::POINT, J_LIGHT_TYPE::SPOT)))
		float falloffStart = 1.0f;
		REGISTER_PROPERTY_EX(falloffEnd, GetFalloffEnd, SetFalloffEnd, GUI_SLIDER(1, 100, true, false, GUI_ENUM_CONDITION_USER(LightType, J_LIGHT_TYPE::POINT, J_LIGHT_TYPE::SPOT)))
		float falloffEnd = 10.0f;
		REGISTER_PROPERTY_EX(spotPower, GetSpotPower, SetSpotPower, GUI_SLIDER(0, 1, true, false, GUI_ENUM_CONDITION_USER(LightType, J_LIGHT_TYPE::POINT)))
		float spotPower = 0;
	public:
		REGISTER_PROPERTY_EX(onShadow, IsShadowActivated, SetShadow, GUI_CHECKBOX())
		bool onShadow = false;
	public:
		bool onOcclusion;		//미구현 .. editor interface && register함수 추가필요 추후 Graphic 기능 확장할때 구현예정
		DirectX::XMFLOAT4X4 shadowTransform;
	public:
		JLightImpl(const InitData& initData, JLight* thisLitRaw) {}
		~JLightImpl() {}
	public:
		J_LIGHT_TYPE GetLightType()const noexcept
		{
			return lightType;
		}
		DirectX::XMFLOAT3 GetStrength()const noexcept
		{
			return strength;
		}
		float GetFalloffStart()const noexcept
		{
			return falloffStart;
		}
		float GetFalloffEnd()const noexcept
		{
			return falloffEnd;
		}
		float GetSpotPower()const noexcept
		{
			return spotPower;
		}
	public:
		void SetLightType(const J_LIGHT_TYPE newLightType)noexcept
		{
			lightType = newLightType;
			SetFrameDirty();
		}
		void SetStrength(const DirectX::XMFLOAT3& newStrength)noexcept
		{
			strength = newStrength;
			SetFrameDirty();
		}
		void SetFalloffStart(const float newFalloffStart)noexcept
		{
			falloffStart = newFalloffStart;
			SetFrameDirty();
		}
		void SetFalloffEnd(const float newFalloffEnd)noexcept
		{
			falloffEnd = newFalloffEnd;
			SetFrameDirty();
		}
		void SetSpotPower(const float newSpotPower)noexcept
		{
			spotPower = newSpotPower;
			SetFrameDirty();
		}
		void SetShadow(const bool value)noexcept
		{
			static auto IsShadow = [](const JUserPtr<JComponent>& jcomp) {return static_cast<JLight*>(jcomp.Get())->impl->onShadow; };
			if (value == onShadow)
				return;

			if (value)
			{
				if (thisPointer->IsActivated() && !onShadow)
				{
					CreateShadowMap();
					SetFrameDirty();
					onShadow = value;
				}
			}
			else
			{
				if (onShadow)
				{
					DestroyShadowMap();
					SetFrameDirty();
					onShadow = value;
				}
			}
		}
	public:
		bool IsShadowActivated()const noexcept
		{
			return onShadow;
		}
		bool IsOcclusionActivated()const noexcept
		{
			return onOcclusion;
		}
	public:
		void CreateShadowMap()noexcept
		{
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT);
			RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT);
			RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP);

			CreateShadowMapTexture();
			AddDrawShadowRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);

			CreateFrustumCullingData();
			AddFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
			//CreateFrustumCullingData();
			//AddFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer, Graphic::J_GRAPHIC_DRAW_FREQUENCY::UPDATED);
		}
		void DestroyShadowMap(const bool calledByDeAct = false)noexcept
		{
			if(!calledByDeAct)
				RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT);
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT);
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP);
			PopDrawRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
			DestroyAllTexture();

			DestroyCullingData(this, Graphic::J_CULLING_TYPE::FRUSTUM);
			PopFrustumCullingRequest(thisPointer->GetOwner()->GetOwnerScene(), thisPointer);
		};
	public:
		DirectX::XMMATRIX CalLightView()const noexcept
		{
			const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR lightPosV = XMVectorScale(thisPointer->GetLightDir(), -50);

			const XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			return XMMatrixLookAtLH(lightPosV, targetPosV, lightUp);
		}
		DirectX::XMMATRIX CalLightProj()const noexcept
		{
			const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);

			XMFLOAT3 sphereCenterLS;
			XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPosV, CalLightView()));

			float l = sphereCenterLS.x - 50;
			float b = sphereCenterLS.y - 50;
			float n = sphereCenterLS.z - 50;
			float r = sphereCenterLS.x + 50;
			float t = sphereCenterLS.y + 50;
			float f = sphereCenterLS.z + 50;

			return XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
		}
	public:
		void Activate()noexcept
		{
			RegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT);
			if (onShadow)
				CreateShadowMap();
		}
		void DeActivate()noexcept
		{ 
			//has order dependency
			DestroyShadowMap(true);
			DeRegisterLightFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT);
		}
	public:
		void UpdateFrame(Graphic::JLightConstants& constant) noexcept final
		{
			constant.strength = strength;
			constant.falloffStart = falloffStart;
			XMStoreFloat3(&constant.direction, thisPointer->GetLightDir());
			constant.falloffEnd = falloffEnd;
			constant.position = thisPointer->GetOwner()->GetTransform()->GetPosition();
			constant.spotPower = spotPower;
			constant.lightType = (int)lightType;
			LitFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapLightConstants& constant)noexcept final
		{
			UpdateShadowTransform();
			XMStoreFloat4x4(&constant.shadowTransform, XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
			constant.strength = strength;
			constant.falloffStart = falloffStart;
			XMStoreFloat3(&constant.direction, thisPointer->GetLightDir());
			constant.falloffEnd = falloffEnd;
			constant.position = thisPointer->GetOwner()->GetTransform()->GetPosition();
			constant.spotPower = spotPower;
			constant.lightType = (int)lightType;
			constant.shadowMapIndex = GetResourceArrayIndex(Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP);
			ShadowLitFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JShadowMapConstants& constant) noexcept final
		{
			XMStoreFloat4x4(&constant.viewProj, XMMatrixTranspose(XMMatrixMultiply(CalLightView(), CalLightProj())));
			ShadowMapFrame::MinusMovedDirty();
		}
		void UpdateFrame(Graphic::JHzbOccPassConstants& constant, const uint queryCount, const uint queryOffset)noexcept final
		{
			auto info = JGraphic::Instance().GetGraphicInfo();
			auto option = JGraphic::Instance().GetGraphicOption();
			//미구현
			HzbOccPassFrame::MinusMovedDirty();
		}
		void UpdateShadowTransform()noexcept
		{
			const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR lightPosV = XMVectorScale(thisPointer->GetLightDir(), -50);

			const XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			const XMMATRIX lightView = XMMatrixLookAtLH(lightPosV, targetPosV, lightUp);

			XMFLOAT3 sphereCenterLS;
			XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPosV, lightView));

			float l = sphereCenterLS.x - 50;
			float b = sphereCenterLS.y - 50;
			float n = sphereCenterLS.z - 50;
			float r = sphereCenterLS.x + 50;
			float t = sphereCenterLS.y + 50;
			float f = sphereCenterLS.z + 50;

			const XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
			//const XMMATRIX lightProj_P = XMMatrixPerspectiveOffCenterLH(l, r, b, t, n, f);
			// JTransform NDC space [-1,+1]^2 to texture space [0,1]^2
			const XMMATRIX T(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, -0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f, 0.0f, 1.0f);

			const XMMATRIX S = lightView * lightProj * T;
			XMStoreFloat4x4(&shadowTransform, S);
		}
	public:
		static bool DoCopy(JLight* from, JLight* to)
		{
			to->impl->strength = from->impl->strength;
			to->impl->falloffStart = from->impl->falloffStart;
			to->impl->falloffEnd = from->impl->falloffEnd;
			to->impl->spotPower = from->impl->spotPower;

			to->SetLightType(from->impl->lightType);
			to->SetShadow(from->impl->onShadow);
			to->impl->SetFrameDirty();
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
			LitFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT, (LitFrame*)this);
			ShadowLitFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT, (ShadowLitFrame*)this);
			ShadowMapFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP, (ShadowMapFrame*)this);
			HzbOccPassFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_PASS, (HzbOccPassFrame*)this);
		}
	public:
		void RegisterThisPointer(JLight* lit)
		{
			thisPointer = Core::GetWeakPtr(lit);
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
		void RegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT)
				LitFrame::RegisterFrameData(type, (LitFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT)
				ShadowLitFrame::RegisterFrameData(type, (ShadowLitFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP)
				ShadowMapFrame::RegisterFrameData(type, (ShadowMapFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_PASS)
				HzbOccPassFrame::RegisterFrameData(type, (HzbOccPassFrame*)this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void DeRegisterLightFrameData(const Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE type)
		{
			if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::LIGHT)
				LitFrame::DeRegisterFrameData(type, (LitFrame*)this);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP_LIGHT)
				ShadowLitFrame::DeRegisterFrameData(type, (ShadowLitFrame*)this);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::SHADOW_MAP)
				ShadowMapFrame::DeRegisterFrameData(type, (ShadowMapFrame*)this);
			else if (type == Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::HZB_OCC_PASS)
				HzbOccPassFrame::DeRegisterFrameData(type, (HzbOccPassFrame*)this);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JLight::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JLight::InitData>(ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JLight::InitData>(parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static auto setFrameLam = [](JComponent* comp) {static_cast<JLight*>(comp)->impl->SetFrameDirty(); };
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable,isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable};

			RegisterCTypeInfo(JLight::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JLight::StaticTypeInfo(), lPrivate);

			IMPL_REALLOC_BIND(JLight::JLightImpl, thisPointer)
		}
	};

	JLight::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(JLight::StaticTypeInfo(), owner)
	{}
	JLight::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(JLight::StaticTypeInfo(), GetDefaultName(JLight::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JLight::PrivateInterface()const noexcept
	{
		return lPrivate;
	}
	Graphic::JFrameUpdateUserAccess JLight::FrameUserInterface() noexcept
	{
		return Graphic::JFrameUpdateUserAccess(Core::GetUserPtr(this), impl.get());
	}
	const Graphic::JGraphicMultiResourceUserInterface JLight::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicMultiResourceUserInterface{ impl.get() };
	}
	const Graphic::JCullingUserInterface JLight::CullingUserInterface()const noexcept
	{
		return Graphic::JCullingUserInterface{ impl.get() };
	}
	J_COMPONENT_TYPE JLight::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	J_LIGHT_TYPE JLight::GetLightType()const noexcept
	{
		return impl->GetLightType();
	}
	DirectX::XMFLOAT3 JLight::GetStrength()const noexcept
	{
		return impl->GetStrength();
	}
	DirectX::XMVECTOR JLight::GetLightDir()const noexcept
	{
		const XMFLOAT3 rotf = GetOwner()->GetTransform()->GetRotation();
		const XMVECTOR rotv = XMLoadFloat3(&rotf);
		const XMVECTOR qV = XMQuaternionRotationRollPitchYawFromVector(XMVectorScale(rotv, JMathHelper::DegToRad));
		const XMVECTOR initDir = XMVectorSet(0, -1, 0, 1);
		return XMVector3Normalize(XMVector3Rotate(initDir, qV));
	}
	float JLight::GetFalloffStart()const noexcept
	{
		return impl->GetFalloffStart();
	}
	float JLight::GetFalloffEnd()const noexcept
	{
		return impl->GetFalloffEnd();
	}
	float JLight::GetSpotPower()const noexcept
	{
		return impl->GetSpotPower();
	}
	void JLight::SetLightType(const J_LIGHT_TYPE lightType)noexcept
	{
		impl->SetLightType(lightType);
	}
	void JLight::SetStrength(const DirectX::XMFLOAT3& strength)noexcept
	{
		impl->SetStrength(strength);
	}
	void JLight::SetFalloffStart(const float falloffStart)noexcept
	{
		impl->SetFalloffStart(falloffStart);
	}
	void JLight::SetFalloffEnd(const float falloffEnd)noexcept
	{
		impl->SetFalloffEnd(falloffEnd);
	}
	void JLight::SetSpotPower(const float spotPower)noexcept
	{
		impl->SetSpotPower(spotPower);
	}
	void JLight::SetShadow(const bool value)noexcept
	{
		impl->SetShadow(value);
	}
	bool JLight::IsShadowActivated()const noexcept
	{
		return impl->IsShadowActivated();
	}
	bool JLight::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	bool JLight::AllowHzbOcclusionCulling()const noexcept
	{
		return false;
	}
	void JLight::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent(impl->thisPointer);
		impl->Activate();
		impl->SetFrameDirty();
	}
	void JLight::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent(impl->thisPointer);
		impl->DeActivate();
		impl->OffFrameDirty();
	}
	JLight::JLight(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JLightImpl>(initData, this))
	{ }
	JLight::~JLight()
	{
		impl.reset();
	}


	using CreateInstanceInterface = JLightPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JLightPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JLightPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JLightPrivate::FrameUpdateInterface; 
	using FrameIndexInterface = JLightPrivate::FrameIndexInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JLight>(*static_cast<JLight::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JLight* lit = static_cast<JLight*>(createdPtr);
		lit->impl->RegisterThisPointer(lit);
		lit->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JLight::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JLight::JLightImpl::DoCopy(static_cast<JLight*>(from.Get()), static_cast<JLight*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
		static_cast<JLight*>(ptr)->impl->DeRegisterPreDestruction();
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JLight::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		XMFLOAT3 sStrength;
		float sFallOffsetStart;
		float sFallOffsetEnd;
		float sSpotPower;
		bool sOnShadow;

		auto loadData = static_cast<JLight::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JUserPtr<JGameObject> owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadXMFloat3(stream, sStrength);
		JFileIOHelper::LoadAtomicData(stream, sFallOffsetStart);
		JFileIOHelper::LoadAtomicData(stream, sFallOffsetEnd);
		JFileIOHelper::LoadAtomicData(stream, sSpotPower);
		JFileIOHelper::LoadAtomicData(stream, sOnShadow);

		auto idenUser = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JLight::InitData>(guid, flag, owner), &lPrivate);
		JUserPtr<JLight> litUser;
		litUser.ConnnectChild(idenUser);
		litUser->SetStrength(sStrength);
		litUser->SetFalloffStart(sFallOffsetStart);
		litUser->SetFalloffEnd(sFallOffsetEnd);
		litUser->SetSpotPower(sSpotPower);
		litUser->SetShadow(sOnShadow);

		return litUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JLight::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JLight::StoreData*>(data);
		if (!storeData->HasCorrectType(JLight::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JLight> lit;
		lit.ConnnectChild(storeData->obj);

		JLight::JLightImpl* impl = lit->impl.get();
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, lit.Get());
		JFileIOHelper::StoreXMFloat3(stream, L"Strength:", impl->strength);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetStart:", impl->falloffStart);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetEnd:", impl->falloffEnd);
		JFileIOHelper::StoreAtomicData(stream, L"SpotPower:", impl->spotPower);
		JFileIOHelper::StoreAtomicData(stream, L"OnShadow:", impl->onShadow);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JLight* lit, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			lit->impl->SetFrameDirty();

		lit->impl->SetLastFrameUpdatedTrigger(false);
		lit->impl->SetLastFrameHotUpdatedTrigger(false);
		return lit->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JLight* lit, Graphic::JLightConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JLight* lit, Graphic::JShadowMapLightConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JLight* lit, Graphic::JShadowMapConstants& constant)noexcept
	{
		lit->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateFrame(JLight* lit, Graphic::JHzbOccPassConstants& constant, const uint queryCount, const uint queryOffset)noexcept
	{
		lit->impl->UpdateFrame(constant, queryCount, queryOffset);
	}
	void FrameUpdateInterface::UpdateEnd(JLight* lit)noexcept
	{
		if (lit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources)
			lit->impl->SetLastFrameHotUpdatedTrigger(true);
		lit->impl->SetLastFrameUpdatedTrigger(true);
		lit->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->LitFrame::GetUploadIndex();
	}
	int FrameUpdateInterface::GetShadowLitFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->ShadowLitFrame::GetUploadIndex();
	}
	int FrameUpdateInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->ShadowMapFrame::GetUploadIndex();
	}
	int FrameUpdateInterface::GetHzbOccPassFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->HzbOccPassFrame::GetUploadIndex();
	}
	bool FrameUpdateInterface::IsHotUpdated(JLight* lit)noexcept
	{
		return lit->impl->IsLastFrameHotUpdated();
	}
	bool FrameUpdateInterface::IsLastUpdated(JLight* lit)noexcept
	{
		return lit->impl->IsLastFrameUpdated();
	}
	bool FrameUpdateInterface::HasLitRecopyRequest(JLight* lit)noexcept
	{
		return lit->impl->LitFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasShadowLitRecopyRequest(JLight* lit)noexcept
	{
		return lit->impl->ShadowLitFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasShadowMapRecopyRequest(JLight* lit)noexcept
	{
		return lit->impl->ShadowMapFrame::HasMovedDirty();
	}
	bool FrameUpdateInterface::HasOccPassRecopyRequest(JLight* lit)noexcept
	{
		return lit->impl->HzbOccPassFrame::HasMovedDirty();
	}

	int FrameIndexInterface::GetLitFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->LitFrame::GetUploadIndex();
	}
	int FrameIndexInterface::GetShadowLitFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->ShadowLitFrame::GetUploadIndex();
	}
	int FrameIndexInterface::GetShadowMapFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->ShadowMapFrame::GetUploadIndex();
	}
	int FrameIndexInterface::GetHzbOccPassFrameIndex(JLight* lit)noexcept
	{
		return lit->impl->HzbOccPassFrame::GetUploadIndex();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JLightPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JLightPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JLightPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}