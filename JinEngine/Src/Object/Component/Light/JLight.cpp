#include"JLight.h"  
#include"../JComponentFactory.h"
#include"../../Component/Transform/JTransform.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Graphic/FrameResource/JLightConstants.h" 
#include"../../../Graphic/FrameResource/JShadowMapConstants.h" 
#include<Windows.h>
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	static auto isAvailableoverlapLam = []() {return true; };
	J_COMPONENT_TYPE JLight::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	J_LIGHT_TYPE JLight::GetLightType()const noexcept
	{
		return lightType;
	}
	DirectX::XMFLOAT3 JLight::GetStrength()const noexcept
	{
		return strength;
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
		return falloffStart;
	}
	float JLight::GetFalloffEnd()const noexcept
	{
		return falloffEnd;
	}
	float JLight::GetSpotPower()const noexcept
	{
		return spotPower;
	}
	void JLight::SetLightType(const J_LIGHT_TYPE lightType)noexcept
	{
		JLight::lightType = lightType;
		SetFrameDirty();
	}
	void JLight::SetStrength(const DirectX::XMFLOAT3& strength)noexcept
	{
		JLight::strength = strength;
		SetFrameDirty();
	}
	void JLight::SetFalloffStart(const float falloffStart)noexcept
	{
		JLight::falloffStart = falloffStart;
		SetFrameDirty();
	}
	void JLight::SetFalloffEnd(const float falloffEnd)noexcept
	{
		JLight::falloffEnd = falloffEnd;
		SetFrameDirty();
	}
	void JLight::SetSpotPower(const float spotPower)noexcept
	{
		JLight::spotPower = spotPower;
		SetFrameDirty();
	}
	void JLight::SetShadow(const bool value)noexcept
	{
		static auto IsShadow = [](JComponent& jcomp)
		{
			return static_cast<JLight*>(&jcomp)->onShadow;
		};

		if (value == onShadow)
			return;

		if (value)
		{
			if (IsActivated() && !onShadow)
			{
				CreateShadowMap();
				GetOwner()->GetOwnerScene()->AppInterface()->SetBackSideComponentDirty(*this, IsShadow);
				SetFrameDirty();
				onShadow = value;
			}
		}
		else
		{
			if (onShadow)
			{
				DestroyShadowMap();
				GetOwner()->GetOwnerScene()->AppInterface()->SetBackSideComponentDirty(*this, IsShadow);
				SetFrameDirty();
				onShadow = value;
			}
		}
	}
	bool JLight::IsShadowActivated()const noexcept
	{
		return onShadow;
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
	void JLight::DoCopy(JObject* ori)
	{
		JLight* oriLit = static_cast<JLight*>(ori);
		strength = oriLit->strength;
		falloffStart = oriLit->falloffStart;
		falloffEnd = oriLit->falloffEnd;
		spotPower = oriLit->spotPower;

		SetLightType(oriLit->lightType);
		SetShadow(oriLit->onShadow);
		SetFrameDirty();
	}
	void JLight::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent();
		if (onShadow)
			CreateShadowMap();
		SetFrameDirty();
	}
	void JLight::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
		if (onShadow)
			DestroyShadowMap();
		OffFrameDirty();
	}
	void JLight::CreateShadowMap()noexcept
	{
		CreateShadowMapTexture();
		AddDrawRequest(GetOwner()->GetOwnerScene(), this);
	}
	void JLight::DestroyShadowMap()noexcept
	{
		PopDrawRequest(GetOwner()->GetOwnerScene(), this);
		DestroyTxtHandle();
	}
	DirectX::XMMATRIX JLight::CalLightView()const noexcept
	{
		const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);
		const XMVECTOR lightPosV = XMVectorScale(GetLightDir(), -50);

		const XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		return XMMatrixLookAtLH(lightPosV, targetPosV, lightUp);
	}
	DirectX::XMMATRIX JLight::CalLightProj()const noexcept
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
	void JLight::UpdateShadowTransform()noexcept
	{
		const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);
		const XMVECTOR lightPosV = XMVectorScale(GetLightDir(), -50);

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
	void JLight::UpdateFrame(Graphic::JLightConstants& lightConstant)
	{
		lightConstant.strength = strength;
		lightConstant.falloffStart = falloffStart;
		XMStoreFloat3(&lightConstant.direction, GetLightDir());
		lightConstant.falloffEnd = falloffEnd;
		lightConstant.position = GetOwner()->GetTransform()->GetPosition();
		lightConstant.spotPower = spotPower;
		lightConstant.lightType = (int)lightType;
	}
	void JLight::UpdateFrame(Graphic::JShadowMapLightConstants& smLightConstant)
	{
		UpdateShadowTransform();
		XMStoreFloat4x4(&smLightConstant.shadowTransform, XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
		smLightConstant.strength = strength;
		smLightConstant.falloffStart = falloffStart;
		XMStoreFloat3(&smLightConstant.direction, GetLightDir());
		smLightConstant.falloffEnd = falloffEnd;
		smLightConstant.position = GetOwner()->GetTransform()->GetPosition();
		smLightConstant.spotPower = spotPower;
		smLightConstant.lightType = (int)lightType;
		smLightConstant.shadowMapIndex = GetResourceArrayIndex();
	}
	void JLight::UpdateFrame(Graphic::JShadowMapConstants& shadowConstant)
	{
		XMStoreFloat4x4(&shadowConstant.viewProj, XMMatrixTranspose(XMMatrixMultiply(CalLightView(), CalLightProj())));
	}
	Core::J_FILE_IO_RESULT JLight::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JLight::StoreObject(std::wofstream& stream, JLight* light)
	{
		if (light == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)light->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::StoreObjectIden(stream, light);
		JFileIOHelper::StoreXMFloat3(stream, L"Strength:", light->strength);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetStart:", light->falloffStart);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetEnd:", light->falloffEnd);
		JFileIOHelper::StoreAtomicData(stream, L"SpotPower:", light->spotPower);
		JFileIOHelper::StoreAtomicData(stream, L"OnShadow:", light->onShadow);

		return  Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JLight* JLight::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open())
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		XMFLOAT3 sStrength;
		float sFallOffsetStart;
		float sFallOffsetEnd;
		float sSpotPower;
		bool sOnShadow;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadXMFloat3(stream, sStrength);
		JFileIOHelper::LoadAtomicData(stream, sFallOffsetStart);
		JFileIOHelper::LoadAtomicData(stream, sFallOffsetEnd);
		JFileIOHelper::LoadAtomicData(stream, sSpotPower);
		JFileIOHelper::LoadAtomicData(stream, sOnShadow);

		Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(guid, flag, owner);
		JLight* newLightComponent = ownerPtr.Get();

		if (!AddInstance(std::move(ownerPtr)))
			return nullptr;

		newLightComponent->SetStrength(sStrength);
		newLightComponent->SetFalloffStart(sFallOffsetStart);
		newLightComponent->SetFalloffEnd(sFallOffsetEnd);
		newLightComponent->SetSpotPower(sSpotPower);
		newLightComponent->SetShadow(sOnShadow);

		return newLightComponent;
	}
	void JLight::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
			JLight* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(guid, objFlag, owner);
			JLight* newComp = ownerPtr.Get();
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
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(Core::MakeGuid(), ori->GetFlag(), owner);
			JLight* newComp = ownerPtr.Get();
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
		JCFI<JLight>::Register(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JLight::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JLight::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

		static auto setFrameLam = [](JComponent& component) {static_cast<JLight*>(&component)->SetFrameDirty(); };
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{ getTypeNameCallable, getTypeInfoCallable,isAvailableOverlapCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JLight::JLight(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JLightInterface(TypeName(), guid, objFlag, owner)
	{
		RegisterFrameDirtyListener(*GetOwner()->GetTransform());
		lightType = J_LIGHT_TYPE::DIRECTIONAL;
	}
	JLight::~JLight()
	{
		if (GetOwner()->GetTransform() != nullptr)
			DeRegisterFrameDirtyListener(*GetOwner()->GetTransform());
	}
}