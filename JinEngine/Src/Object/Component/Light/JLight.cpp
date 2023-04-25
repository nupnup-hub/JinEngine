#include"JLight.h"   
#include"JLightPrivate.h"
#include"../Transform/JTransform.h"
#include"../Transform/JTransformPrivate.h"
#include"../JComponentHint.h"
#include"../../JFrameUpdate.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h" 
#include"../../Resource/Scene/JScenePrivate.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Graphic/FrameResource/JLightConstants.h" 
#include"../../../Graphic/FrameResource/JShadowMapConstants.h" 
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include<Windows.h>
#include<fstream>

namespace JinEngine
{ 
	using namespace DirectX;
	namespace
	{
		using LitFrameUpdate = JFrameUpdate3 <JFrameUpdateBase<Graphic::JLightConstants&>,
			JFrameUpdateBase<Graphic::JShadowMapLightConstants&>,
			JFrameUpdateBase<Graphic::JShadowMapConstants&>>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JLightPrivate lPrivate;
	}
 
	class JLight::JLightImpl : public Core::JIdentifierImplBase,
		public JFrameUpdate<LitFrameUpdate, JFrameDirtyListener, FrameUpdate::nonBuff>,
		public Graphic::JGraphicResourceInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JLightImpl)
	public:
		JLight* thisLit;
	public:
		REGISTER_GUI_ENUM_CONDITION(LightType, J_LIGHT_TYPE, lightType)
		REGISTER_PROPERTY_EX(lightType, GetLightType, SetLightType, GUI_ENUM_COMBO(J_LIGHT_TYPE))
		J_LIGHT_TYPE lightType;
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
		DirectX::XMFLOAT4X4 shadowTransform;
	public:
		JLightImpl(const InitData& initData, JLight* thisLit)
			:thisLit(thisLit)
		{
			JTransformPrivate::FrameDirtyInterface::RegisterFrameDirtyListener(thisLit->GetOwner()->GetTransform(), this);
			lightType = J_LIGHT_TYPE::DIRECTIONAL;
		}
		~JLightImpl()
		{
			if (thisLit->GetOwner()->GetTransform() != nullptr)
				JTransformPrivate::FrameDirtyInterface::DeRegisterFrameDirtyListener(thisLit->GetOwner()->GetTransform(), this);
		}
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
			static auto IsShadow = [](JComponent& jcomp){return static_cast<JLight*>(&jcomp)->impl->onShadow;};
			if (value == onShadow)
				return;

			if (value)
			{
				if (thisLit->IsActivated() && !onShadow)
				{
					CreateShadowMap();
					JScenePrivate::CompFrameInterface::SetComponentFrameDirty(thisLit->GetOwner()->GetOwnerScene(), thisLit->GetComponentType(), thisLit, IsShadow);
					SetFrameDirty();
					onShadow = value;
				}
			}
			else
			{
				if (onShadow)
				{
					DestroyShadowMap();
					JScenePrivate::CompFrameInterface::SetComponentFrameDirty(thisLit->GetOwner()->GetOwnerScene(), thisLit->GetComponentType(), thisLit, IsShadow);
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
	public:
		void CreateShadowMap()noexcept
		{
			CreateShadowMapTexture();
			AddDrawRequest(thisLit->GetOwner()->GetOwnerScene(), thisLit);
		}
		void DestroyShadowMap()noexcept
		{
			PopDrawRequest(thisLit->GetOwner()->GetOwnerScene(), thisLit);
			DestroyTexture();
		};
	public:
		DirectX::XMMATRIX CalLightView()const noexcept
		{
			const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR lightPosV = XMVectorScale(thisLit->GetLightDir(), -50);

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
		void UpdateFrame(Graphic::JLightConstants& constant) noexcept final
		{
			constant.strength = strength;
			constant.falloffStart = falloffStart;
			XMStoreFloat3(&constant.direction, thisLit->GetLightDir());
			constant.falloffEnd = falloffEnd;
			constant.position = thisLit->GetOwner()->GetTransform()->GetPosition();
			constant.spotPower = spotPower;
			constant.lightType = (int)lightType;
		}
		void UpdateFrame(Graphic::JShadowMapLightConstants& constant)noexcept final
		{
			UpdateShadowTransform();
			XMStoreFloat4x4(&constant.shadowTransform, XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
			constant.strength = strength;
			constant.falloffStart = falloffStart;
			XMStoreFloat3(&constant.direction, thisLit->GetLightDir());
			constant.falloffEnd = falloffEnd;
			constant.position = thisLit->GetOwner()->GetTransform()->GetPosition();
			constant.spotPower = spotPower;
			constant.lightType = (int)lightType;
			constant.shadowMapIndex = GetResourceArrayIndex();
		}
		void UpdateFrame(Graphic::JShadowMapConstants& constant) noexcept final
		{
			XMStoreFloat4x4(&constant.viewProj, XMMatrixTranspose(XMMatrixMultiply(CalLightView(), CalLightProj())));
		}
		void UpdateShadowTransform()noexcept
		{
			const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);
			const XMVECTOR lightPosV = XMVectorScale(thisLit->GetLightDir(), -50);

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
		static void RegisterCallOnce()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JLight::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](JGameObject* parent, InitUnq&& parentClassInitData) -> InitUnq
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

			static auto setFrameLam = [](JComponent* component) {static_cast<JLight*>(component)->impl->SetFrameDirty(); };
			static SetCFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable,isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ &setFrameDirtyCallable, nullptr };

			RegisterCTypeInfo(JLight::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JLight::StaticTypeInfo(), lPrivate);
		}
	};

	JLight::InitData::InitData(JGameObject* owner)
		:JComponent::InitData(JLight::StaticTypeInfo(), owner)
	{}
	JLight::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		: JComponent::InitData(JLight::StaticTypeInfo(), GetDefaultName(JLight::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JLight::GetPrivateInterface()const noexcept
	{
		return lPrivate;
	}
	JFrameUpdateUserAccess JLight::GetFrameUserInterface() noexcept
	{
		return JFrameUpdateUserAccess(Core::GetUserPtr(this), impl.get());
	}
	const Graphic::JGraphicResourceUserInterface JLight::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface{ impl.get() };
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
	void JLight::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent();
		if (IsShadowActivated())
			impl->CreateShadowMap();
		impl->SetFrameDirty();
	}
	void JLight::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
		if (IsShadowActivated())
			impl->DestroyShadowMap();
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
	using AssetDataIOInterface = JLightPrivate::AssetDataIOInterface; 
	using FrameUpdateInterface = JLightPrivate::FrameUpdateInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JLight>(*static_cast<JLight::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JLight::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JLight::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JLight::JLightImpl::DoCopy(static_cast<JLight*>(from), static_cast<JLight*>(to));
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
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
		JGameObject* owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadXMFloat3(stream, sStrength);
		JFileIOHelper::LoadAtomicData(stream, sFallOffsetStart);
		JFileIOHelper::LoadAtomicData(stream, sFallOffsetEnd);
		JFileIOHelper::LoadAtomicData(stream, sSpotPower);
		JFileIOHelper::LoadAtomicData(stream, sOnShadow);

		auto rawPtr = lPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JLight::InitData>(guid, flag, owner), &lPrivate);
		JLight* newLit = static_cast<JLight*>(rawPtr);
		newLit->SetStrength(sStrength);
		newLit->SetFalloffStart(sFallOffsetStart);
		newLit->SetFalloffEnd(sFallOffsetEnd);
		newLit->SetSpotPower(sSpotPower);
		newLit->SetShadow(sOnShadow);

		return newLit;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JLight::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JLight::StoreData*>(data);
		if (!storeData->HasCorrectType(JLight::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JLight* lit = static_cast<JLight*>(storeData->obj);
		JLight::JLightImpl* impl = lit->impl.get();
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, lit);
		JFileIOHelper::StoreXMFloat3(stream, L"Strength:", impl->strength);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetStart:", impl->falloffStart);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetEnd:", impl->falloffEnd);
		JFileIOHelper::StoreAtomicData(stream, L"SpotPower:", impl->spotPower);
		JFileIOHelper::StoreAtomicData(stream, L"OnShadow:", impl->onShadow);

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JLight* cam, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			cam->impl->SetFrameDirty();

		return cam->impl->IsFrameDirted();
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
	void FrameUpdateInterface::UpdateEnd(JLight* lit)noexcept
	{
		lit->impl->UpdateEnd();
	}
	bool FrameUpdateInterface::IsHotUpdated(JLight* lit)noexcept
	{
		return lit->impl->GetFrameDirty() == Graphic::Constants::gNumFrameResources;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JLightPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JLightPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}